//
// Created by odehnaltomas on 24.10.19.
//
#include "Stream.h"

int main(int argc, char **argv) {
	if(argc != 2) {
		printf("ERROR: Input file is missing.");
		exit(1)
	}

	Stream stream = new Stream(argv[1]);
	stream.parse_file()

}