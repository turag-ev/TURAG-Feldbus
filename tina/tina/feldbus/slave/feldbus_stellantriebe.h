/**
 *  @brief		Implements slave side TURAG feldbus für Stellantriebe support
 *  @file		feldbus_stellantriebe.h
 *  @date		17.11.2013
 *  @author		Martin Oemus <martin@oemus.net>
 *  @ingroup	feldbus-slave-stellantriebe
 */

/**
 *  @defgroup 	feldbus-slave-stellantriebe Stellantriebe
 *  @ingroup	feldbus-slave
 *
 * This module implements support for the %TURAG Feldbus für Stellantriebe device protocol. 
 * This device protocol was designed to meet the communication requirements of 
 * usual mechanical actors. 
 * 
 * As a main feature it is possible to define a table
 * of values within the device's firmware which is called command set. 
 * This table is used by the protocol implementation to enable 
 * read and or write access to these values. For each entry of the command set
 * you can adjust the length, write access and a factor which needs to be applied to 
 * the returned raw value to get a value of the correct physical dimension.
 * This is useful to avoid expensive floating point operations in potentially 
 * slow devices. The different possible configuration options are defined in the protocol
 * specification header. The actual location in memory of an entry of the command
 * set is given with a void pointer to the corresponding address.
 * Generally the entries of the command set are supposed to have a meaning that
 * justifies them to be handled as floating point values (values with a physical 
 * representation of some kind). However by setting
 * the factor to 0.0f you can indicate that the value should rather be seen
 * as a control value.
 * 
 * Additionally it is possible to supply a human-understandable description
 * for each value which needs to be supplied in a separate array of strings.
 * 
 * Ein weiteres wichtiges Feature ist die Ausgabe zusammenhängender 
 * Gerätewerte. Dafür wird im Gerät ein Puffer vorgesehen. Der Client 
 * kann eine Liste gewünschter abzufragender Werte übermitteln. Danach
 * kann beliebig oft mit einem simplen Befehl das Versenden der 
 * kompletten Daten initiiert werden. Auf diese Art und Weise können 
 * größere Datenmengen mit geringst möglichem Overhead vom Gerät abgefragt
 * werden und zum Beispiel zur grafischen Darstellung benutzt werden.
 * 
 * @section feldbus-slave-stellantriebe-config Konfiguration
 * Die folgenden Makros werden von diesem Protokoll in feldbus_config.h 
 * erwartet:
 * 
 * **TURAG_FELDBUS_STELLANTRIEBE_COMMAND_NAMES_USING_AVR_PROGMEM**:\n
 * Weist die Firmware an, dass die Zeichenketten, die die Gerätewerte
 * beschreiben, mit dem PROGMEM-Macro gespeichert wurden.
 * 
 * **TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_BUFFER_SIZE**:\n
 * Gibt an, wieviel Speicher für die zusammenhängende Datenausgabe verwendet
 * werden soll. Diese Zahl beeinflusst, wieviele Gerätewerte maximal gemeinsam
 * ausgegeben werden können.
 *
 *
 */
#ifndef TINA_FELDBUS_SLAVE_FELDBUS_STELLANTRIEBE_H_
#define TINA_FELDBUS_SLAVE_FELDBUS_STELLANTRIEBE_H_

#include <tina/tina.h>
#include <tina/feldbus/slave/feldbus_config_check.h>


#ifdef __cplusplus
extern "C" {
#endif




#if (TURAG_FELDBUS_DEVICE_PROTOCOL==TURAG_FELDBUS_DEVICE_PROTOCOL_STELLANTRIEBE) || defined(__DOXYGEN__)

// do not change this structure!!! Otherwise the device will send 
// corrupted command info packages.
typedef struct _packed {
	/// pointer to the actual value
    void* value;  
	/// is the value allowed to be altered by the host?
    uint8_t write_access;
	/// length of the value
    uint8_t length;
	/// factor that should be applied to the returned values by the host. can be set
	/// to 0.0f to indicate that the value is a control value (not floating point)
    float factor;
} feldbus_stellantriebe_command_t;


typedef union {
	int8_t char_buffer;
	int16_t short_buffer;
	int32_t long_buffer;
	uint8_t raw_buffer[4];
} feldbus_stellantriebe_value_buffer_t;

/// Buffers the old value of a changed entry of the command set.
extern feldbus_stellantriebe_value_buffer_t feldbus_stellantriebe_old_value;

/**
 * Sets up this module.
 *
 * This function calls turag_feldbus_slave_init(),
 * so you should not do this again.
 * 
 * @param command_set pointer to array containing the command set definition of the device
 * @param command_names pointer to array of strings describing the command set
 * @param command_set_length length of the command set
 */
void turag_feldbus_stellantriebe_init(
    feldbus_stellantriebe_command_t* command_set, 
    const char** command_names, 
    uint8_t command_set_length);

/** 
 * This function is called after a value was changed.
 * 
 * You can use this function to start certain actions that have to follow
 * the change of a value or you can check whether the new value is 
 * a valid one and if not, change it back using the variable feldbus_stellantriebe_old_value
 * that always holds the original value of the entry that was changed last.
 * 
 * @param key of the changed value
 * 
 */
extern void turag_feldbus_stellantriebe_value_changed(uint8_t key);


/**
 * This function is called upon the receiption of packages that are not handled by the 
 * Stellantriebe protocol.
 * 
 * If you don't nedd this functionality you can simply leave the function body empty.
 */
extern FeldbusSize_t turag_feldbus_stellantriebe_process_package(const uint8_t* message, FeldbusSize_t message_length, uint8_t* response);


FeldbusSize_t turag_feldbus_slave_process_package(const uint8_t* message, FeldbusSize_t message_length, uint8_t* response);


#endif



#ifdef __cplusplus
}
#endif

#endif /* TINA_FELDBUS_SLAVE_FELDBUS_STELLANTRIEBE_H_ */
