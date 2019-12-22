//
// Created by odehnaltomas on 24.10.19.
//

#include <utility>
#include "Demultiplexor.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

void Demultiplexor::increment_packet_numb() {
	packet_num++;
}

void Demultiplexor::increment_packet_num_by_pid(uint16_t packet_pid) {
	packet_numb_by_pid[packet_pid]++;
}

bool Demultiplexor::parse_pat(Packet packet) {
	vector<uint8_t> payload = packet.get_payload();

	uint8_t table_id = payload[0];
	if(table_id != TABLE_ID_PAT) {
		cerr << "ERROR: Bad table_id in PAT." << endl;
		return false;
	}

	uint16_t section_length = ((payload[1] & 0xF) << 8) + payload[2];

	size_t i = 0;
	while(i < section_length) {
		// PAT header has 8B header length.
		size_t index = 8 + i;
		uint16_t program_number = (payload[index] << 8) + payload[index+1];

		index += 2;

		uint16_t network_pid = ((payload[index] & 0x1F) << 8) + payload[index+1];

		pat_data.insert(make_pair(program_number, network_pid));
		if(program_number == 0) {
			PID_NIT = network_pid;
		}

		i += 4;
	}
	pat_analysed = true;
	return true;
}

bool Demultiplexor::parse_nit(Packet packet) {
	vector<uint8_t> payload = packet.get_payload();

	uint8_t table_id = payload[0];
	if(table_id != TABLE_ID_NIT) {
		cerr << "ERROR: Bad table_id in NIT." << endl;
		return false;
	}

	network_id = (payload[3] << 8) + payload[4];

	uint16_t network_descriptors_length = ((payload[8] & 0xF) << 8) + payload[9];
	size_t skip = 10;
	parse_descriptors(skip, network_descriptors_length, payload);

	skip += network_descriptors_length;

	uint16_t transport_stream_loop_length = ((payload[skip] & 0xF) << 8) + payload[skip+1];
	skip += 2;

	size_t i = 0;
	while(i < transport_stream_loop_length) {
		skip += 4;
		uint16_t transport_descriptors_length = ((payload[skip] & 0xF) << 8) + payload[skip+1];
		skip += 2;

		parse_descriptors(skip, transport_descriptors_length, payload);

		skip += transport_descriptors_length;
		i += transport_descriptors_length + 6;
	}

	nit_analysed = true;
	return true;
}

bool Demultiplexor::parse_sdt(Packet packet) {
	vector<uint8_t> payload = packet.get_payload();

	uint16_t section_length = ((payload[1] & 0xF) << 8) + payload[2];

	bool analyse = false;
	if(packet.get_payload_unit_start() && sdt_payload.size() == 0) {
		adt_add_data = true;
	}
	if(packet.get_payload_unit_start() && sdt_payload.size() > 0) {
		analyse = true;
		adt_add_data = false;
	}

	if(adt_add_data) {
		sdt_payload.insert(sdt_payload.end(), payload.begin(), payload.end());
	}
	if(analyse) {
		sdt_analysed = true;
		size_t skip = 0;
		while(skip < section_length) {
			Service service;

			uint16_t service_id = (sdt_payload[skip+11] << 8) + sdt_payload[skip+12];
			uint16_t descriptors_loop_length = ((sdt_payload[skip+11+3] & 0xF) << 8) + sdt_payload[skip+11+4];


			skip += 5;
			parse_descriptors(skip + 11, descriptors_loop_length, sdt_payload);
			skip += descriptors_loop_length;
			if(valid) {
				service.service_id = service_id;
				service.service_provider_name = tmp_service_provider_name;
				service.service_name = tmp_service_name;
				services.push_back(service);
			}
			valid = false;

//			cout << service_id << "-" << tmp_service_provider_name << "-" << tmp_service_name << endl;
		}
	}
}

