#include "atsha204.h"

const char *atsha_error_name(int err) {
	if (err == ATSHA_ERR_OK) {
		return "OK";
	} else if (err == ATSHA_ERR_MEMORY_ALLOCATION_ERROR) {
		return "Memory allocation error. Some memory allocation failed.";
	} else if (err == ATSHA_ERR_INVALID_INPUT) {
		return "Invalid input parameter.";
	} else if (err == ATSHA_ERR_COMMUNICATION) {
		return "Communication error: is not possible to send packet to the device, receive packet from the device, or multiple times was delivered/received malformed packet.";
	} else if (err == ATSHA_ERR_BAD_COMMUNICATION_STATUS) {
		return "Communication crashed due to bad status code from ATSHA204 device. Please, use verbose mode for more information.";
	} else if (err == ATSHA_ERR_NOT_IMPLEMENTED) {
		return "Operation not implemented. Requested function or feature in not implemented in applicable layer (yet).";
	} else if (err == ATSHA_ERR_WAKE_NOT_CONFIRMED) {
		return "Is not confirmed if device is wake up or not.";
	} else {
		return "Error code is not in the list";
	}
}