/**
 * \file crc_checksum.h
 * Functions and types for CRC checks.
 */

#ifndef __TINA_CRC_CRC_CHECKSUMS_H__
#define __TINA_CRC_CRC_CHECKSUMS_H__

#include <tina/tina.h>

#if !TURAG_CRC_CRC8_ALGORITHM && !TURAG_CRC_CRC16_ALGORITHM
# warning Either TURAG_CRC_CRC8_ALGORITHM or TURAG_CRC_CRC16_ALGORITHM must be defined to non-zero
#endif


/*
 * documentation
 */

#ifdef __DOXYGEN__

/**
 * @addtogroup checksums-crc
 * @{
 */
 
/** Calculates a CRC8-checksum (using CRC-8/I-CODE)
 * @param		data	pointer to data that is to be included in the calculation
 * @param		length	length in bytes of the given data pointer
 * @return		checksum
 */
uint8_t turag_crc8_calculate(const void* data, size_t length);


/** Checks data with a given CRC8-checksum (using CRC-8/I-CODE)
 * @param		data	pointer to data that is to be checked
 * @param		length	length in bytes of the given data pointer
 * @param		chksum	checksum used to check the data
 * @return		true on data correct, otherwise false
 */
bool turag_crc8_check(const void* data, size_t length, uint8_t chksum);

/** Calculates a CRC8-checksum (using Maxim 1-Wire CRC)
 * @param		data	pointer to data that is to be included in the calculation
 * @param		length	length in bytes of the given data pointer
 * @return		checksum
 */
uint8_t turag_crc8_mow_calculate(const void* data, size_t length);


/** Checks data with a given CRC8-checksum (using Maxim 1-Wire CRC)
 * @param		data	pointer to data that is to be checked
 * @param		length	length in bytes of the given data pointer
 * @param		chksum	checksum used to check the data
 * @return		true on data correct, otherwise false
 */
bool turag_crc8_mow_check(const void* data, size_t length, uint8_t chksum);

/** Calculates a CRC16_MCRF4-checksum.
 * @param[in]	data	pointer to data that is to be included in the calculation
 * @param[in]	length	length in bytes of the given data pointer
 * @return		checksum
 */
uint16_t turag_crc16_calculate(const void* data, size_t length);

/** Checks data with a given CRC16_MCRF4-checksum.
 * @param[in]	data	pointer to data that is to be checked
 * @param[in]	length	length in bytes of the given data pointer
 * @param[in]	chksum	checksum used to check the data
 * @return		true on data correct, otherwise false
 */
bool turag_crc16_check(const void* data, size_t length, uint16_t chksum);

/**
 * @}
 */

#else // __DOXYGEN



