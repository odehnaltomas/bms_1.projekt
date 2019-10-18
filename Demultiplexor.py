import sys


class Demultiplexor:
    PID_PAT = 0x0000
    PID_NIT = 0x0010
    PID_SDT = 0x0011

    TABLE_ID_PAT = 0x00
    TABLE_ID_PMT = 0x02
    TABLE_ID_NIT = 0x40
    TABLE_ID_SDT = 0x42


    def __init__(self):
        self.packet_num = 0
        self.PAT_data = dict()

    def parse_PAT(self, packet):
        table_id = packet.payload[0]
        # Check if table_id is OK
        if table_id != self.TABLE_ID_PAT:
            print("ERROR: Bad table id in PAT.", file=sys.stderr)
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

        self.print_PAT(table_id, section_length, self.PAT_data, transport_stream_id, version_number, current_next_indicator,
                       section_number, last_section_number)
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
            print("ERROR: Bad table id in NIT.", file=sys.stderr)
            return False

        section_length = ((packet.payload[1] & 0xf) << 8) + packet.payload[2]
        network_id = (packet.payload[3] << 8) + packet.payload[4]
        version_number = packet.payload[5] & 0x3E
        section_number = packet.payload[6]
        last_section_number = packet.payload[7]
        network_descriptors_length = ((packet.payload[8] & 0xF) << 8) + packet.payload[9]
        descriptor = ''
        for i in range(0, network_descriptors_length):
            index = 10
            print(hex(packet.payload[index+i]))
            descriptor += chr(packet.payload[index+i])
            self.parse_descriptors(10, network_descriptors_length)

        print(descriptor)
        transport_stream_loop_length = ((packet.payload[10 + network_descriptors_length] & 0xf) << 8) + packet.payload[10 + network_descriptors_length+1]
        print("transport len:", transport_stream_loop_length)

        i = 0

        # while i <
        return True

    def parse_descriptors(self, offset, length):

