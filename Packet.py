class Packet:

    def __init__(self, packet_size, buffer):
        self.packet_size = packet_size
        self.buffer = buffer

    def get_header(self):
        return self.buffer[0:4]
    