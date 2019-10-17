class Demultiplexor:
    PID_PAT = 0x0000
    PID_NIT = 0x0010
    PID_SDT = 0x0011

    def __init__(self):
        self.packet_num = 0

    def parse_PAT(self, packet):
        table_id = packet.payload[0]
        