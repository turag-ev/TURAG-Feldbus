/**
 *  @brief		TURAG-Feldbus basic protocol definitions.
 *  @file		turag_feldbus_bus_protokoll.h
 *  @date		22.03.2013
 *  @author		Martin Oemus <martin@oemus.net>
 *  @ingroup	feldbus-protocol
 *
 */



#ifndef TURAG_FELDBUS_BUS_PROTOKOLL_H_
#define TURAG_FELDBUS_BUS_PROTOKOLL_H_

/**
 * @name Special bus addresses
 * @{
 */
/// @brief Master address on the bus.
#define TURAG_FELDBUS_MASTER_ADDR			(0x80)
/// 2-byte master address on the bus.
#define TURAG_FELDBUS_MASTER_ADDR_2			(0x8000)

/// @brief Broadcast address on the bus.
#define TURAG_FELDBUS_BROADCAST_ADDR		(0x00)
/// @brief 2-byte broadcast address on the bus.
#define TURAG_FELDBUS_BROADCAST_ADDR_2		(0x0000)

/// @}


/**
 * @name Checksum types
 * @{
 */

/// @brief Xor-based checksum.
#define TURAG_FELDBUS_CHECKSUM_XOR				0x00
/// @brief CRC8-Checksum.
#define TURAG_FELDBUS_CHECKSUM_CRC8				0x01

#define TURAG_FELDBUS_CHECKSUM_CRC8_16_MIXED   	0x02

///@}

/**
 * @name Device Protocol Types and broadcast addresses
 * @{
 */
 
/// @brief Broadcast to all device types
#define TURAG_FELDBUS_BROADCAST_TO_ALL_DEVICES					0x00

/// @brief "Stellantrieb" device type
#define TURAG_FELDBUS_DEVICE_PROTOCOL_STELLANTRIEBE				0x01

/// @brief "Lokalisierungssensoren" device type
#define TURAG_FELDBUS_DEVICE_PROTOCOL_LOKALISIERUNGSSENSOREN	0x02

/// @brief "ASEB" device type
#define TURAG_FELDBUS_DEVICE_PROTOCOL_ASEB						0x03

/// @brief "BMaX" bootloader device type
#define TURAG_FELDBUS_DEVICE_PROTOCOL_BOOTLOADER				0x04

/// @brief "ESCON" motor drive device type
#define TURAG_FELDBUS_DEVICE_PROTOCOL_ESCON 					0x05

///@}


/**
 * @name Reservierte Pakete
 * @{
 */
 
/// @brief Ausgeben des Gerätenamens
#define TURAG_FELDBUS_DEVICE_COMMAND_DEVICE_NAME					0x00

/// @brief Ausgeben des Uptime-Counters
#define TURAG_FELDBUS_DEVICE_COMMAND_UPTIME_COUNTER					0x01

/// @brief Ausgeben des Versionsinfo-Strings
#define TURAG_FELDBUS_DEVICE_COMMAND_VERSIONINFO					0x02

/// @brief Ausgabe der Anzahl korrekt empfangener Pakete
#define TURAG_FELDBUS_DEVICE_COMMAND_PACKAGE_COUNT_CORRECT			0x03

/// @brief Ausgabe der Anzahl von Pufferüberläufen
#define TURAG_FELDBUS_DEVICE_COMMAND_PACKAGE_COUNT_BUFFEROVERFLOW	0x04

/// @brief Ausgabe der Anzahl verlorener Pakete
#define TURAG_FELDBUS_DEVICE_COMMAND_PACKAGE_COUNT_LOST				0x05

/// @brief Ausgabe der Anzahl von Checksummenfehlern
#define TURAG_FELDBUS_DEVICE_COMMAND_PACKAGE_COUNT_CHKSUM_MISMATCH	0x06

/// @brief Ausgabe aller vier Paketanzahlen in folgender Reihenfolge: korrekte Pakete, Pufferüberläufe, verlorene Pakete, Checksummenfehler
#define TURAG_FELDBUS_DEVICE_COMMAND_PACKAGE_COUNT_ALL				0x07

/// @brief Alle 4 Paketanzahlen werden auf 0 zurückgesetzt
#define TURAG_FELDBUS_DEVICE_COMMAND_RESET_PACKAGE_COUNT			0x08

/// @brief Returns UUID of the device
#define TURAG_FELDBUS_DEVICE_COMMAND_GET_UUID						0x09

/// @brief Return the extended device info packet. Not available when the device returns a legacy style device info packet.
#define TURAG_FELDBUS_DEVICE_COMMAND_GET_EXTENDED_INFO				0x0A


///@}
/**
 * @name Reserved broadcasts with broadcast ID 0x00
 * @{
 */

/// @brief Ping any device to return UUID
#define TURAG_FELDBUS_DEVICE_BROADCAST_UUID						0x00

/// @brief get/set bus address of device with given UUID
#define TURAG_FELDBUS_DEVICE_BROADCAST_UUID_ADDRESS				0x00

/// @brief reset bus address of specified device
#define TURAG_FELDBUS_DEVICE_BROADCAST_UUID_RESET_ADDRESS		0x01

/// @brief enable bus neighbors
#define TURAG_FELDBUS_DEVICE_BROADCAST_ENABLE_NEIGHBOURS		0x01

/// @brief disable bus neighbors
#define TURAG_FELDBUS_DEVICE_BROADCAST_DISABLE_NEIGHBOURS		0x02

/// @brief disable bus neighbors
#define TURAG_FELDBUS_DEVICE_BROADCAST_RESET_ADDRESSES			0x03

/// @brief disable bus neighbors
#define TURAG_FELDBUS_DEVICE_BROADCAST_REQUEST_BUS_ASSERTION	0x04

/// @brief disable bus neighbors if no valid bus address
#define TURAG_FELDBUS_DEVICE_BROADCAST_REQUEST_BUS_ASSERTION_IF_NO_ADRRESS	0x05

/// @brief disable bus neighbors if no valid bus address
#define TURAG_FELDBUS_DEVICE_BROADCAST_GO_TO_SLEEP				0x06



///@}


#endif
