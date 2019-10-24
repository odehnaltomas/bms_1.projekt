//
// Created by odehnaltomas on 24.10.19.
//

#ifndef C_DEMULTIPLEXOR_H
#define C_DEMULTIPLEXOR_H

#include "misc.h"

#define PID_PAT 0x0000;
#define PID_SDT 0x0011;

class Demultiplexor {
public:
	uint8_t PID_NIT = 0x0010;

	void increment_packet_numb(uint16_t packet_pid);
};


#endif //C_DEMULTIPLEXOR_H
