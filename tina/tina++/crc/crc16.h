#ifndef TINAPP_CRC_CRC16_H
#define TINAPP_CRC_CRC16_H

#include <memory>

#include <tina++/tina.h>
#include <tina/crc/crc16_mcrf4/crc16_mcrf4.h>

namespace TURAG {

/**
 * @addtogroup checksums-crc
 * @{
 */
	
/** 
 * @brief C++-Interface for CRC-based checksums module
 * @copydetails checksums-crc
 */
namespace CRC16 {

/** 
 * @name table-driven
 * @{
 */

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

/**
 * @}
 */

/** 
 * @name direct
 * @{
 */
 
template <typename T> _always_inline
uint16_t calculate_direct(const T& data) {
	return turag_crc16_calculate_direct(std::adressof(data), sizeof(T));
}

template <typename T, std::size_t N> _always_inline
uint16_t calculate_direct(const T (&data)[N]) {
	return turag_crc16_calculate_direct(data, N * sizeof(T));
}

_always_inline
uint16_t calculate_direct(const void* data, std::size_t length) {
	return turag_crc16_calculate_direct(data, length);
}



template <typename T> _always_inline
bool check_direct(const T& data, uint16_t chksum) {
	return turag_crc16_check_direct(std::addressof(data), sizeof(T), chksum);
}

template <typename T, std::size_t N> _always_inline
bool check_direct(const T (&data)[N], uint16_t chksum) {
	return turag_crc16_check_direct(data, N * sizeof(T), chksum);
}

_always_inline
bool check_direct(const void* data, std::size_t length, uint16_t chksum) {
	return turag_crc16_check_direct(data, length, chksum);
}

/**
 * @}
 */

} // namespace CRC16

/**
 * @}
 */
	
} // namespace TURAG

#endif // TINAPP_CRC_CRC16_H
