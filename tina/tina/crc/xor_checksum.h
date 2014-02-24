/**
 *  @file		xor_checksum.h
 *  @date		11.12.2012
 *  @author		martin
 *  @brief		Provides xor checksum calculation
 */


#ifndef TINA_UTIL_XOR_CHECKSUM_H_
#define TINA_UTIL_XOR_CHECKSUM_H_


#include <tina/tina.h>


#ifdef __cplusplus
extern "C" {
#endif

	
/**
 * @ingroup checksums-xor
 * @{
 */


/** 
 * @brief Calculates a simple 8Bit-XOR-checksum.
 * @param[in]	data	pointer to data that is to be included in the calculation
 * @param[in]	size	length in bytes of the given data pointer
 * @return		checksum
 *
 */
TURAG_INLINE uint8_t xor_checksum_calculate(const void* data, unsigned size) {
	uint8_t checksum = 0;

	uint8_t i;
	for (i=0; i < size; ++i) checksum ^= ((const uint8_t*)data)[i];

	return checksum;
}

/** 
 * @brief Checks data with a given 8Bit-XOR-checksum.
 * @param[in]	data	pointer to data that is to be checked
 * @param[in]	size	length in bytes of the given data pointer
 * @param[in]	chksum	checksum used to check the data
 * @return		true on data correct, otherwise false
 *
 */
TURAG_INLINE bool xor_checksum_check(const void* data, unsigned size, uint8_t chksum) {
	if (chksum == xor_checksum_calculate(data, size)) {
		return true;
	} else {
		return false;
	}
}

/**
 * @}
 */


#ifdef __cplusplus
}
#endif


#endif /* TINA_UTIL_XOR_CHECKSUM_H_ */
