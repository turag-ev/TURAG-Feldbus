/**
 * \file crc16_mcrf4.h
 * Functions and types for CRC checks.
 *
 * Generated on Sat Nov  2 16:02:08 2013,
 * by pycrc v0.8.1, http://www.tty1.net/pycrc/
 * using the configuration:
 *    Width        = 16
 *    Poly         = 0x1021
 *    XorIn        = 0xffff
 *    ReflectIn    = True
 *    XorOut       = 0x0000
 *    ReflectOut   = True
 *    Algorithm    = table-driven
 * 
 * edited by Martin Oemus <martin@oemus.net>
 *****************************************************************************/
#ifndef __TINA_CRC_CRC_16_MCRF4_H__
#define __TINA_CRC_CRC_16_MCRF4_H__

#include <tina/tina.h>
#include <tina/bytes.h>

#ifdef __cplusplus
extern "C" {
#endif


extern const uint16_t turag_crc_crc16_table[256];
	
/** Calculates a CRC16_MCRF4-checksum with the help of a table
 * @param		data	pointer to data that is to be included in the calculation
 * @param		length	length in bytes of the given data pointer
 * @return		checksum
 * 
 */
TURAG_INLINE uint16_t turag_crc16_calculate(const void* data, size_t length) {
    uint16_t crc = 0xffff;

    while (length--) {
		crc = crc_table[(crc ^ *(uint8_t*)data) & 0xff] ^ (crc >> 8);
		data = (uint8_t*)data + 1;
	}
    return crc;
}



/** Checks data with a given CRC16_MCRF4-checksum with the help of a table
 * @param		data	pointer to data that is to be checked
 * @param		length	length in bytes of the given data pointer
 * @param		chksum	checksum used to check the data
 * @return		true on data correct, otherwise false
 * 
 */
TURAG_INLINE bool turag_crc16_check(const void* data, size_t length, uint16_t chksum) {
    if (chksum == turag_crc16_calculate(data, length)) {
	return true;
    } else {
	return false;
    }
}



/** Calculates a CRC16_MCRF4-checksum directly
 * @param		data	pointer to data that is to be included in the calculation
 * @param		length	length in bytes of the given data pointer
 * @param 		seed	initial check sum value
 * @return		checksum
 * 
 */
TURAG_INLINE uint16_t turag_crc16_calculate_direct(const void* data, unsigned size) {
	// preload checksum
	uint16_t chksum = 0xffff;
	uint8_t data_byte;
	
	uint8_t i;
	for (i=0; i < size; ++i) {
		data_byte = ((uint8_t*)data)[i] ^ turag_lowbyte(chksum);
		data_byte ^= data_byte << 4;
		
		chksum = (((uint16_t)data_byte << 8) | turag_highbyte(chksum)) ^
					(uint8_t)(data_byte >> 4) ^
					((uint16_t)data_byte << 3);
	}
	return chksum;
}


/** Checks data with a given CRC16_MCRF4-checksum directly
 * @param		data	pointer to data that is to be checked
 * @param		length	length in bytes of the given data pointer
 * @param		chksum	checksum used to check the data
 * @return		true on data correct, otherwise false
 * 
 */
TURAG_INLINE bool turag_crc16_check_direct(const void* data, unsigned size, uint16_t chksum) {
	if (chksum == turag_crc16_calculate_direct(data, size)) {
		return true;
	} else {
		return false;
	}
}

#ifdef __cplusplus
}           /* closing brace for extern "C" */
#endif

#endif      /* __TINA_CRC_CRC_16_MCRF4_H__ */
