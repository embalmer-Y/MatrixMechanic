#include <stdint.h>

#define		CRC_POLY_16		0xA001
#define		CRC_START_16    0x0000

uint16_t crc_16( const unsigned char *input_str, size_t num_bytes);
