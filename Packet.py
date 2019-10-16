import sys


class Packet:

    def __init__(self, packet_size, buffer):
        self.packet_size = packet_size
        self.buffer = buffer

    def parse(self):
        header = self.buffer[0:4]

        sync_byte = header[0]
        if sync_byte != 0x47:
            print("ERROR: Bad sync error!", file=sys.stderr)
            return False
        
    