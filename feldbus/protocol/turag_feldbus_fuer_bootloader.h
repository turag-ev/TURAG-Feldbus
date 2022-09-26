/**
 *  @brief		TURAG-Feldbus für BMaX-Bootloader protocol definitions.
 *  @file		turag_feldbus_fuer_bootloader.h
 *  @date		21.02.2014
 *  @author		Florian Völker <flo-voelker@gmx.de>
 *  @ingroup	feldbus-protocol
 *
 */

#ifndef TURAG_FELDBUS_FUER_BOOTLOADER_H_
#define TURAG_FELDBUS_FUER_BOOTLOADER_H_


#include "turag_feldbus_bus_protokoll.h"

/**
 * @name Gerätetypen/Architekturen
 * @{
 */
#define TURAG_FELDBUS_BOOTLOADER_GENERIC				0x01
#define TURAG_FELDBUS_BOOTLOADER_ATMEGA					0x02
#define TURAG_FELDBUS_BOOTLOADER_XMEGA  				0x03
#define TURAG_FELDBUS_BOOTLOADER_STM32  				0x04
#define TURAG_FELDBUS_BOOTLOADER_STM32V2				0x05

///@}


/*
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * 
 * NEVER EVER CHANGE ANY 
 * OF THOSE DEFINITIONS!!!!!!
 * (Adding new ones is ok.)
 * 
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */


/**
 * @name Allgemeine Commands
 * @{
 */
#define TURAG_FELDBUS_BOOTLOADER_COMMAND_GET_MCUID				0x01
#define TURAG_FELDBUS_BOOTLOADER_COMMAND_UNLOCK_BOOTLOADER		0x02

// only supported if the device returns 0xFFFF as mcu id
#define TURAG_FELDBUS_BOOTLOADER_COMMAND_RECEIVE_MCU_STRING		0x03
#define TURAG_FELDBUS_BOOTLOADER_COMMAND_RECEIVE_MCU_STRING_LENGTH		0x04

#define TURAG_FELDBUS_BOOTLOADER_COMMAND_ENTER_BOOTLOADER		0xA1
#define TURAG_FELDBUS_BOOTLOADER_COMMAND_START_PROGRAMM			0xAF

#define TURAG_FELDBUS_BOOTLOADER_UNLOCK_CODE					0x4266
#define TURAG_FELDBUS_BOOTLOADER_RESPONSE_UNLOCKED				0x00
#define TURAG_FELDBUS_BOOTLOADER_RESPONSE_UNLOCK_REJECTED		0x01

///@}


/**
 * @name Commands for AVR8 (Atmega and Xmega)
 * @{
 */
#define TURAG_FELDBUS_BOOTLOADER_AVR_GET_PAGE_SIZE			0x11
#define TURAG_FELDBUS_BOOTLOADER_AVR_GET_FLASH_SIZE			0x13
#define TURAG_FELDBUS_BOOTLOADER_AVR_PAGE_WRITE				0xAA
#define TURAG_FELDBUS_BOOTLOADER_AVR_DATA_READ				0xAB


#define TURAG_FELDBUS_BOOTLOADER_AVR_RESPONSE_SUCCESS            0x00
#define TURAG_FELDBUS_BOOTLOADER_AVR_RESPONSE_FAIL_SIZE          0xFA
#define TURAG_FELDBUS_BOOTLOADER_AVR_RESPONSE_FAIL_ADDRESS       0xFB
#define TURAG_FELDBUS_BOOTLOADER_AVR_RESPONSE_FAIL_NOT_SUPPORTED 0xFC
#define TURAG_FELDBUS_BOOTLOADER_AVR_RESPONSE_FAIL_CONTENT       0xFD
///@}

/**
 * @name AVR Atmega Commands
 * @{
 */
#define TURAG_FELDBUS_BOOTLOADER_ATMEGA_GET_FUSES				0x12
///@}

/**
 * @name AVR Xmega Commands
 * @{
 */
#define TURAG_FELDBUS_BOOTLOADER_XMEGA_GET_FUSES				0x12
#define TURAG_FELDBUS_BOOTLOADER_XMEGA_GET_REVISION				0x14
///@}


/**
 * @name STM32v2 Commands
 * @{
 */
#define TURAG_FELDBUS_BOOTLOADER_STM32V2_GET_PAGE_SIZE				0x11
#define TURAG_FELDBUS_BOOTLOADER_STM32V2_GET_FLASH_SIZE				0x13

