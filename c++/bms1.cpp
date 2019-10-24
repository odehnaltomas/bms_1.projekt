//
// Created by odehnaltomas on 24.10.19.
//
#include "Stream.h"
#include "misc.h"

int main(int argc, char **argv) {
	if(argc != 2) {
		cerr << "ERROR: Input file is missing." << endl;
		exit(1);
	}

	Stream stream = Stream(argv[1]);
	stream.parse_file();

}