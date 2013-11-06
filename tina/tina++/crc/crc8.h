#ifndef TINAPP_CRC_CRC8_H
#define TINAPP_CRC_CRC8_H

#include <tina++/tina.h>
#include <tina/crc/crc8_icode/crc8_icode.h>

namespace TURAG {
namespace CRC8 {

template <typename T, std::size_t N> _always_inline
uint16_t calculate(const T (&data)[N]) {
  return turag_crc8_calculate(&data, N);
}

_always_inline
uint16_t calculate(const void* data, std::size_t length) {
  return turag_crc8_calculate(data, length);
}

template <typename T, std::size_t N> _always_inline
bool check(const T (&data)[N], uint16_t chksum) {
  return turag_crc8_check(&data, N, chksum);
}

_always_inline
bool check(const void* data, std::size_t length, uint16_t chksum) {
  return turag_crc8_check(data, length, chksum);
}

} // namespace CRC8
} // namespace TURAG

#endif // TINAPP_CRC_CRC8_H
