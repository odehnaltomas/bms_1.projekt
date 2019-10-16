import sys
import os
from Stream import Stream


def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def main():
    if len(sys.argv) != 2:
        eprint("Error:", "Missing source file name!", sep=" ")
        exit(1)

    stream = Stream(sys.argv[1])
    stream.parse_file()


if __name__ == "__main__":
    main()
