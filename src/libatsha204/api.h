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

#ifndef API_H
#define API_H

#include <stdio.h>
#include <stdint.h>

/**
 * \file api.h
 * \brief Definition of internal structures
 */

/**
 * \brief Global configuration of library
 */
typedef struct {
	bool verbose; ///<Enable verbose mode
	void (*log_callback)(const char* msg); ///<Callback for error reporting
} atsha_configuration;

/**
 * \brief Instance of library
 *
 * There may be several parallel instances
 */
struct atsha_handle {
	int bottom_layer; ///<What kind of bottom layer is used
	bool is_srv_emulation; ///<Server-side or client-side emulation?
	int fd;  ///<File descriptor of binary file (e.g. USB layer file)
	FILE *file; ///<Text file handler, mainly for emulation
	int lockfile; ///<File descriptor of lock file
	struct mpsse_context *i2c; ///<Instance of libmpsse library
	unsigned char *sn; ///<Serial number for server-side emulation and for caching
	unsigned char *key; ///<Key for server-side emulation
	uint32_t key_origin; ///<Cached key origin value
	bool key_origin_cached; ///<It key origin value cached?
	unsigned char slot_id; ///<Cached key origin value that is read from OTP memory
	unsigned char nonce[32]; ///<Emulation of TempKey memory slot
};

#define BOTTOM_LAYER_EMULATION 0
#define BOTTOM_LAYER_NI2C 1
#define BOTTOM_LAYER_I2C 2
#define BOTTOM_LAYER_USB 3
#define DNS_ERR_CONST 255

/**
 * \brief Use callback (from global configuration) and send message through it
 */
void log_message(const char* msg);
#endif //MAIN_H
