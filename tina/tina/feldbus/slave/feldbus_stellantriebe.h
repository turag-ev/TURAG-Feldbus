/**
 *  @brief		Implements slave side TURAG feldbus für Stellantriebe support
 *  @file		feldbus_stellantriebe.h
 *  @date		17.11.2013
 *  @author		Martin Oemus <martin@oemus.net>
 *
 * This module implements support for the TURAG Feldbus für Stellantriebe device protocol.
 *
 * If you want to use it, you have to call turag_feldbus_stellantriebe_init() once.
 * Whenever there is a get request, turag_feldbus_stellantriebe_get_value() will be called.
 * You have to supply this function yourself.
 *
 * When there is a set request to be handled, the command data is copied into feldbus_stellantriebe_commmand_buffer.data
 * and feldbus_stellantriebe_commmand_buffer.new data is set to one. The devices main loop should periodically check
 * whether new_data changed. While extracting the required data from feldbus_stellantriebe_commmand_buffer.data you should
 * disable all interrupts to avoid any pitfalls.
 *
 */
#ifndef TINA_FELDBUS_SLAVE_FELDBUS_STELLANTRIEBE_H_
#define TINA_FELDBUS_SLAVE_FELDBUS_STELLANTRIEBE_H_

#include <tina/tina.h>


#define TURAG_FELDBUS_DEVICE_PROTOCOL TURAG_FELDBUS_DEVICE_PROTOCOL_STELLANTRIEBE
#define TURAG_FELDBUS_DEVICE_PROTOCOL_VERSION TURAG_FELDBUS_STELLANTRIEBE_DEVICE_PROTOCOL_VERSION_1_0


typedef struct {
	/// holds the set request that was received
	volatile uint8_t data[3];
	/// this is 1 if the data array was updated
	volatile uint8_t new_data;

} feldbus_stellantriebe_command_t;


extern feldbus_stellantriebe_command_t feldbus_stellantriebe_commmand_buffer;

/**
 * Sets up this module.
 *
 * This function calls turag_feldbus_slave_init(),
 * so you should not do this again.
 */
void turag_feldbus_stellantriebe_init();

/** Reads value from the device.
 *
 * This function needs to be implemented by the specific device code.
 * It is called by this module to handle get requests from the bus master.
 *
 * @param key		specifies which data to get
 * @param output 	pointer holding the requested data
 * @return 			1 on success, 0 if the requested key could not be returned.
 */
extern uint8_t turag_feldbus_stellantriebe_get_value(uint8_t key, uint16_t* output);


#endif /* TINA_FELDBUS_SLAVE_FELDBUS_STELLANTRIEBE_H_ */
