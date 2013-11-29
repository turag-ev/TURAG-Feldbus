/**
 *  @brief		TURAG feldbus basic protocol
 *  @file		turag_feldbus_bus_protokoll.h
 *  @date		22.03.2013
 *  @author		Martin Oemus <martin@oemus.net>
 *
 */

/** @defgroup Feldbus
 *
 * The TURAG Feldbus is a communication protocol on top of RS485.
 * It consists of a basic protocol and specific device protocols.
 * 
 * 
 * For more information see https://www.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus
 * 
 * @dir tina/feldbus/protocol
 * This directory contains mostly symbols that directly reflect the definitions of the basic protocol and device 
 * protocols.
 * 
 * @dir tina/feldbus/slave
 * This directory contains ready-tu-use code-snippets that should be used when developing
 * the firmware of a new TURAG Feldbus device.
 * 
 * @dir tina++/feldbus/host
 * This directory contains classes that should be used to interface TURAG Feldbus slave devices.
 *
 */ 



#ifndef TURAG_FELDBUS_BUS_PROTOKOLL_H_
#define TURAG_FELDBUS_BUS_PROTOKOLL_H_


#define TURAG_FELDBUS_MASTER_ADDR		(0x80)
#define TURAG_FELDBUS_BROADCAST_ADDR		(0x00)


#define TURAG_FELDBUS_CHECKSUM_XOR			0x00
#define TURAG_FELDBUS_CHECKSUM_CRC8_ICODE		0x01


#define TURAG_FELDBUS_BROADCAST_TO_ALL_DEVICES			0x00
#define TURAG_FELDBUS_DEVICE_PROTOCOL_STELLANTRIEBE		0x01
#define TURAG_FELDBUS_DEVICE_PROTOCOL_LOKALISIERUNGSSENSOREN	0x02


#endif