bool Demultiplexor::parse_pmt(Packet packet) {
	vector<uint8_t> payload = packet.get_payload();

	uint8_t table_id = payload[0];
	if(table_id != TABLE_ID_PMT) {
		cerr << "ERROR: Bad table_id in PMT." << endl;
		return false;
	}

	uint16_t section_length = ((payload[1] & 0xF) << 8) + payload[2];
	uint16_t program_number = (payload[3] << 8) + payload[4];
	uint16_t program_info_length = ((payload[10] & 0xF) << 8) + payload[11];

	size_t skip = program_info_length + 12;
	size_t i = 0;
	while(i < section_length) {
		uint16_t elementary_pid = ((payload[skip+i+1] & 0x1F) << 8) + payload[skip+i+2];
		uint16_t ES_info_length = ((payload[skip+i+3] & 0xF) << 8) + payload[skip+i+4];

		if(elementary_pid != 0x1FFF) {
			programs_channels[program_number].insert(elementary_pid);
		}

		i += ES_info_length + 5;
	}
}

bool Demultiplexor::check_pmt(uint16_t packet_pid) {
	if(packet_pid == PID_NIT) {
		return false;
	}
	map<uint16_t,uint16_t>::iterator iter;
	for(iter = pat_data.begin(); iter != pat_data.end(); iter++) {
		if(iter->second == packet_pid) {
			return true;
		}
	}
	return false;
}

void Demultiplexor::parse_descriptors(size_t skip, uint16_t length, vector<uint8_t> payload) {
	string service_provider_name;
	string service_name;
	size_t j = 0;
	while(j < length) {
		uint8_t descriptor_len = payload[skip+j+1];
		if(payload[skip+j] == DESCRIPTOR_NETWORK_NAME) { // From PAT
			for(size_t i = 0; i < descriptor_len; i++) {
				network_name += (char)payload[skip+j+i+2];
			}
//			cout << network_name << endl;
		}
		else if(payload[skip+j] == DESCRIPTOR_TERRESTRIAL_DELIVERY_SYSTEM) { // From PAT
			bandwith = payload[skip+j+6] >> 5;
			constellation = payload[skip+j+7] >> 6;
			code_rate = payload[skip+j+7] & 0x7;
			guard_interval = (payload[skip+j+8] >> 3) & 0x3;
		}
		else if(payload[skip+j] == DESCRIPTOR_SERVICE) {
			uint8_t service_provider_name_length = payload[skip+j+3];
			for(int i = 0; i < service_provider_name_length; i++) {
				service_provider_name += (char)payload[skip+4+j+i];
				tmp_service_provider_name = service_provider_name;
			}

			uint8_t service_name_length = payload[skip+j+4+service_provider_name_length];
			for(int i = 0; i < service_name_length; i++) {
				service_name += (char)payload[skip+5+service_provider_name_length+i];
				tmp_service_name = service_name;
				valid = true;
			}

		}
		j += descriptor_len + 2;
	}
}

void Demultiplexor::bind_pids_and_counters() {
	for(auto &service: services) {
		service.service_pid = pat_data[service.service_id];
//		cout << hex << service.service_id << endl;
		for(auto &prg_chnl_pid: programs_channels[service.service_id]) {
			if(packet_numb_by_pid.find(prg_chnl_pid) != packet_numb_by_pid.end()) {
				packet_numb_by_pid[service.service_pid] += packet_numb_by_pid[prg_chnl_pid];
			}
		}
		service.service_bitrate = ((double) packet_numb_by_pid[service.service_pid] / packet_num) * stream_bitrate;
		service.service_bitrate /= 1000 * 1000;
//		cout << service.service_bitrate << endl;
//		cout << hex << service.service_pid << ": " << dec << packet_numb_by_pid[service.service_pid] << endl;
	}
}

void Demultiplexor::calculate_bitrate() {
	double A = 49764705.88;
	double B = A * (get_bandwith_value(bandwith)/8.0);
	double C = B * get_modulation_value(constellation);
	double D = C * get_code_rate_value(code_rate);
	double j = get_guard_interval_value(guard_interval);
	stream_bitrate = D * get_guard_interval_value(guard_interval);
//	cout << dec << stream_bitrate << endl;
}

void Demultiplexor::save_data_to_file(ostream &file) {
	file << "Network name: " << network_name << endl
	     << "Network ID: " << network_id << endl
	     << "Bandwidth: " << get_str_bandwith(bandwith) << endl
	     << "Constellation: " << get_str_constellation(constellation) << endl
	     << "Guard interval: " << get_str_guard_interval(guard_interval) << endl
	     << "Code rate: " << get_str_code_rate(code_rate) << endl << endl;
	for(auto &service: services) {
		file << "0x" << hex << setw(4) << setfill('0') << service.service_pid
		<< "-" << service.service_provider_name
		<< "-" << service.service_name
		<< ": " << fixed << setprecision(2) << service.service_bitrate << " Mbps" << endl;
	}
}

