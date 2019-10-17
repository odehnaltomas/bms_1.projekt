import sys


class Packet:

    def __init__(self, packet_size, buffer):
        self.packet_size = packet_size  # Size of TS packet (always 188B)
        self.buffer = buffer    # Content of packet (all 188B)
        self.transport_error_ind = 0    # Check if there is error in packet \\TODO: check meaning of this field
        self.payload_unit_start_ind = 0     #
        self.transport_prio = 0     #
        self.pid = 0    #
        self.transport_scrambling_ctrl = 0  #
        self.adaptation_field_ctrl = 0  #
        self.sc = 0     #

        self.len = 0    # Length of extension of packet header (PTR | Adaptation Field

    def parse(self):
        header = self.buffer[0:4]

        sync_byte = header[0]
        if sync_byte != 0x47:
            print("ERROR: Bad sync error!", file=sys.stderr)
            return False

        # Fill the Packet variables with values in header
        self.transport_error_ind = header[1] >> 7
        self.payload_unit_start_ind = (header[1] >> 6) & 0x1
        self.transport_prio = (header[1] >> 5) & 0x1
        self.pid = (((header >> 4) & 0x1F) << 8) + header[2]
        self.transport_scrambling_ctrl = (header[3] >> 6)
        self.adaptation_field_ctrl = (header[3] >> 4) & 0x3
        self.sc = header[3] & 0xF

        # Now i will use only buffer variable ("basic" header is depleted)
        self.len = 0
        if self.adaptation_field_ctrl == 0x2 or self.adaptation_field_ctrl == 0x3:
            self.len = self.buffer[4]
