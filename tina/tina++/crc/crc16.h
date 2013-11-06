#ifndef TINAPP_CRC_CRC16_H
#define TINAPP_CRC_CRC16_H

#include <tina/crc/crc16_mcrf4/crc16_mcrf4.h>

namespace TURAG {
namespace CRC16 {

template <typename T, std::size_t N> _always_inline
uint16_t calculate(const T (data&)[N]) {
  return turag_crc16_calculate(&data, N);
}

template <typename T, std::size_t N> _always_inline
bool check(const T (data&)[N], uint16_t chksum) {
  return turag_crc16_check(&data, N);
}

template <typename T, std::size_t N> _always_inline
uint16_t turag_crc16_calculate_direct(const T (data&)[N]) {
	return turag_crc16_calculate_direct(&data, N);
}

template <typename T, std::size_t N> _always_inline
bool turag_crc16_check_direct(const void* data, unsigned size, uint16_t chksum) {
	return turag_crc16_check_direct(&data, N, chksum);
}

} // namespace CRC16
} // namespace TURAG

#endif // TINAPP_CRC_CRC16_H
