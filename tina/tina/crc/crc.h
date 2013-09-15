/**
 *  @file		crc.h
 *  @date		11.12.2012
 *  @author		martin
 *  @brief		Provides crc error checking functions
 * 
 *  
 */

#ifndef UTIL_CRC_H_
#define UTIL_CRC_H_

#ifdef __cplusplus
extern "C" {
#endif
// add definitions below this comment
// ---------------------------------------------- 

#include <util/types.h>
#include <util/makros.h>



/** Calculates a simple 8Bit-XOR-checksum
 * @param		data	pointer to data that is to be included in the calculation
 * @param		length	length in bytes of the given data pointer
 * @return		checksum
 *
 */
__attribute__((always_inline)) static inline uint8_t xor_checksum_calculate(void* data, unsigned size) {
	uint8_t checksum = 0;

	uint8_t i;
	for (i=0; i < size; ++i) checksum ^= ((uint8_t*)data)[i];

	return checksum;
}

/** Checks data with a given CRC16-checksum
 * @param		data	pointer to data that is to be checked
 * @param		length	length in bytes of the given data pointer
 * @param		chksum	checksum used to check the data
 * @return		true on data correct, otherwise false
 *
 */
__attribute__((always_inline)) static inline bool xor_checksum_check(void* data, unsigned size, uint8_t chksum) {
	if (chksum == xor_checksum_calculate(data, size)) {
		return true;
	} else {
		return false;
	}
}


/** Calculates a CRC16-checksum
 * @param		data	pointer to data that is to be included in the calculation
 * @param		length	length in bytes of the given data pointer
 * @param 		seed	initial check sum value
 * @return		checksum
 * 
 */
static inline uint16_t crc_ccitt_calculate(void* data, unsigned size, uint16_t seed) {
	// preload checksum
	uint16_t chksum = seed;
	uint8_t data_byte;
	
	uint8_t i;
	for (i=0; i < size; ++i) {
		data_byte = ((uint8_t*)data)[i] ^ lowbyte(chksum);
		data_byte ^= data_byte << 4;
		
		chksum = (((uint16_t)data_byte << 8) | highbyte(chksum)) ^
					(uint8_t)(data_byte >> 4) ^
					((uint16_t)data_byte << 3);
	}
	return chksum;
}


/** Checks data with a given CRC16-checksum
 * @param		data	pointer to data that is to be checked
 * @param		length	length in bytes of the given data pointer
 * @param		chksum	checksum used to check the data
 * @return		true on data correct, otherwise false
 * 
 */
static inline bool crc_ccitt_check(void* data, unsigned size, uint16_t chksum, uint16_t seed) {
	if (chksum == crc_ccitt_calculate(data, size, seed)) {
		return true;
	} else {
		return false;
	}
}

// ----------------------------------------------
// don't add anything below this comment
#ifdef __cplusplus
}
#endif


#endif /* UTIL_CRC_H_ */
