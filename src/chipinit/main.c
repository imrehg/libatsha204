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
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "../libatsha204/atsha204.h"
#include "../libatsha204/atsha204consts.h"
#include "../libatsha204/tools.h"

#define BUFFSIZE_LINE 128
#define BYTESIZE_KEY 32
#define BYTESIZE_OTP 4
#define BYTESIZE_CNF 4
#define SLOT_CNT 16
#define CONFIG_CNT 22

#define SLOT_CONFIG_READ 0x80
#define SLOT_CONFIG_WRITE 0x80

#define ERR_INIT 1
#define ERR_CNF_READ 2
#define ERR_LOCK 3

void log_callback(const char *msg) {
	fprintf(stderr, "Log: %s\n", msg);
}

static bool read_config(FILE *conf, unsigned char *data, size_t line_cnt) {
	char line[BUFFSIZE_LINE];

	for (size_t item = 0; item < SLOT_CNT; item++) {
		if (fgets(line, BUFFSIZE_LINE, conf) == NULL) {
			return false;
		}

		char *line_p = line;
		char *line_end_p = (line_p + strlen(line_p));

		size_t i = 0;
		while (i < line_cnt) {
			if (line_p[0] == ' ' || line_p[0] == '\t' || line_p[0] == ';' || line_p[0] == ',' || line_p[0] == ':') {
				line_p++;
				continue;
			}

			(data+(line_cnt*item))[i++] = get_number_from_hex_char(line_p[0], line_p[1]);
			line_p += 2;

			if (line_p >= line_end_p) {
				return false;
			}
		}
	}

	return true;
}

static bool set_otp_mode(struct atsha_handle *handle, unsigned char *config) {
	atsha_big_int record;

	if (atsha_raw_conf_read(handle, 0x04, &record) != ATSHA_ERR_OK) return false;
	record.data[2] = ATSHA204_CONFIG_OTPMODE_READONLY;
	if (atsha_raw_conf_write(handle, 0x04, record) != ATSHA_ERR_OK) return false;

	config[0x04*BYTESIZE_CNF+2] = ATSHA204_CONFIG_OTPMODE_READONLY;

	return true;

}

static bool set_slot_config(struct atsha_handle *handle, unsigned char *config) {
	atsha_big_int record;

	for (unsigned char addr = 0x05; addr <= 0x0C; addr++) {
		if (atsha_raw_conf_read(handle, addr, &record) != ATSHA_ERR_OK) return false;
		record.data[0] = SLOT_CONFIG_READ;
		record.data[1] = SLOT_CONFIG_WRITE;
		record.data[2] = SLOT_CONFIG_READ;
		record.data[3] = SLOT_CONFIG_WRITE;
		if (atsha_raw_conf_write(handle, addr, record) != ATSHA_ERR_OK) return false;

		config[addr*BYTESIZE_CNF+0] = SLOT_CONFIG_READ;
		config[addr*BYTESIZE_CNF+1] = SLOT_CONFIG_WRITE;
		config[addr*BYTESIZE_CNF+2] = SLOT_CONFIG_READ;
		config[addr*BYTESIZE_CNF+3] = SLOT_CONFIG_WRITE;
	}

	return true;
}

static bool create_and_lock_config(struct atsha_handle *handle) {
	unsigned char config[CONFIG_CNT*BYTESIZE_CNF];
	unsigned char crc[2];
	atsha_big_int record;

	size_t item = 0;
	record.bytes = BYTESIZE_CNF;
	for (unsigned char addr = 0x00; addr <= 0x15; addr++) {
		if (atsha_raw_conf_read(handle, addr, &record) != ATSHA_ERR_OK) return false;
		memcpy((config + (item * BYTESIZE_CNF)), record.data, record.bytes);
		item++;
	}

	if (!set_otp_mode(handle, config)) return false;
	if (!set_slot_config(handle, config)) return false;

	calculate_crc(CONFIG_CNT*BYTESIZE_CNF, config, crc);
	if (atsha_lock_config(handle, crc) != ATSHA_ERR_OK) return false;

	return true;
}

static bool write_and_lock_data(struct atsha_handle *handle, unsigned char *data, unsigned char *otp) {
	atsha_big_int number;

	//Write keys into chip
	number.bytes = BYTESIZE_KEY;
	for (size_t item = 0; item < SLOT_CNT; item++) {
		memcpy(number.data, (data + (item * BYTESIZE_KEY)), number.bytes);
		if (atsha_raw_slot_write(handle, item, number) != ATSHA_ERR_OK) return false;
	}

	//Write OTP items into chip
	size_t item = 0;
	number.bytes = BYTESIZE_OTP;
	for (unsigned char addr = 0x00; addr <= 0x0F; addr++) {
		memcpy(number.data, (otp + (item * BYTESIZE_OTP)), number.bytes);
		item++;
		if (atsha_raw_otp_write(handle, addr, number) != ATSHA_ERR_OK) return false;
	}

	unsigned char crc[2];
	unsigned char both[(SLOT_CNT*BYTESIZE_KEY)+(SLOT_CNT*BYTESIZE_OTP)];

	memcpy(both, data, (SLOT_CNT*BYTESIZE_KEY));
	memcpy((both+(SLOT_CNT*BYTESIZE_KEY)), otp, (SLOT_CNT*BYTESIZE_OTP));

	calculate_crc((SLOT_CNT*BYTESIZE_KEY)+(SLOT_CNT*BYTESIZE_OTP), both, crc);
	if (atsha_lock_data(handle, crc) != ATSHA_ERR_OK) return false;

	return true;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s config\n", argv[0]);
		return ERR_INIT;
	}

	FILE *conf = fopen(argv[1], "r");
	if (conf == NULL) {
		fprintf(stderr, "Couldn't open config file %s\n", argv[1]);
		return ERR_INIT;
	}
	//init LIBATSHA204
	atsha_set_verbose();
	atsha_set_log_callback(log_callback);

	//Create LIBATSHA204 handler
	struct atsha_handle *handle = atsha_open();
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open I2C devidce.\n");
		return ERR_INIT;
	}

	//Prepare data structures
	unsigned char data[SLOT_CNT*BYTESIZE_KEY];
	unsigned char otp[SLOT_CNT*BYTESIZE_OTP];

	//Read keys
	if (!read_config(conf, data, BYTESIZE_KEY)) {
		fprintf(stderr, "Couldn't read config data (keys).\n");
		return ERR_CNF_READ;
	}

	//Read OTP items
	if (!read_config(conf, otp, BYTESIZE_OTP)) {
		fprintf(stderr, "Couldn't read config data (OTP).\n");
		return ERR_CNF_READ;
	}

	if (create_and_lock_config(handle)) {
		printf("Configuration is locked\n");
	} else {
		printf("Configuration is NOT locked\n");
		return ERR_LOCK;
	}

	//Write data
	if (write_and_lock_data(handle, data, otp)) {
		printf("Data and OTP zones are locked\n");
	} else {
		printf("Data and OTP zones are NOT locked\n");
		return ERR_LOCK;
	}

	fclose(conf);
	atsha_close(handle);

	return 0;
}
