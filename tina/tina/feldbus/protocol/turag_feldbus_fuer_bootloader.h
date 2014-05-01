/**
 *  @brief		TURAG-Feldbus für Bootloader protocol definitions.
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
 * @name device types
 * @{
 */
#define TURAG_FELDBUS_BOOTLOADER_GENERIC				0x01
#define TURAG_FELDBUS_BOOTLOADER_ATMEGA                 0x02
#define TURAG_FELDBUS_BOOTLOADER_XMEGA  				0x03

///@}


/**
 * @name command keys
 * @{
 */
#define TURAG_FELDBUS_BOOTLOADER_COMMAND_START_PROGRAMM			0xFF
#define TURAG_FELDBUS_BOOTLOADER_COMMAND_PAGE_WRITE			0xAA
#define TURAG_FELDBUS_BOOTLOADER_COMMAND_ENTER_BOOTLOADER		0x11
#define TURAG_FELDBUS_BOOTLOADER_COMMAND_GET_MCUID			0x12
#define TURAG_FELDBUS_BOOTLOADER_COMMAND_TEST				0x01
///@}


/**
 * @name response codes
 * @{
 */
#define TURAG_FELDBUS_BOOTLOADER_RESPONSE_PAGE_CORRECT_SIZE		0xBA
#define TURAG_FELDBUS_BOOTLOADER_RESPONSE_PAGE_FAIL_END			0xBB
#define TURAG_FELDBUS_BOOTLOADER_RESPONSE_PAGE_FAIL_SIZE		0xBF
///@}


/**
 * @name mcu-ids (signature)
 * @{
 */
#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA8				0x07
#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA88			0x0A
#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA168			0x06
#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA32			0x02
#define TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA644			0x09

///@}

#endif
