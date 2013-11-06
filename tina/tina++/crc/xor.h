#ifndef TINAPP_CRC_XOR_H
#define TINAPP_CRC_XOR_H

#include <tina/crc/xor_checksum.h>

namespace TURAG {
namespace XOR {

/** Calculates a simple 8Bit-XOR-checksum
 * @param		data	pointer to data that is to be included in the calculation
 * @param		length	length in bytes of the given data pointer
 * @return		checksum
 *
 */
template <typename T, std::size_t N>
_always_inline uint8_t calculate(const T (data&)[N]) {
	return xor_checksum_calculate(data, N);
}

/** Checks data with a given CRC16-checksum
 * @param		data	pointer to data that is to be checked
 * @param		length	length in bytes of the given data pointer
 * @param		chksum	checksum used to check the data
 * @return		true on data correct, otherwise false
 *
 */
template <typename T, std::size_t N>
_always_inline bool check(const T (data&)[N], uint8_t chksum) {
	return xor_checksum_check(data, N, chksum);
}

} // namespace XOR
} // namespace TURAG

#endif // TINAPP_CRC_XOR_H
