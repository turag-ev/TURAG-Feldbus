
#include <tina/tina.h>
#include "feldbus_stellantriebe.h"
#include <string.h>

#if (TURAG_FELDBUS_DEVICE_PROTOCOL==TURAG_FELDBUS_DEVICE_PROTOCOL_STELLANTRIEBE) || defined(__DOXYGEN__)


#ifndef TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_BUFFER_SIZE
# error TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_BUFFER_SIZE must be defined
#endif
#ifndef TURAG_FELDBUS_STELLANTRIEBE_COMMAND_NAMES_USING_AVR_PROGMEM
# error TURAG_FELDBUS_STELLANTRIEBE_COMMAND_NAMES_USING_AVR_PROGMEM must be defined
#endif


#if TURAG_FELDBUS_STELLANTRIEBE_COMMAND_NAMES_USING_AVR_PROGMEM
# include <avr/pgmspace.h>
#endif		




static feldbus_stellantriebe_command_t* commmand_set = 0;
static const char** command_names = 0;
static uint8_t command_set_length = 0;

static feldbus_stellantriebe_command_t* structured_output_table[TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_BUFFER_SIZE];
static uint8_t structured_output_table_length = 0;

feldbus_stellantriebe_value_buffer_t feldbus_stellantriebe_old_value;


void turag_feldbus_stellantriebe_init(feldbus_stellantriebe_command_t* command_set_, const char** command_names_, uint8_t command_set_length_) {
    commmand_set = command_set_;
    command_names = command_names_;
    command_set_length = command_set_length_;

    turag_feldbus_slave_init();
}


