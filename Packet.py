import sys


class Packet:

    def __init__(self, packet_size, buffer):
        self.packet_size = packet_size  # Size of TS packet (always 188B)
        self.buffer = buffer    # Content of packet (all 188B)
        self.sync_byte = 0
        self.transport_error_ind = 0    # Check if there is error in packet \\TODO: check meaning of this field
        self.payload_unit_start_ind = 0     #
        self.transport_prio = 0     #
        self.pid = 0    #
        self.transport_scrambling_ctrl = 0  #
        self.adaptation_field_ctrl = 0  #
        self.continuity_counter = 0     #

        self.payload = ''

    def parse(self):
        header = self.buffer[0:4]

        self.sync_byte = header[0]
        if self.sync_byte != 0x47:
            print("ERROR: Bad sync error!", file=sys.stderr)
            return False

        # Fill the Packet variables with values in header
        self.transport_error_ind = header[1] >> 7
        self.payload_unit_start_ind = (header[1] >> 6) & 0x1
        self.transport_prio = (header[1] >> 5) & 0x1
        self.pid = ((header[1] & 0x1F) << 8) + header[2]
        self.transport_scrambling_ctrl = (header[3] >> 6)
        self.adaptation_field_ctrl = (header[3] >> 4) & 0x3
        self.continuity_counter = header[3] & 0xF

        # Now i will use only buffer variable ("basic" header is parsed)
        len = 0
        if self.adaptation_field_ctrl == 0x2 or self.adaptation_field_ctrl == 0x3:
            len = self.buffer[4]

        if self.payload_unit_start_ind:
            len += 1

        if self.adaptation_field_ctrl == 0x1 or self.adaptation_field_ctrl == 0x3:
            # TODO: not finished
            skip = 4 + len
            self.payload = self.buffer[skip:]

    def print_header(self):
        print('')
        print('*********************************************************')
        print("Sync byte: " + hex(self.sync_byte))
        print("Transport error indicator: " + hex(self.transport_error_ind))
        print("Payload unit start indicator: " + hex(self.payload_unit_start_ind))
        print("Transport priority: " + hex(self.transport_prio))
        print("PID: " + hex(self.pid))
        print("Transport scrambling control: " + hex(self.transport_scrambling_ctrl))
        print("Adaptation field control: " + hex(self.adaptation_field_ctrl))
        print("Continuity counter: " + hex(self.continuity_counter))
        print('*********************************************************')
