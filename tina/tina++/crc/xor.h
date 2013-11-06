#ifndef TINAPP_CRC_XOR_H
#define TINAPP_CRC_XOR_H

#include <memory>

#include <tina++/tina.h>
#include <tina/crc/xor_checksum.h>

namespace TURAG {
namespace XOR {

template <typename T> _always_inline
uint8_t calculate(const T& data) {
	return xor_checksum_calculate(std::addressof(data), sizeof(T));
}

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

_always_inline
uint8_t check(const void* data, std::size_t length, uint8_t chksum) {
	return xor_checksum_check(data, length, chksum);
}

} // namespace XOR
} // namespace TURAG

#endif // TINAPP_CRC_XOR_H
