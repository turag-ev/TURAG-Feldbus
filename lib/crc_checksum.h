/**
 * \file crc_checksum.h
 * Functions and types for CRC checks.
 */

#ifndef __TINA_CRC_CRC_CHECKSUMS_H__
#define __TINA_CRC_CRC_CHECKSUMS_H__


#ifdef __cplusplus
# include <cstdint>
# include <cstdbool>
#else
# include <stdint.h>
# include <stdbool.h>
#endif



#ifdef __cplusplus
extern "C" {
#endif

	
/*
 * CRC8 implementation
 */	
/* Generated on Fri Nov  1 21:06:17 2013,
 * by pycrc v0.8.1, http://www.tty1.net/pycrc/
 * using the configuration:
 *    Width        = 8
 *    Poly         = 0x1d
 *    XorIn        = 0xfd
 *    ReflectIn    = False
 *    XorOut       = 0x00
 *    ReflectOut   = False
 *    Algorithm    = table-driven
 */
extern const uint8_t turag_crc_crc8_table[256];

/** Calculates a CRC8-checksum (using CRC-8/I-CODE)
 * @param		data	pointer to data that is to be included in the calculation
 * @param		length	length in bytes of the given data pointer
 * @return		checksum
 */
static inline uint8_t turag_crc8_calculate(const void* data, size_t length) {
	uint8_t crc = 0xfd;

    while (length--) {
        crc = turag_crc_crc8_table[crc ^ *(uint8_t*)data];
		data = (uint8_t*)data + 1;
	}
    return crc;
}


/** Checks data with a given CRC8-checksum (using CRC-8/I-CODE)
 * @param		data	pointer to data that is to be checked
 * @param		length	length in bytes of the given data pointer
 * @param		chksum	checksum used to check the data
 * @return		true on data correct, otherwise false
 */
static inline bool turag_crc8_check(const void* data, size_t length, uint8_t chksum) {
    return chksum == turag_crc8_calculate(data, length);
}









#ifdef __cplusplus
}           /* closing brace for extern "C" */
#endif


#endif      /* __TINA_CRC_CRC_CHECKSUMS_H__ */
