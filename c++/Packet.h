//
// Created by odehnaltomas on 24.10.19.
//

#ifndef C_PACKET_H
#define C_PACKET_H


class Packet {
public:
	void Packet();
	bool parse_header(const uint8_t *buffer);

private:
	uint8_t
	print_header()

};


#endif //C_PACKET_H