uint8_t turag_feldbus_slave_process_package(uint8_t* message, uint8_t message_length, uint8_t* response) {
    // the feldbus base implementation guarantees message_length >= 1 and message[0] >= 1 
	// so we don't need to check that

    uint8_t index = message[0] - 1;
	uint8_t* pValue;
    
    if (index < command_set_length) {
        if (message_length == 1) {
            // read request
            feldbus_stellantriebe_command_t* command = commmand_set + index;

            switch (command->length) {
            case TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_CHAR:
                *response = *((uint8_t*)command->value);
                return 1;
                break;
            case TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT:
                pValue = (uint8_t*)command->value;
                response[0] = pValue[0];
                response[1] = pValue[1];
                return 2;
                break;
            case TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_LONG:
                pValue = (uint8_t*)command->value;
                response[0] = pValue[0];
                response[1] = pValue[1];
                response[2] = pValue[2];
                response[3] = pValue[3];
                return 4;
                break;
            case TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_NONE:
            default:
                return TURAG_FELDBUS_IGNORE_PACKAGE;
                break;
            }
        } else if (message_length != 4) {
            // write request
            feldbus_stellantriebe_command_t* command = commmand_set + index;

            if (command->write_access != TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS) {
                return TURAG_FELDBUS_IGNORE_PACKAGE;
            }

            // buffer received data to handle situations
            // where there is to little data 
            uint8_t buffer[4] = {0};
			
            switch (message_length) {
            case 2:
                buffer[0] = message[1];
                break;
            case 3:
                buffer[0] = message[1];
                buffer[1] = message[2];
                break;
            case 5:
                buffer[0] = message[1];
                buffer[1] = message[2];
                buffer[2] = message[3];
                buffer[3] = message[4];
                break;
            default:
                return TURAG_FELDBUS_IGNORE_PACKAGE;
                break;
            }

			uint8_t* pValue = (uint8_t*)command->value;

			switch (command->length) {
			case TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_CHAR:
				feldbus_stellantriebe_old_value.raw_buffer[0] = pValue[0];
				pValue[0] = buffer[0];
				turag_feldbus_stellantriebe_value_changed(message[0]);
				return 0;
				break;
			case TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT:
				feldbus_stellantriebe_old_value.raw_buffer[0] = pValue[0];
				pValue[0] = buffer[0];
				feldbus_stellantriebe_old_value.raw_buffer[1] = pValue[1];
				pValue[1] = buffer[1];
				turag_feldbus_stellantriebe_value_changed(message[0]);
				return 0;
				break;
			case TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_LONG:
				feldbus_stellantriebe_old_value.raw_buffer[0] = pValue[0];
				pValue[0] = buffer[0];
				feldbus_stellantriebe_old_value.raw_buffer[1] = pValue[1];
				pValue[1] = buffer[1];
				feldbus_stellantriebe_old_value.raw_buffer[2] = pValue[2];
				pValue[2] = buffer[2];
				feldbus_stellantriebe_old_value.raw_buffer[3] = pValue[3];
				pValue[3] = buffer[3];
				turag_feldbus_stellantriebe_value_changed(message[0]);
				return 0;
				break;
			default: 
				return TURAG_FELDBUS_IGNORE_PACKAGE;
				break;
			}

		} else {
            if (message[1] == TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_COMMANDSET_SIZE) {
                // return length of command set
                response[0] = command_set_length;
                return 1;

            } else if (message[1] == TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET) {
                // command info request
				_Static_assert(6 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
                feldbus_stellantriebe_command_t* command = commmand_set + index;
                memcpy(response, &command->write_access, 6);
                return 6;

            } else if (message[1] == TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_NAME_LENGTH) {
                // return length of command name
                if (!command_names) {
                    response[0] = 0;
                } else {
#if TURAG_FELDBUS_STELLANTRIEBE_COMMAND_NAMES_USING_AVR_PROGMEM
                    response[0] = strlen_PF((uint_farptr_t)((uint16_t)command_names[index]));
#else
                    response[0] = strlen(command_names[index]);
#endif
                }
                return 1;

            } else if (message[1] == TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_NAME) {
                // return command name
                if (!command_names) {
                    return 0;
                }

                uint8_t length = 0;

#if TURAG_FELDBUS_STELLANTRIEBE_COMMAND_NAMES_USING_AVR_PROGMEM
                length = strlen_PF((uint_farptr_t)((uint16_t)command_names[index]));
#else
                length = strlen(command_names[index]);
#endif				
				if (length + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH > TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE) {
					length = TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE - TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH;
				}
				
#if TURAG_FELDBUS_STELLANTRIEBE_COMMAND_NAMES_USING_AVR_PROGMEM
                memcpy_PF(response, (uint_farptr_t)((uint16_t)command_names[index]), length);
#else
                memcpy(response, command_names[index], length);
#endif
                return length;

            } else {
                return TURAG_FELDBUS_IGNORE_PACKAGE;
            }
        }
    } else if (message[0] == TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_GET) {
        if (message_length == 1) {
            // generate structured output
            int i;
            uint8_t* out = response;
            feldbus_stellantriebe_command_t* command;

            // we do not check whether command->value is a valid pointer
            // because we check for validity of the requested values
            // when the table is generated
			// There is also a check done whether the output will fit
			// into the bufer, so there is no check required either.
            for (i = 0; i < structured_output_table_length; ++i) {
				command = structured_output_table[i];
				
                switch (command->length) {
                case TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_CHAR:
                    *out = *((uint8_t*)command->value);
                    out += 1;
                    break;
                case TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT:
                    pValue = (uint8_t*)command->value;
                    out[0] = pValue[0];
                    out[1] = pValue[1];
                    out += 2;
                    break;
                case TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_LONG:
                    pValue = (uint8_t*)command->value;
                    out[0] = pValue[0];
                    out[1] = pValue[1];
                    out[2] = pValue[2];
                    out[3] = pValue[3];
                    out += 4;
                    break;
                }
            }
            return out - response;

        } else {
            if (message[1] == TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_SET_STRUCTURE) {
                // update structure table
                int8_t i, error = 0;
                uint8_t size_sum = 0, value_index;
                feldbus_stellantriebe_command_t* command;

                // cancel if the request is too long
                if (message_length - 2 > TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_BUFFER_SIZE) {
                    error = 1;
                    message_length = 0;
                }

                for (i = 2; i < message_length; ++i) {
                    value_index = message[i] - 1;

                    // cancel if the host demands a non-supported key
                    if (value_index >= command_set_length) {
                        error = 1;
                        break;
                    }

                    command = commmand_set + value_index;

                    // cancel if the host demands a non-supported key
                    if (command->length == TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_NONE) {
                        error = 1;
                        break;
                    }

                    structured_output_table[i-2] = command;
                    size_sum += command->length;

                    // cancel if whole package would not fit into buffer
                    if (size_sum >= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE) {
                        error = 1;
                        break;
                    }
                }

                if (error == 1) {
                    structured_output_table_length = 0;
                    response[0] = TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_TABLE_REJECTED;
                    return 1;
                } else {
                    structured_output_table_length = message_length - 2;
                    response[0] = TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_TABLE_OK;
                    return 1;
                }
            } else if (message[1] == TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_GET_BUFFER_SIZE) {
                // return table size
                response[0] = TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_BUFFER_SIZE;
                return 1;
            } else {
                return TURAG_FELDBUS_IGNORE_PACKAGE;
            }
        }
    } else {
        return turag_feldbus_stellantriebe_process_package(message, message_length, response);
    }
}


#endif
