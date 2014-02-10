#ifndef TURAG_FELDBUS_FUER_BOOTLOADER_H_
#define TURAG_FELDBUS_FUER_BOOTLOADER_H_

/* 
 * This header file contains the specification for the protocol
 * "TURAG-FeldBus für Bootloader". Refer to the DokuWiki for documentation.
 */

#include "turag_feldbus_bus_protokoll.h"

// device types
#define TURAG_FELDBUS_BOOTLOADER_GENERIC		0x01

// command
#define TURAG_FELDBUS_BOOTLOADER_COMMAND_START_PROGRAMM		0xBB
#define TURAG_FELDBUS_BOOTLOADER_COMMAND_TEXT				0x11

#endif
