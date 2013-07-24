#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>

#include "tools.h"

static const uint16_t POLYNOM = 0x8005; //TODO: Move this constant to more reasonable place

bool check_crc(unsigned char length, unsigned char *data, unsigned char *crc) {
	unsigned char rcrc[2];

	calculate_crc(length, data, rcrc);
	if ((crc[0] != rcrc[0]) || (crc[1] != rcrc[1])) {
		return false;
	}

	return true;
}

void calculate_crc(unsigned char length, unsigned char *data, unsigned char *crc) {
	unsigned char counter;
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
		fprintf(stderr, "0x%02X (%c) ", buff[i], buff[i]);
	}
	fprintf(stderr, "\n");
}
