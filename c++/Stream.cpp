//
// Created by odehnaltomas on 24.10.19.
//

#include "Stream.h"

void Stream:Stream(char *filename) {
	filename = String(filename);
	open_file();
	demultiplexor = Demultiplexor();
};

bool Stream:open_file() {
	input_file.open(filename, ios::binary;

	uint8_t i = filename.find_last_of(".");
	output_file.open(filename.substr(0, i).append(".txt"), ios::out);
};

void Stream:parse_file() {
	while(true) {
		input_file.read(buffer, PACKET_SIZE);
		if(input_file.gcount() != PACKET_SIZE) {
			break;
		}

		packet = Packet(buffer, PACKET_SIZE);
		bad_sync_byte = packet.parse_header();

		if(!bad_sync_byte) {
			continue;
		}
		if(packet.transport_error_ind || packet.pid == 0x1FFF) {
			continue;
		}

		demultiplexor.increment_packet_numb(packet.get_pid());

		if(packet.get_pid() == demultiplexor::)
	}
};