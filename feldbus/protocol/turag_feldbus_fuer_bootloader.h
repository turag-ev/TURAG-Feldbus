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
 * @name MCU-IDs 
 * @{
 */
#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_INVALID				    0x0000

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

//legacy ChibiOS based bootloaders
#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_CHIBIOS_STM32F411xE 0x1000
#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_CHIBIOS_STM32F051x8 0x9748 // auto-generated
#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_CHIBIOS_STM32F405xG 0x9749 // auto-generated


///@}

#endif
