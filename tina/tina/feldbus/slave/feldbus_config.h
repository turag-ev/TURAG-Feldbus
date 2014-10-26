/**
 *  @file		feldbus_config.h
 *  @date		09-2014
 *  @author		Martin Oemus <martin@oemus.net>
 *  @ingroup feldbus-slave-base
 * 
 * @brief Beispiel für den Config-Header eines Feldbus Slave-Gerätes. Diese Datei kann in das Quellverzeichnis des Gerätes kopiert und angepasst werden.
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
 * Infos gibt es im Wiki: https://www.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus/
 * 
 * Für manche Definitionen sind mögliche Werte aufgelistet. Diese Auflistung
 * ist nicht notwendigerweise vollständig, insbesondere wenn in der Zukunft
 * neue Anwendungsprotokolle dazukommen.
 * 
 * 
 */
#error THIS FILE IS INTENDED FOR DOCUMUNTATIONAL PURPOSES ONLY.

#ifndef FELDBUS_CONFIG_H_
#define FELDBUS_CONFIG_H_


#include <turag_feldbus_bus_protokoll.h>


/**
 * Legt die Adresse des Gerätes fest.
 * 
 * Diese Angabe wird üblicherweise nicht in diesem Header,
 * sondern im Makefile gemacht.
 */
#define MY_ADDR		1


/**
 * Legt die bezeichnung des Gerätes fest.
 * 
 * Diese Angabe wird üblicherweise nicht in diesem Header,
 * sondern im Makefile gemacht.
 * 
 * Wird dieses Symbol nicht definiert, so wird der Defaultwert
 * "unnamed device" benutzt.
 */
#define TURAG_FELDBUS_DEVICE_NAME "Feldbus Device"


/**
 * Enthält Versionsinformationen über die auf dem Geräte
 * hinterlegte Firmware.
 * 
 * Diese Angabe wird üblicherweise nicht in diesem Header,
 * sondern im Makefile gemacht.
 * 
 * Wird dieses Symbol nicht definiert, so wird als Defaultwert
 * die aktuelle Zeit und das Datum vom Compiler benutzt.
 */
#define TURAG_FELDBUS_DEVICE_VERSIONINFO "25.09.2014 15:12"


/**
 * Defines the device protocol type of the slave.
 * 
 * Mögliche Werte:
 * - \ref TURAG_FELDBUS_DEVICE_PROTOCOL_STELLANTRIEBE
 * - \ref TURAG_FELDBUS_DEVICE_PROTOCOL_LOKALISIERUNGSSENSOREN
 * - \ref TURAG_FELDBUS_DEVICE_PROTOCOL_ASEB	
 * - \ref TURAG_FELDBUS_DEVICE_PROTOCOL_BOOTLOADER
 */
#define TURAG_FELDBUS_DEVICE_PROTOCOL 		TURAG_FELDBUS_DEVICE_PROTOCOL_STELLANTRIEBE


/**
 * Defines the device type of the slave.
 * 
 * Possible values depend on the specified device protocol 
 * and can be found in the corresponding headers.
 */
#define TURAG_FELDBUS_DEVICE_TYPE_ID 		TURAG_FELDBUS_STELLANTRIEBE_DEVICE_TYPE_DC


/**
 * Defines the length of the address used in 
 * each package.
 * 
 * Possible values:
 * - 1
 * - 2
 */
#define TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH 1


/**
 * Defines the crc algorithm to use for the device. 
 * 
 * Possible values:
 * - \ref TURAG_FELDBUS_CHECKSUM_XOR
 * - \ref TURAG_FELDBUS_CHECKSUM_CRC8_ICODE
 */
#define TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE 	TURAG_FELDBUS_CHECKSUM_CRC8_ICODE


/**
 * Defines the size of the I/O-buffers. The base implementation
 * allocates 3 buffers of the specified size.
 */
#define TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE		80		


/**
 * If set one, debug functions like print_text() become available.
 * If set to zero the function calls are removed and no output is generated.
 */
#define TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED		0


/**
 * If set to one, the device listens for broadcasts messages.
 * If set to zero, these packages are droppped which saves some
 * processing time and reduces codesize.
 */
#define TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE		0


/**
 * Legt die Frequenz[Hz] fest, mit der turag_feldbus_slave_increase_uptime_counter()
 * aufgerufen wird.
 * 
 * Gültige Werte: 0-65535
 * 
 * Bei einem Wert von 0 wird das Feature des Uptime-Counters
 * deaktiviert. Ein Wert zwischen 20 und 100 Hz ist empfehlenswert.
 */
#define TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY			50


/**
 * Legt fest, ob die Funktion turag_feldbus_slave_toggle_led()
 * mit einem charakteristischen Frequenzmuster aufgerufen wird.
 * 
 * Gültige Werte: 0/1
 * 
 * Diese Option spielt nur eine Rolle, wenn 
 * \ref TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY mit einem Wert
 * ungleich 0 konfiguriert ist.
 */
#define TURAG_FELDBUS_SLAVE_CONFIG_USE_LED_CALLBACK	1

#endif /* FELDBUS_CONFIG_H_ */
 
