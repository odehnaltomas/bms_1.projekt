//
// Created by odehnaltomas on 24.10.19.
//

#include "Stream.h"
#include "Packet.h"
#include "Demultiplexor.h"

Stream::Stream(const char *filename) {
	Stream::filename = string(filename);
	open_file();
};

bool Stream::open_file() {
	input_file.open(filename, ios::binary);

	uint8_t i = filename.find_last_of(".");
	output_file.open(filename.substr(0, i).append(".txt"), ios::out);
};

void Stream::parse_file() {
	while(true) {
		input_file.read(reinterpret_cast<char*>(buffer), PACKET_SIZE);
		if(input_file.gcount() != PACKET_SIZE) {
			break;
		}

		cout << "stream:" << hex << +buffer[0];

		Packet packet = Packet(buffer, PACKET_SIZE);
		bool bad_sync_byte = packet.parse_header();

		if(!bad_sync_byte) {
			continue;
		}
		if(packet.get_transport_err_ind() || packet.get_pid() == 0x1FFF) {
			continue;
		}

		demultiplexor.increment_packet_numb(packet.get_pid());

//		if(packet.get_pid() == 0)
	}
};