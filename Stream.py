from Packet import Packet
from Demultiplexor import Demultiplexor

PACKET_SIZE = 188


class Stream:
    filename = ""

    def __init__(self, filename):
        self.filename = filename
        self.in_file = self.open_file()
        self.out_file = self.open_file(out=True)
        self.demultiplexor = Demultiplexor()

    def open_file(self, out=False):
        if not out:
            try:
                return open(self.filename, 'rb')
            except OSError as e:
                print("ERROR: File \"" + self.filename + "\" cannot be opened.")
                exit(2)
        filename = self.filename.replace('.ts', '.txt')
        return open(filename, 'w')

    def parse_file(self):
        while True:
            buffer = self.in_file.read(PACKET_SIZE)
            if buffer == '':
                break
            elif len(buffer) != PACKET_SIZE:
                break

            packet = Packet(PACKET_SIZE, buffer)
            bad_sync_byte = packet.parse()

            if not bad_sync_byte:
                continue

            self.demultiplexor.packet_num += 1

            # If there is error in the packet or pid is 0x1fff => skip this packet
            if packet.transport_error_ind or packet.pid == 0x1fff:
                continue
            # packet.print_header()

            if packet.pid == self.demultiplexor.PID_PAT and not self.demultiplexor.PAT_analysed:
                self.demultiplexor.parse_PAT(packet)
                # break
            elif packet.pid == self.demultiplexor.PID_NIT and not self.demultiplexor.NIT_analysed:
                self.demultiplexor.parse_NIT(packet)
                # break
            elif packet.pid == self.demultiplexor.PID_SDT:
                self.demultiplexor.parse_SDT(packet)
                # break
        # TODO: add
