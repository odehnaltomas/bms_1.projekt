//
// Created by odehnaltomas on 24.10.19.
//

#ifndef C_PACKET_H
#define C_PACKET_H

#include "misc.h"

class Packet {
public:
	Packet(uint8_t*, int);
	bool parse_header();
	vector<uint8_t>get_payload(){ return payload; };
	uint16_t get_pid(){ return header.pid; };
	bool get_payload_unit_start(){ return header.payload_unit_start; };
	bool get_transport_err_ind(){ return header.transport_error_ind; };

private:
	vector<uint8_t> buffer;
	vector<uint8_t> payload;
	void print_header();

	struct Header {
		uint8_t sync_byte: 8;
		uint8_t transport_error_ind: 1;
		uint8_t payload_unit_start: 1;
		uint8_t transport_prio: 1;
		uint16_t pid: 13;
		uint8_t transport_scrambling_ctrl: 2;
		uint8_t adaptation_field_ctrl: 2;
		uint8_t continuity_counter: 4;
	};

	Header header;

};


#endif //C_PACKET_H