#define TURAG_FELDBUS_BOOTLOADER_STM32V2_GET_APP_ADDRESS_MODE	 		0x15

// In this mode the stack address and the address of the reset handler are stored
// at a specific address in the flash in addition to the program itself by using a
// special command. When the bootloader
// needs to start the program it reads these addresses from this flash position.
// This approach is easy to implement but requires an additional erasable flash segment which
// may be wasteful on some F4 devices whose flash sectors are very large at the end of the flash.
#define TURAG_FELDBUS_BOOTLOADER_STM32V2_APP_ADDRESS_MODE_DEDICATED_PAGE	0x00

// This method makes use of reserved positions within the vector table itself. This is the
// preferred option for any device which has suitable unused space within its vector table.
#define TURAG_FELDBUS_BOOTLOADER_STM32V2_APP_ADDRESS_MODE_HIDDEN_IN_VECTOR_TABLE	0x01


//#define TURAG_FELDBUS_BOOTLOADER_STM32V2_GET_BOOTLOADER_RESET_VECTOR			0x16
//#define TURAG_FELDBUS_BOOTLOADER_STM32V2_STORE_APP_RESET_VECTORS 	0x17  // only defined in APP_ADDRESS_MODE_DEDICATED_PAGE
//#define TURAG_FELDBUS_BOOTLOADER_STM32V2_GET_APP_RESET_VECTOR_STORAGE_ADDRESS	0x18	// only defined in APP_ADDRESS_MODE_HIDDEN_IN_VECTOR_TABLE

// Transmits the content of the reset vector of the user application to the bootloader.
// This command must be called before transmitting any of the user application code if the address mode
// is hidden vector.
#define TURAG_FELDBUS_BOOTLOADER_STM32V2_TRANSMIT_APP_RESET_VECTOR	0x16

// This function schould be called after the whole user application has been
// transmitted. Depending on the used address mode this function either
// - writes the user application reset vector to the dedicated flash page
// - checks whether the supplied reset vector can be found at the expected
//   position in the vector table.
//
// Returns an error code.
#define TURAG_FELDBUS_BOOTLOADER_STM32V2_COMMIT_APP_RESET_VECTOR  0x17

// Returns the location of the user app reset vector. This can be used
// by the host when verifying the flash contents by reading it and comparing it
// with the written data. If the reset vector is stored in the vector table, the first
// flash page will be modified at this position and the read data will be different than the written data.
#define TURAG_FELDBUS_BOOTLOADER_STM32V2_GET_APP_RESET_VECTOR_STORAGE_ADDRESS	0x18

// and APP_ADDRESS_MODE_AFTER_USERAPP
#define TURAG_FELDBUS_BOOTLOADER_STM32V2_PAGE_WRITE				0xAA
#define TURAG_FELDBUS_BOOTLOADER_STM32V2_DATA_READ				0xAB




#define TURAG_FELDBUS_BOOTLOADER_STM32V2_RESPONSE_SUCCESS            0x00
#define TURAG_FELDBUS_BOOTLOADER_STM32V2_RESPONSE_FAIL_SIZE          0xFA
#define TURAG_FELDBUS_BOOTLOADER_STM32V2_RESPONSE_FAIL_ADDRESS       0xFB
#define TURAG_FELDBUS_BOOTLOADER_STM32V2_RESPONSE_FAIL_NOT_SUPPORTED 0xFC
#define TURAG_FELDBUS_BOOTLOADER_STM32V2_RESPONSE_FAIL_FLASH	       0xFD




///@}



/**
 * @name MCU-IDs 
 * @{
 */
#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_INVALID				    0x0000
#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_STRING					0xFFFF

#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA8				    0x0793
#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA16			    0x0394
#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA32			    0x0295
#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA128			    0x0297
#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA88			    0x0A93
#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA168			    0x0694
#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA644			    0x0996

#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_XMEGA16D4			    0x9442
#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_XMEGA32D4			    0x9542
#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_XMEGA64D4			    0x9647
#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_XMEGA128D4			    0x9747

#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_XMEGA16E5			    0x9445
#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_XMEGA32E5			    0x944C

// note: the values above are legacy. From here on values are manually
// chosen (mainly because for STM32 there seems to be no unique
// model nuber).
#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_STM32F051x8 0x1001
#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_STM32F030x4 0x1002
#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_STM32F031x6 0x1003



///@}

#endif
