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
        # while True:
        for i in range(0, 2):
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
                print(len(packet.payload))
                continue
            # packet.print_header()

            if packet.pid == self.demultiplexor.PID_PAT:

            # break

