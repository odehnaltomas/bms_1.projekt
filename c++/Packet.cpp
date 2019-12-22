//
// Created by odehnaltomas on 24.10.19.
//

#include "Packet.h"

Packet::Packet(uint8_t* buff, int packet_size) {
	buffer.assign(buff, buff + packet_size);
}

bool Packet::parse_header() {
	header.sync_byte = buffer[0];
	if(header.sync_byte != 0x47) {
		return false;
	}

	header.transport_error_ind = (buffer[1] & 0x80) >> 7;
	header.payload_unit_start = (buffer[1] & 0x40) >> 6;
	header.pid = ((buffer[1] & 0x1F) << 8) + buffer[2];
	header.adaptation_field_ctrl = (buffer[3] & 0x30) >> 4;

	uint8_t skip_len = 0;
	// Check if there is only adaptation field or adaptation field with payload.
	if(header.adaptation_field_ctrl == 2 || header.adaptation_field_ctrl == 3) {
		// Skip adaptation field.
		skip_len += buffer[4];
	}
	// Check if there is pointer in packet.
	if(header.payload_unit_start) {
		// Skip pointer.
		skip_len++;
	}
	// Check if there is only payload or adaptation field with payload.
	if(header.adaptation_field_ctrl == 1 || header.adaptation_field_ctrl == 3) {
		// Adaptation field skipped in upper condition.
		// Skip header.
		skip_len += 4;
		// Get payload.
		payload.insert(payload.begin(), buffer.begin() + skip_len, buffer.end());
	}
	return true;
}

