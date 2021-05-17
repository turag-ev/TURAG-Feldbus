/**
 *  @file		feldbus_config.h
 *  @date		09-2014
 *  @author		Martin Oemus <martin@oemus.net>
 *  @ingroup    feldbus-slave-base
 * 
 * @brief Beispiel für den Config-Header eines Feldbus Slave-Gerätes. 
 * Diese Datei kann in das Quellverzeichnis des Gerätes kopiert und angepasst werden.
 * 
 * Die Basisimplementierung und die Protokollimplementierungen
 * sind konfigurierbar. Die Konfiguration muss in einem Header
 * dieses Namens bereitgestellt werden.
 * 
 * Alle hier aufgeführten Definitionen sind für jedes Slave-Gerät notwendig.
 * Fehlt eine (Ausnahmen sind angegeben), so wird der Code nicht compilieren.
 * Darüber hinaus kann es sein, dass das benutzte Anwendungsprotokoll weitere
 * Definitionen verlangt.
 * 
 * Die Definitionen spiegeln die Fähigkeiten des Basis-Protokolls wieder. Nähere
 * Infos gibt es im Wiki: https://intern.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus/
 * 
 * Für manche Definitionen sind mögliche Werte aufgelistet. Diese Auflistung
 * ist nicht notwendigerweise vollständig, insbesondere wenn in der Zukunft
 * neue Anwendungsprotokolle dazukommen.
 * 
 * 
 */
#error THIS FILE IS INTENDED FOR DOCUMENTATIONAL PURPOSES ONLY.

#ifndef FELDBUS_CONFIG_H_
#define FELDBUS_CONFIG_H_


#include <feldbus/protocol/turag_feldbus_bus_protokoll.h>



/**
 * Defines the length of the address used in 
 * each package.
 * 
 * Possible values:
 * - 1
 * - 2
 */
#define TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH 1


/**
 * Defines the crc algorithm to use for the device. 
 * 
 * Mögliche Werte:
 * - \ref TURAG_FELDBUS_CHECKSUM_XOR
 * - \ref TURAG_FELDBUS_CHECKSUM_CRC8
 * - \ref TURAG_FELDBUS_CHECKSUM_CRC8_16_MIXED
 */
#define TURAG_FELDBUS_DEVICE_CONFIG_CRC_TYPE 	TURAG_FELDBUS_CHECKSUM_CRC8


/**
 * Defines the size of the I/O-buffers. The base implementation
 * allocates 2 buffers of the specified size.
 */
#define TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE		80


/**
 * If set one, debug functions like print_text() become available.
 * If set to zero the function calls are removed and no output is generated.
 */
#define TURAG_FELDBUS_DEVICE_CONFIG_DEBUG_ENABLED		0


/**
 * Legt die Frequenz[Hz] fest, mit der turag_feldbus_slave_increase_uptime_counter()
 * aufgerufen wird.
 *
 * Gültige Werte: 0-65535
 *
 * Bei einem Wert von 0 wird das Feature des Uptime-Counters
 * deaktiviert. Ein Wert zwischen 20 und 100 Hz ist empfehlenswert.
 */
#define TURAG_FELDBUS_DEVICE_CONFIG_UPTIME_FREQUENCY			50



#endif /* FELDBUS_CONFIG_H_ */
 
