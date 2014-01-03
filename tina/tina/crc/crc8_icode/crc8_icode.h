/**
 * \file crc-8_icode.h
 * Functions and types for CRC checks.
 *
 * Generated on Fri Nov  1 21:06:17 2013,
 * by pycrc v0.8.1, http://www.tty1.net/pycrc/
 * using the configuration:
 *    Width        = 8
 *    Poly         = 0x1d
 *    XorIn        = 0xfd
 *    ReflectIn    = False
 *    XorOut       = 0x00
 *    ReflectOut   = False
 *    Algorithm    = table-driven
 * 
 * edited by Martin Oemus <martin@oemus.net>
 *****************************************************************************/
#ifndef __TINA_CRC_CRC_8_ICODE_H__
#define __TINA_CRC_CRC_8_ICODE_H__

#include <tina/tina.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const uint8_t* turag_crc_crc8_table;


/** Calculates a CRC8-checksum
 * @param		data	pointer to data that is to be included in the calculation
 * @param		length	length in bytes of the given data pointer
 * @return		checksum
 * 
 */
TURAG_INLINE uint8_t turag_crc8_calculate(const void* data, size_t length) {
	uint8_t crc = 0xfd;

    while (length--) {
		crc = turag_crc_crc8_table[crc ^ *(uint8_t*)data];
		data = (uint8_t*)data + 1;
	}
    return crc;
}



/** Checks data with a given CRC8-checksum
 * @param		data	pointer to data that is to be checked
 * @param		length	length in bytes of the given data pointer
 * @param		chksum	checksum used to check the data
 * @return		true on data correct, otherwise false
 * 
 */
TURAG_INLINE bool turag_crc8_check(const void* data, size_t length, uint8_t chksum) {
    if (chksum == turag_crc8_calculate(data, length)) {
		return true;
    } else {
		return false;
    }
}



#ifdef __cplusplus
}           /* closing brace for extern "C" */
#endif

#endif      /* __TINA_CRC_CRC_8_ICODE_H__ */
