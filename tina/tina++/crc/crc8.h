#ifndef TINAPP_CRC_CRC8_H
#define TINAPP_CRC_CRC8_H

#include <memory>

#include <tina++/tina.h>
#include <tina/crc/crc8_icode/crc8_icode.h>

namespace TURAG {
namespace CRC8 {

template <typename T> _always_inline
uint16_t calculate(const T& data) {
  return turag_crc8_calculate(std::addressof(data), sizeof(T));
}

template <typename T, std::size_t N> _always_inline
uint16_t calculate(const T (&data)[N]) {
  return turag_crc8_calculate(data, N * sizeof(T));
}

_always_inline
uint16_t calculate(const void* data, std::size_t length) {
  return turag_crc8_calculate(data, length);
}



template <typename T> _always_inline
bool check(const T& data, uint16_t chksum) {
  return turag_crc8_check(std::addressof(data), sizeof(T), chksum);
}

template <typename T, std::size_t N> _always_inline
bool check(const T (&data)[N], uint16_t chksum) {
  return turag_crc8_check(data, N * sizeof(T), chksum);
}

_always_inline
bool check(const void* data, std::size_t length, uint16_t chksum) {
  return turag_crc8_check(data, length, chksum);
}

} // namespace CRC8
} // namespace TURAG

#endif // TINAPP_CRC_CRC8_H
