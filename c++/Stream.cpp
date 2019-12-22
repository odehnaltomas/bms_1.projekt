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

		Packet packet = Packet(buffer, PACKET_SIZE);
		bool bad_sync_byte = packet.parse_header();

		demultiplexor.increment_packet_numb();
		demultiplexor.increment_packet_num_by_pid(packet.get_pid());

		if(!bad_sync_byte) {
			continue;
		}
		if(packet.get_transport_err_ind() || packet.get_pid() == 0x1FFF) {
			continue;
		}


		if(packet.get_pid() == demultiplexor.PID_PAT && !demultiplexor.is_pat_analysed()) {
			demultiplexor.parse_pat(packet);
		}
		else if(packet.get_pid() == demultiplexor.PID_NIT && !demultiplexor.is_nit_analysed()) {
			demultiplexor.parse_nit(packet);
		}
		else if(packet.get_pid() == demultiplexor.PID_SDT && !demultiplexor.is_sdt_analysed()) {
			demultiplexor.parse_sdt(packet);
		}
		else if(demultiplexor.is_pat_analysed() && demultiplexor.check_pmt(packet.get_pid())) {
			demultiplexor.parse_pmt(packet);
		}

	}
//	demultiplexor.print_pat_data();
//	demultiplexor.print_nit_data();
//	demultiplexor.print_sdt_data();
	demultiplexor.calculate_bitrate();
	demultiplexor.bind_pids_and_counters();
	demultiplexor.save_data_to_file(output_file);
};