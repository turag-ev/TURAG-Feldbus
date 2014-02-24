/**
 *  @file		tina++/crc/xor.h
 *  @brief		C++-Interface to xor checksum calculation routines
 * 
 * bsdlfkl
 */



#ifndef TINAPP_CRC_XOR_H
#define TINAPP_CRC_XOR_H

#include <memory>

#include <tina++/tina.h>
#include <tina/crc/xor_checksum.h>

namespace TURAG {
	
	
/** @namespace XOR
 * Namespace-test
 */
namespace XOR {
	
/**
 * @ingroup checksums-xor
 * @{
 */


/**
 * @brief Calculates a simple 8Bit-XOR-checksum.
 * @param[in] data Object to calculate checksum of
 * @return checksum
 * test
 */
template <typename T> _always_inline
uint8_t calculate(const T& data) {
	return xor_checksum_calculate(std::addressof(data), sizeof(T));
}

/** test
 * 
 */
template <typename T, std::size_t N> _always_inline
uint8_t calculate(const T (&data)[N]) {
	return xor_checksum_calculate(data, N * sizeof(T));
}

_always_inline
uint8_t calculate(const void* data, std::size_t length) {
	return xor_checksum_calculate(data, length);
}



template <typename T> _always_inline
bool check(const T& data, uint8_t chksum) {
	return xor_checksum_check(std::addressof(data), sizeof(T), chksum);
}

template <typename T, std::size_t N> _always_inline
bool check(const T (&data)[N], uint8_t chksum) {
	return xor_checksum_check(data, N * sizeof(T), chksum);
}

/**
 * @brief Checks data with a given 8Bit-XOR-checksum.
 * @param[in]	data	pointer to data that is to be checked
 * @param[in]	length	length in bytes of the given data pointer
 * @param[in]	chksum	checksum used to check the data
 */
_always_inline
uint8_t check(const void* data, std::size_t length, uint8_t chksum) {
	return xor_checksum_check(data, length, chksum);
}

/**
 * @}
 */


} // namespace XOR
} // namespace TURAG


#endif // TINAPP_CRC_XOR_H
