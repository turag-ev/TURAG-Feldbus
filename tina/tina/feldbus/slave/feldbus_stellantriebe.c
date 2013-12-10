/**
 *  @brief		Implements slave side TURAG feldbus für Stellantriebe support
 *  @file		feldbus_stellantriebe.c
 *  @date		17.11.2013
 *  @author		Martin Oemus <martin@oemus.net>
 * 
 */

#include <tina/tina.h>
#include "feldbus_stellantriebe.h"
#include "feldbus.h"
#include <feldbus_config.h>


#if (TURAG_FELDBUS_DEVICE_PROTOCOL==TURAG_FELDBUS_DEVICE_PROTOCOL_STELLANTRIEBE) || defined(DOXYGEN)

feldbus_stellantriebe_command_t feldbus_stellantriebe_commmand_buffer;

void turag_feldbus_stellantriebe_init() {
	feldbus_stellantriebe_commmand_buffer.new_data = 0;

	turag_feldbus_slave_init();
}


uint8_t turag_feldbus_slave_process_package(uint8_t* message, uint8_t message_length, uint8_t* response) {
// the feldbus base implementation guarantees message_length to be >= 1 so we don't need to check that
	if (message[0] & RS485_MOTOR_SET_COMMAND_MASK) {
		// we received a set command, so copy command data
		feldbus_stellantriebe_commmand_buffer.data[0] = message[0];
		feldbus_stellantriebe_commmand_buffer.data[1] = message[1];
		feldbus_stellantriebe_commmand_buffer.data[2] = message[2];
		feldbus_stellantriebe_commmand_buffer.new_data = 1;

		// dont return data
		return 0;
	} else {
		// we received a get command, so access its value
		uint16_t data_to_return = 0;
		uint8_t result = turag_feldbus_stellantriebe_get_value(message[0], &data_to_return);

		// in case we don't support the requested command, we return an appropriate answer
		uint8_t return_key;
		if (result == 1) {
			return_key = message[0];
		} else {
			return_key = RS485_MOTOR_COM_NOT_IMPLEMENTED;
		}

		response[0] = return_key;
		response[1] = (unsigned char)data_to_return;
		response[2] = (unsigned char)(data_to_return>>8);
		return 3;
	}
}

#endif
