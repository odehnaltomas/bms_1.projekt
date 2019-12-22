CXX=g++
FLAGS=-std=c++11 -o
FILES=bms1.cpp Stream.h Stream.cpp Packet.h Packet.cpp Demultiplexor.h Demultiplexor.cpp misc.h

all: bms1

bms1: $(FILES)
	$(CXX) $(FLAGS) $^ -o bms1

clean:
	rm -f $(FILES) *.o

.PHONY: clean