#ifdef __cplusplus
extern "C" {
#endif

	
/*
 * CRC8 implementation
 */	
#if TURAG_CRC_INLINED_CALCULATION

# if TURAG_CRC_CRC8_ALGORITHM == 1
/* Generated on Fri Nov  1 21:06:17 2013,
 * by pycrc v0.8.1, http://www.tty1.net/pycrc/
 * using the configuration:
 *    Width        = 8
 *    Poly         = 0x1d
 *    XorIn        = 0xfd
 *    ReflectIn    = False
 *    XorOut       = 0x00
 *    ReflectOut   = False
 *    Algorithm    = table-driven
 */
extern const uint8_t turag_crc_crc8_table[256];

TURAG_INLINE uint8_t turag_crc8_calculate(const void* data, size_t length) {
	uint8_t crc = 0xfd;

    while (length--) {
        crc = turag_crc_crc8_table[crc ^ *(const uint8_t*)data];
        data = (const uint8_t*)data + 1;
	}
    return crc;
}
# endif


#if TURAG_CRC_CRC8_ALGORITHM == 2
/* Generated on Mon Jan 19 21:14:24 2015,
 * by pycrc v0.8.2, http://www.tty1.net/pycrc/
 * using the configuration:
 *    Width        = 8
 *    Poly         = 0x1d
 *    XorIn        = 0xfd
 *    ReflectIn    = False
 *    XorOut       = 0x00
 *    ReflectOut   = False
 *    Algorithm    = bit-by-bit-fast
 */
TURAG_INLINE uint8_t turag_crc8_calculate(const void* data_, size_t length) {
	uint8_t crc = 0xfd;
	uint8_t i;
    uint8_t bit;
    uint8_t c;
    const uint8_t* data = (const uint8_t*)data_;

    while (length--) {
        c = *data++;
        for (i = 0x80; i > 0; i >>= 1) {
            bit = crc & 0x80;
            if (c & i) {
                bit = !bit;
            }
            crc <<= 1;
            if (bit) {
                crc ^= 0x1d;
            }
        }
        crc &= 0xff;
    }
	return crc;
}
# endif


#if TURAG_CRC_CRC8_ALGORITHM == 3
/* Generated on Mon Jan 19 21:14:24 2015,
 * by pycrc v0.8.2, http://www.tty1.net/pycrc/
 * using the configuration:
 *    Width        = 8
 *    Poly         = 0x1d
 *    XorIn        = 0xfd
 *    ReflectIn    = False
 *    XorOut       = 0x00
 *    ReflectOut   = False
 *    Algorithm    = table-driven
 */
extern const uint8_t turag_crc_crc8_table[16];

TURAG_INLINE uint8_t turag_crc8_calculate(const void* data_, size_t length) {
	uint8_t crc = 0xfd;
	uint8_t tbl_idx;
    const uint8_t* data = (const uint8_t*)data_;

    while (length--) {
        tbl_idx = (crc >> 4) ^ (*data >> 4);
        crc = turag_crc_crc8_table[tbl_idx & 0x0f] ^ (crc << 4);
        tbl_idx = (crc >> 4) ^ (*data >> 0);
        crc = turag_crc_crc8_table[tbl_idx & 0x0f] ^ (crc << 4);

        data++;
    }
    return crc;
}
# endif

#else
# if TURAG_CRC_CRC8_ALGORITHM != 0
uint8_t turag_crc8_calculate(const void* data, size_t length);
# endif
#endif


#if TURAG_CRC_CRC8_ALGORITHM != 0
// the check function is always inline
TURAG_INLINE bool turag_crc8_check(const void* data, size_t length, uint8_t chksum) {
    return chksum == turag_crc8_calculate(data, length);
}
#endif





/*
 * CRC8 Maxim 1-Wire CRC implementation
 */	
#if TURAG_CRC_INLINED_CALCULATION

# if TURAG_CRC_CRC8_MOW_ALGORITHM == 1
/* Generated on Sat Aug  8 01:03:18 2015,
 * by pycrc v0.8.2, http://www.tty1.net/pycrc/
 * using the configuration:
 *    Width        = 8
 *    Poly         = 0x31
 *    XorIn        = 0x00
 *    ReflectIn    = True
 *    XorOut       = 0x00
 *    ReflectOut   = True
 *    Algorithm    = table-driven
 */
extern const uint8_t turag_crc_crc8_mow_table[256];

TURAG_INLINE uint8_t turag_crc8_mow_calculate(const void* data, size_t length) {
	uint8_t crc = 0x00;

    while (length--) {
        crc = turag_crc_crc8_mow_table[crc ^ *(const uint8_t*)data];
        data = (const uint8_t*)data + 1;
	}
    return crc;
}
# endif


#if TURAG_CRC_CRC8_MOW_ALGORITHM == 2
# error not implemented
# endif


#if TURAG_CRC_CRC8_MOW_ALGORITHM == 3
# error not implemented
# endif

#else
# if TURAG_CRC_CRC8_MOW_ALGORITHM != 0
uint8_t turag_crc8_mow_calculate(const void* data, size_t length);
# endif
#endif


#if TURAG_CRC_CRC8_MOW_ALGORITHM != 0
// the check function is always inline
TURAG_INLINE bool turag_crc8_mow_check(const void* data, size_t length, uint8_t chksum) {
    return chksum == turag_crc8_mow_calculate(data, length);
}
#endif





/*
 * CRC16-implementation
 */
#if TURAG_CRC_INLINED_CALCULATION
# if TURAG_CRC_CRC16_ALGORITHM == 1
/* Generated on Sat Nov  2 16:02:08 2013,
 * by pycrc v0.8.1, http://www.tty1.net/pycrc/
 * using the configuration:
 *    Width        = 16
 *    Poly         = 0x1021
 *    XorIn        = 0xffff
 *    ReflectIn    = True
 *    XorOut       = 0x0000
 *    ReflectOut   = True
 *    Algorithm    = table-driven
 */
extern const uint16_t turag_crc_crc16_table[256];

TURAG_INLINE uint16_t turag_crc16_calculate(const void* data, size_t length) {
    uint16_t crc = 0xffff;

    while (length--) {
        crc = turag_crc_crc16_table[(crc ^ *(const uint8_t*)data) & 0xff] ^ (crc >> 8);
        data = (const uint8_t*)data + 1;
	}
    return crc;
}
# endif

# if TURAG_CRC_CRC16_ALGORITHM == 2
TURAG_INLINE uint16_t turag_crc16_calculate(const void* data, unsigned size) {
	// preload checksum
	uint16_t chksum = 0xffff;
	uint8_t data_byte;
	
	uint8_t i;
	for (i=0; i < size; ++i) {
        data_byte = ((const uint8_t*)data)[i] ^ turag_lowbyte(chksum);
		data_byte ^= data_byte << 4;
		
		chksum = (((uint16_t)data_byte << 8) | turag_highbyte(chksum)) ^
					(uint8_t)(data_byte >> 4) ^
					((uint16_t)data_byte << 3);
	}
	return chksum;
}
# endif
#else
uint16_t turag_crc16_calculate(const void* data, size_t length);
#endif

#if TURAG_CRC_CRC16_ALGORITHM != 0
TURAG_INLINE bool turag_crc16_check(const void* data, size_t length, uint16_t chksum) {
    return chksum == turag_crc16_calculate(data, length);
}
#endif


#ifdef __cplusplus
}           /* closing brace for extern "C" */
#endif

#endif // __DOXYGEN__

#endif      /* __TINA_CRC_CRC_CHECKSUMS_H__ */
