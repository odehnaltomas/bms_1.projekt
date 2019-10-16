from Packet import Packet

PACKET_SIZE = 188


class Stream:
    filename = ""

    def __init__(self, filename):
        self.filename = filename
        self.in_file = self.open_file()
        self.out_file = self.open_file(out=True)

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
            # print(packet.get_header())
            break

