#ifndef TINAPP_CRC_CRC_H
#define TINAPP_CRC_CRC_H

#include <memory>

#include <tina++/tina.h>
#include <tina/crc/crc_checksum.h>

namespace TURAG {
	
/**
 * @addtogroup checksums-crc
 * @{
 */
	
/** 
 * @brief C++-Interface for CRC-based checksums module
 * @copydetails checksums-crc
 */
namespace CRC8 {

#if TURAG_CRC_CRC8_ALGORITHM != 0 || defined(__DOXYGEN__)	
	
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

#endif

} // namespace CRC8

/**
 * @}
 */


/**
 * @addtogroup checksums-crc
 * @{
 */

/** 
 * @brief C++-Interface for CRC-based checksums module
 * @copydetails checksums-crc
 */
namespace CRC16 {

#if TURAG_CRC_CRC16_ALGORITHM != 0 || defined(__DOXYGEN__)	

template <typename T> _always_inline
uint16_t calculate(const T& data) {
  return turag_crc16_calculate(std::addressof(data), sizeof(T));
}

template <typename T, std::size_t N> _always_inline
uint16_t calculate(const T (&data)[N]) {
  return turag_crc16_calculate(data, N * sizeof(T));
}

_always_inline
uint16_t calculate(const void* data, std::size_t length) {
  return turag_crc16_calculate(data, length);
}



template <typename T> _always_inline
bool check(const T& data, uint16_t chksum) {
  return turag_crc16_check(std::addressof(data), sizeof(T), chksum);
}

template <typename T, std::size_t N> _always_inline
bool check(const T (&data)[N], uint16_t chksum) {
  return turag_crc16_check(data, N * sizeof(T), chksum);
}

_always_inline
bool check(const void* data, std::size_t length, uint16_t chksum) {
  return turag_crc16_check(data, length, chksum);
}

#endif

} // namespace CRC16

/**
 * @}
 */

} // namespace TURAG

#endif // TINAPP_CRC_CRC_H
