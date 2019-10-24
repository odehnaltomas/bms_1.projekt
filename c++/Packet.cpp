//
// Created by odehnaltomas on 24.10.19.
//

#include "Packet.h"

Packet::Packet(uint8_t* buff, int packet_size) {
	buffer.assign(buff, buff + packet_size);
}

bool Packet::parse_header() {
	header = (struct Header *)&buffer;
	cout << hex << +(header->sync_byte);
	if(header->sync_byte != 0x47) {
		cout << "neni" << endl;
	}
	cout << "sync byte:" << hex << +header->sync_byte << endl;
	cout << "s:" << hex << +(buffer[0]) << endl;
}

uint16_t Packet::get_pid() {
	return header->pid;
}

bool Packet::get_transport_err_ind() {
	return (header->transport_error_ind ? true : false);
}

