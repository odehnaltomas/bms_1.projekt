//
// Created by odehnaltomas on 24.10.19.
//

#ifndef C_DEMULTIPLEXOR_H
#define C_DEMULTIPLEXOR_H

#include <map>
#include "misc.h"
#include "Packet.h"
#include <set>
#include <fstream>

class Demultiplexor {
public:
	static const uint16_t PID_PAT = 0x0000;
	uint16_t PID_NIT = 0x0010;
	static const uint16_t PID_SDT = 0x0011;

	static const uint8_t TABLE_ID_PAT = 0x00;
	static const uint8_t TABLE_ID_PMT = 0x02;
	static const uint8_t TABLE_ID_NIT = 0x40;
	static const uint8_t TABLE_ID_SDT = 0x42;

	static const uint8_t DESCRIPTOR_NETWORK_NAME = 0x40;
	static const uint8_t DESCRIPTOR_TERRESTRIAL_DELIVERY_SYSTEM = 0x5A;
	static const uint8_t DESCRIPTOR_SERVICE = 0X48;

	void increment_packet_numb();
	void increment_packet_num_by_pid(uint16_t packet_pid);

	bool parse_pat(Packet packet);
	bool parse_nit(Packet packet);
	bool parse_sdt(Packet packet);
	bool check_pmt(uint16_t pid);
	bool parse_pmt(Packet packet);

	bool is_pat_analysed(){return pat_analysed;};
	bool is_nit_analysed(){return nit_analysed;};
	bool is_sdt_analysed(){return sdt_analysed;};

	void print_pat_data();
	void print_nit_data();
	void print_sdt_data();
	void print_pmt_data();

	void bind_pids_and_counters();

	void calculate_bitrate();
	void save_data_to_file(ostream& file);

private:
	// Total number of packets
	uint32_t packet_num = 0;

	// Number of packets by packet PID
	// <PID, number of packets>
	map<uint16_t, uint32_t> packet_numb_by_pid;

	map<uint16_t, set<uint16_t>> programs_channels;

	bool pat_analysed;
	bool nit_analysed;
	bool sdt_analysed;

	bool adt_add_data;

	// <program number, program map PID>
	// program number == service.service_id
	map<uint16_t, uint16_t> pat_data;
	vector<uint8_t> sdt_payload;

	string network_name;
	uint16_t network_id;
	uint8_t bandwith;
	uint8_t constellation;
	uint8_t code_rate;
	uint8_t guard_interval;
	unsigned long stream_bitrate;

	void parse_descriptors(size_t skip, uint16_t length, vector<uint8_t> payload);
	string get_str_bandwith(uint8_t bit_field);
	string get_str_constellation(uint8_t bit_field);
	string get_str_code_rate(uint8_t bit_field);
	string get_str_guard_interval(uint8_t bit_field);

	double get_bandwith_value(uint8_t bit_field);
	double get_modulation_value(uint8_t bit_field);
	double get_code_rate_value(uint8_t bit_field);
	double get_guard_interval_value(uint8_t bit_field);

	struct Service {
		uint16_t service_pid;
		uint16_t service_id;
		string service_provider_name;
		string service_name;
		double service_bitrate;
	};

	vector<Service> services;
	bool valid = false;
	string tmp_service_provider_name;
	string tmp_service_name;
};


#endif //C_DEMULTIPLEXOR_H
