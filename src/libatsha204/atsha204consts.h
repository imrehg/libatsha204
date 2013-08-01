#ifndef ATSHA204CONSTS_H
#define ATSHA204CONSTS_H
//Status codes
#define ATSHA204_STATUS_SUCCES 0x00
#define ATSHA204_STATUS_CHECK_MAC_MISCOMPARE 0x01
#define ATSHA204_STATUS_PARSE_ERROR 0x03
#define ATSHA204_STATUS_EXEC_ERROR 0x0F
#define ATSHA204_STATUS_WAKE_OK 0x11
#define ATSHA204_STATUS_COMMUNICATION_ERROR 0xFF

//OpCodes
#define ATSHA204_OPCODE_DEV_REV 0x30
#define ATSHA204_OPCODE_RANDOM 0x1B
#define ATSHA204_OPCODE_READ 0x02
#define ATSHA204_OPCODE_WRITE 0x12
#define ATSHA204_OPCODE_NONCE 0x16
#define ATSHA204_OPCODE_HMAC 0x11
#define ATSHA204_OPCODE_MAC 0x08

#endif //ATSHA204CONSTS_H
