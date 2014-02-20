#ifndef TURAG_FELDBUS_FUER_BOOTLOADER_H_
#define TURAG_FELDBUS_FUER_BOOTLOADER_H_

/* 
 * This header file contains the specification for the protocol
 * "TURAG-FeldBus für Bootloader". Refer to the DokuWiki for documentation.
 */

#include "turag_feldbus_bus_protokoll.h"

// device types
#define TURAG_FELDBUS_BOOTLOADER_GENERIC		0x01

// command PC => Bootloader
#define TURAG_FELDBUS_BOOTLOADER_COMMAND_START_PROGRAMM		0xFF
#define TURAG_FELDBUS_BOOTLOADER_COMMAND_PAGE_WRITE			0xAA


// command Bootloader => PC
#define TURAG_FELDBUS_BOOTLOADER_COMMAND_B_PAGE_FAIL_SIZE	0xFF
#define TURAG_FELDBUS_BOOTLOADER_COMMAND_B_PAGE_CORRECT_SIZE	0x11

#endif
