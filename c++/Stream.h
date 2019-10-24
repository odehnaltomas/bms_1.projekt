//
// Created by odehnaltomas on 24.10.19.
//

#ifndef C_STREAM_H
#define C_STREAM_H

#include <iostream>
#include <fstream>
#include "misc.h"
#include "Demultiplexor.h"

#define PACKET_SIZE 188

class Stream {
public:
	Stream(const char *filename);
	void parse_file();

private:
	string filename;
	Demultiplexor demultiplexor;
	ifstream input_file;
	ofstream output_file;
	uint8_t buffer[PACKET_SIZE];

	bool open_file();

};


#endif //C_STREAM_H
