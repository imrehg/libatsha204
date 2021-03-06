/*
 * libatsha204 is small library and set of tools for Amel ATSHA204 crypto chip
 *
 * Copyright (C) 2013 CZ.NIC, z.s.p.o. (http://www.nic.cz/)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "tools.h"

static const uint16_t POLYNOM = 0x8005; //TODO: Move this constant to more reasonable place

/**
 * This function gets two chars, that are representing hex string, and
 * return real byte value.
 */
unsigned char get_number_from_hex_char(char high, char low) {
	char str[3];
	str[0] = high; str[1] = low; str[2] = '\0';

	return (unsigned char)strtol(str, NULL, 16);
}

uint32_t uint32_from_4_bytes(const unsigned char *data) {
	uint32_t res = 0;

	res |= data[0]; res <<= (3 * 8);
	res |= data[1]; res <<= (2 * 8);
	res |= data[2]; res <<= (1 * 8);
	res |= data[3];

	return res;
}

bool check_packet(unsigned char *packet) {
	unsigned char packet_size;
	unsigned char crc[2];

	packet_size = packet[0];
	crc[0] = packet[packet_size-2];
	crc[1] = packet[packet_size-1];

	if (!check_crc(packet_size-2, packet, crc)) {
		return false;
	}

	return true;
}

unsigned char *generate_command_packet(unsigned char opcode, unsigned char param1, uint16_t param2, unsigned char *data, unsigned char data_count) {
	unsigned char packet_size =
		1 + //count item
		1 + //opcode
		1 + //param1
		2 + //param2
		data_count + //data count
		2; //CRC

	unsigned char crc[2];
	unsigned char *packet = calloc(packet_size, sizeof(unsigned char));
	if (packet == NULL) {
		return NULL;
	}

	packet[0] = packet_size;
	packet[1] = opcode;
	packet[2] = param1;
	packet[3] = (param2 & 0x00FF);
	packet[4] = ((param2 & 0xFF00) >> 8);
	memcpy((packet + 5), data, data_count);
	calculate_crc(packet_size - 2, packet, crc); //skip crc slot
	packet[5 + data_count] = crc[0];
	packet[5 + data_count + 1] = crc[1];

	return packet;
}

unsigned char *generate_answer_packet(unsigned char *data, unsigned char data_count) {
	unsigned char packet_size =
		1 + //count item
		data_count + //data count
		2; //CRC

	unsigned char crc[2];
	unsigned char *packet = calloc(packet_size, sizeof(unsigned char));
	if (packet == NULL) {
		return NULL;
	}

	packet[0] = packet_size;
	memcpy((packet + 1), data, data_count);
	calculate_crc(packet_size - 2, packet, crc); //skip crc slot
	packet[1 + data_count] = crc[0];
	packet[1 + data_count + 1] = crc[1];

	return packet;
}

bool check_crc(unsigned char length, unsigned char *data, unsigned char *crc) {
	unsigned char rcrc[2];

	calculate_crc(length, data, rcrc);
	if ((crc[0] != rcrc[0]) || (crc[1] != rcrc[1])) {
		return false;
	}

	return true;
}

void calculate_crc(uint16_t length, unsigned char *data, unsigned char *crc) {
	uint16_t counter;
	uint16_t crc_register = 0;
	uint16_t polynom = POLYNOM;
	unsigned char shift_register;
	unsigned char data_bit, crc_bit;

	for (counter = 0; counter < length; counter++) {
	  for (shift_register = 0x01; shift_register > 0x00; shift_register <<= 1) {
		 data_bit = (data[counter] & shift_register) ? 1 : 0;
		 crc_bit = crc_register >> 15;
		 crc_register <<= 1;
		 if (data_bit != crc_bit)
			crc_register ^= polynom;
	  }
	}
	crc[0] = (unsigned char) (crc_register & 0x00FF);
	crc[1] = (unsigned char) (crc_register >> 8);
}

void clear_buffer(unsigned char *buff, size_t len) {
	for (size_t i = 0; i < len; i++) {
		buff[i] = 0;
	}
}

void print_buffer_content(unsigned char *buff, ssize_t len) {
	fprintf(stderr, "%zd bytes:\n", len);
	for (ssize_t i = 0; i < len; i++) {
		if (!(i%10)) fprintf(stderr, "\n");
		fprintf(stderr, "%02X ", buff[i]);
	}
	fprintf(stderr, "\n");
}
