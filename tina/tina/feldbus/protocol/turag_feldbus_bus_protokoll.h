/**
 *  @brief		TURAG feldbus basic protocol definitions.
 *  @file		turag_feldbus_bus_protokoll.h
 *  @date		22.03.2013
 *  @author		Martin Oemus <martin@oemus.net>
 *  @ingroup	feldbus-protocol
 *
 */



#ifndef TURAG_FELDBUS_BUS_PROTOKOLL_H_
#define TURAG_FELDBUS_BUS_PROTOKOLL_H_


#define TURAG_FELDBUS_MASTER_ADDR		(0x80)
#define TURAG_FELDBUS_BROADCAST_ADDR		(0x00)


#define TURAG_FELDBUS_CHECKSUM_XOR			0x00
#define TURAG_FELDBUS_CHECKSUM_CRC8_ICODE		0x01


#define TURAG_FELDBUS_BROADCAST_TO_ALL_DEVICES					0x00
#define TURAG_FELDBUS_DEVICE_PROTOCOL_STELLANTRIEBE				0x01
#define TURAG_FELDBUS_DEVICE_PROTOCOL_LOKALISIERUNGSSENSOREN	0x02
#define TURAG_FELDBUS_DEVICE_PROTOCOL_ASEB						0x03
#define TURAG_FELDBUS_DEVICE_PROTOCOL_BOOTLOADER				0x04


#define TURAG_FELDBUS_IGNORE_PACKAGE		0xff


#endif
