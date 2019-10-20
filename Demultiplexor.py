import sys


class Demultiplexor:
    PID_PAT = 0x0000
    PID_NIT = 0x0010
    PID_SDT = 0x0011

    TABLE_ID_PAT = 0x00
    TABLE_ID_PMT = 0x02
    TABLE_ID_NIT = 0x40
    TABLE_ID_SDT = 0x42

    DESCRIPTOR_NETWORK_NAME = 0x40
    DESCRIPTOR_TERRESTRIAL_DELIVERY_SYSTEM = 0x5A

    def __init__(self):
        self.packet_num = 0
        self.PAT_analysed = False
        self.NIT_analysed = False
        self.SDT_analysed = False
        self.PAT_data = dict()
        self.network_name = ''
        self.network_id = 0
        self.bandwith = ''
        self.constellation = ''
        self.code_rate = ''
        self.guard_interval = ''

    def parse_PAT(self, packet):
        table_id = packet.payload[0]
        # Check if table_id is OK
        if table_id != self.TABLE_ID_PAT:
            print("ERROR: Bad table_id in PAT.", file=sys.stderr)
            return False

        section_length = ((packet.payload[1] & 0xf) << 8) + packet.payload[2]
        transport_stream_id = (packet.payload[3] << 8) + packet.payload[4]
        version_number = packet.payload[5] & 0x3E
        current_next_indicator = packet.payload[5] & 0x1
        section_number = packet.payload[6]
        last_section_number = packet.payload[7]

        i = 0
        while i < section_length:
            index = 8 + i
            program_number = (packet.payload[index] << 8) + packet.payload[index+1]

            index += 2

            network_pid = ((packet.payload[index] & 0x1f) << 8) + packet.payload[index+1]

            self.PAT_data[program_number] = network_pid

            if program_number == 0:
                self.PID_NIT = network_pid
            i += 4

        # self.print_PAT(table_id, section_length, self.PAT_data, transport_stream_id, version_number, current_next_indicator,
        #                section_number, last_section_number)

        self.PAT_analysed = True
        return True

    def print_PAT(self, table_id, section_length, PAT_data, transport_stream_id='unknown',
                  version_number='unknown', curr_next_ind='unknown', section_num='unknown',
                  last_section_num='unknown'):
        print("table id:", table_id)
        print("section length:", section_length)
        print("transport stream id:", transport_stream_id)
        print("version number:", version_number)
        print("current next indicator:", curr_next_ind)
        print("section number:", section_num)
        print("last section number:", last_section_num)
        print("PROGRAMS:")
        for prog_num, network_pid in PAT_data.items():
            print("\tprogram number:", prog_num, ",(" + str(hex(prog_num)) + ")")
            print("\tnetwork pid:", network_pid, ",(" + str(hex(network_pid)) + ")")

    def parse_NIT(self, packet):
        table_id = packet.payload[0]
        if table_id != self.TABLE_ID_NIT:
            print("ERROR: Bad table_id in NIT.", file=sys.stderr)
            return False

        section_length = ((packet.payload[1] & 0xf) << 8) + packet.payload[2]
        self.network_id = (packet.payload[3] << 8) + packet.payload[4]
        version_number = packet.payload[5] & 0x3E
        section_number = packet.payload[6]
        last_section_number = packet.payload[7]
        network_descriptors_length = ((packet.payload[8] & 0xF) << 8) + packet.payload[9]
        skip = 10
        self.parse_descriptors(skip, network_descriptors_length, packet)
        skip += network_descriptors_length
        transport_stream_loop_length = ((packet.payload[skip] & 0xF) << 8) + packet.payload[skip+1]
        # print("transport len:", transport_stream_loop_length)
        skip += 2

        i = 0
        while i < transport_stream_loop_length:
            transport_stream_id = (packet.payload[skip] << 8) + packet.payload[skip+1]
            skip += 2
            original_network_id = (packet.payload[skip] << 8) + packet.payload[skip+1]
            skip += 2
            transport_descriptors_length = ((packet.payload[skip] & 0xF) << 8) + packet.payload[skip+1]
            skip += 2
            self.parse_descriptors(skip, transport_descriptors_length, packet)
            skip += transport_descriptors_length
            i += transport_descriptors_length + 6

        self.NIT_analysed = True
        return True

    def parse_SDT(self, packet):
        table_id = packet.payload[0]
        # if table_id != self.TABLE_ID_SDT:
        #     print("ERROR: Bad table_id in SDT.", file=sys.stderr)
        #     return False

        section_length = ((packet.payload[1] & 0xF) << 8) + packet.payload[2]
        print("current_next_indicator:", packet.payload[6] & 0x1)
        transport_stream_id = (packet.payload[3] << 8) + packet.payload[4]
        print("transport stream id:", transport_stream_id)
        print("section length:", section_length)
        print("section number:", packet.payload[7])
        print("last section number:", packet.payload[8])
        return True

    def parse_descriptors(self, offset, length, packet):
        j = 0
        while j < length:
            desc_len = packet.payload[offset+j+1]
            if packet.payload[offset + j] == self.DESCRIPTOR_NETWORK_NAME:
                network_name = ''
                for i in range(0, desc_len):
                    self.network_name += chr(packet.payload[offset + j +2+i])
                print(network_name)
            elif packet.payload[offset + j] == self.DESCRIPTOR_TERRESTRIAL_DELIVERY_SYSTEM:
                # descriptor_tag = packet.payload[offset+i]
                # descriptor_length = packet.payload[offset+1]
                # centre_frequency = (packet.payload[offset+i+2] << 8) + (packet.payload[offset+i+3] << 8)
                # centre_frequency += (packet.payload[offset+i+4] << 8)
                # centre_frequency += (packet.payload[offset + i + 5] << 8)
                self.bandwith = self.get_bandwith(packet.payload[offset + j + 6] >> 5)
                # priority = packet.payload[offset + i + 6] & 0x10
                # time_slicing_indicator = packet.payload[offset + i + 6] & 0x8
                # mpe_fec_indicator = packet.payload[offset + i + 6] & 0x4
                self.constellation = self.get_constellation(packet.payload[offset + j + 7] >> 6)
                # hiearchy_information = packet.payload[offset + i + 7] & 0x38
                self.code_rate = self.get_code_rate(packet.payload[offset + j + 7] & 0x7)
                code_rate_lp_stream = packet.payload[offset + j + 8] & 0xE0
                self.guard_interval = self.get_guard_interval(packet.payload[offset + j + 8] & 0x18)

                # print('network name:', self.network_name)
                # print('network ID:', self.network_id)
                # print('bandwith:', self.bandwith)
                # print('constellation:', self.constellation)
                # print('guard interval:', self.guard_interval)
                # print('code_rate:', self.code_rate)
            j += desc_len + 2

    def get_bandwith(self, bit_field):
        if bit_field == 0:
            return '8 MHz'
        elif bit_field == 1:
            return '7 MHz'
        elif bit_field == 2:
            return '6 MHz'
        elif bit_field == 3:
            return '5 MHz'
        else:
            return 'Unknown'

    def get_constellation(self, bit_field):
        if bit_field == 0:
            return 'QPSK'
        elif bit_field == 1:
            return '16-QAM'
        elif bit_field == 2:
            return '64-QAM'
        else:
            return 'Unknown'

    def get_code_rate(self, bit_field):
        if bit_field == 0:
            return '1/2'
        elif bit_field == 1:
            return '2/3'
        elif bit_field == 2:
            return '3/4'
        elif bit_field == 3:
            return '5/6'
        elif bit_field == 4:
            return '7/8'
        else:
            return 'Unknown'

    def get_guard_interval(self, bit_field):
        if bit_field == 0:
            return '1/32'
        elif bit_field == 1:
            return '1/16'
        elif bit_field == 2:
            return '1/8'
        else:
            return '1/4'