string Demultiplexor::get_str_bandwith(uint8_t bit_field) {
	if(bit_field == 0) {
		return "8 MHz";
	}
	else if(bit_field == 1) {
		return "7 MHz";
	}
	else if(bit_field == 2) {
		return "6 MHz";
	}
	else if(bit_field == 3) {
		return "5 MHz";
	}
	else {
		return "Unknown";
	}
}

string Demultiplexor::get_str_constellation(uint8_t bit_field) {
	if(bit_field == 0) {
		return "QPSK";
	}
	else if(bit_field == 1) {
		return "16-QAM";
	}
	else if(bit_field == 2) {
		return "64-QAM";
	}
	else {
		return "Unknown";
	}
}

string Demultiplexor::get_str_code_rate(uint8_t bit_field) {
	if(bit_field == 0) {
		return "1/2";
	}
	else if(bit_field == 1) {
		return "2/3";
	}
	else if(bit_field == 2) {
		return "3/4";
	}
	else if(bit_field == 3) {
		return "5/6";
	}
	else if(bit_field == 4) {
		return "7/8";
	}
	else {
		return "Unknown";
	}
}

string Demultiplexor::get_str_guard_interval(uint8_t bit_field) {
	if(bit_field == 0) {
		return "1/32";
	}
	else if(bit_field == 1) {
		return "1/16";
	}
	else if(bit_field == 2) {
		return "1/8";
	}
	else {
		return "1/4";
	}
}

double Demultiplexor::get_bandwith_value(uint8_t bit_field) {
	if(bit_field == 0) {
		return 8.0;
	}
	else if(bit_field == 1) {
		return 7.0;
	}
	else if(bit_field == 2) {
		return 6.0;
	}
	else if(bit_field == 3) {
		return 5.0;
	}
}

double Demultiplexor::get_modulation_value(uint8_t bit_field) {
	if(bit_field == 0) {
		return 1.0/4.0;
	}
	else if(bit_field == 1) {
		return 1.0/2.0;
	}
	else if(bit_field == 2) {
		return 3.0/4.0;
	}
}

double Demultiplexor::get_code_rate_value(uint8_t bit_field) {
	if(bit_field == 0) {
		return 1/2.0;
	}
	else if(bit_field == 1) {
		return 2/3.0;
	}
	else if(bit_field == 2) {
		return 3/4.0;
	}
	else if(bit_field == 3) {
		return 5/6.0;
	}
	else if(bit_field == 4) {
		return 7/8.0;
	}
}

double Demultiplexor::get_guard_interval_value(uint8_t bit_field) {
	if(bit_field == 0) {
		return 32/33.0;
	}
	else if(bit_field == 1) {
		return 16/17.0;
	}
	else if(bit_field == 2) {
		return 8/9.0;
	}
	else if(bit_field == 3) {
		return 4/5.0;
	}
}

void Demultiplexor::print_pat_data() {
	map<uint16_t, uint16_t>::iterator it;

	cout << "PAT data:" << endl;
	for(it = pat_data.begin(); it != pat_data.end(); it++) {
		cout << hex << it->first << " : " << hex << it->second << endl;
	}
	cout << endl;
}

void Demultiplexor::print_nit_data() {
	cout << "NIT data:" << endl;
	cout << "Network name: " << network_name << endl
	<< "Network ID: " << network_id << endl
	<< "Bandwidth: " << get_str_bandwith(bandwith) << endl
	<< "Constellation: " << get_str_constellation(constellation) << endl
	<< "Guard interval: " << get_str_guard_interval(guard_interval) << endl
	<< "Code rate: " << get_str_code_rate(code_rate) << endl << endl;
}

void Demultiplexor::print_sdt_data() {
	cout << "SDT data:" << endl;

	for(auto &service: services) {
		cout << hex << service.service_id << "-" << service.service_provider_name << "-" << service.service_name << endl;
	}
	cout << endl;
}