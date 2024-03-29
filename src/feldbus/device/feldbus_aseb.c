
#include <string.h>

#include <feldbus/protocol/simple_io_protocol.h>
#include "feldbus_aseb.h"




// Unfortunately we can not use the size information in the
// init function for static_asserts. So we have to require enough
// buffer to can handle the largest possible package: sync with
// all digital and analog inputs with 2-byte address.
// The output of the labels are capped to what we can handle.
#if TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE < 36
# error Buffer overflow. TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE < 36, must be bigger.
#endif


static feldbus_aseb_digital_io_t* digital_inputs;
static uint8_t digital_inputs_size;
static feldbus_aseb_digital_io_t* digital_outputs;
static uint8_t digital_outputs_size;
static feldbus_aseb_analog_t* analog_inputs;
static uint8_t analog_inputs_size;
static feldbus_aseb_pwm_t* pwm_outputs;
static uint8_t pwm_outputs_size;
static uint8_t analog_resolution;



void turag_feldbus_aseb_init(
    feldbus_aseb_digital_io_t* digital_inputs_, const uint8_t digital_inputs_size_,
    feldbus_aseb_digital_io_t* digital_outputs_, const uint8_t digital_outputs_size_,
    feldbus_aseb_analog_t* analog_inputs_, const uint8_t analog_inputs_size_,
	feldbus_aseb_pwm_t* pwm_outputs_, const uint8_t pwm_outputs_size_, const uint8_t analog_resolution_)
{
	
	digital_inputs = digital_inputs_;
	digital_inputs_size = digital_inputs_size_;
	digital_outputs = digital_outputs_;
	digital_outputs_size = digital_outputs_size_;
	analog_inputs = analog_inputs_;
	analog_inputs_size = analog_inputs_size_;
	pwm_outputs = pwm_outputs_;
	pwm_outputs_size = pwm_outputs_size_;
	analog_resolution = analog_resolution_;
}


FeldbusSize_t turag_feldbus_aseb_process_package(const uint8_t* message, FeldbusSize_t message_length, uint8_t* response) {
    // the feldbus base implementation guarantees message_length >= 1 and message[0] >= 1 
	// so we don't need to check that
	
	if (message[0] == TURAG_FELDBUS_ASEB_SYNC) {
		// sync request
		
		uint8_t* out = response;
		uint8_t low_byte = 0;
		uint8_t high_byte = 0;
		
		if (digital_inputs && digital_inputs_size > 0) {
			if (digital_inputs[0].value) low_byte = (1<<0);
			if (digital_inputs_size > 1) {
				if (digital_inputs[1].value) low_byte |= (1<<1);
				if (digital_inputs_size > 2) {
					if (digital_inputs[2].value) low_byte |= (1<<2);
					if (digital_inputs_size > 3) {
						if (digital_inputs[3].value) low_byte |= (1<<3);
						if (digital_inputs_size > 4) {
							if (digital_inputs[4].value) low_byte |= (1<<4);
							if (digital_inputs_size > 5) {
								if (digital_inputs[5].value) low_byte |= (1<<5);
								if (digital_inputs_size > 6) {
									if (digital_inputs[6].value) low_byte |= (1<<6);
									if (digital_inputs_size > 7) {
										if (digital_inputs[7].value) low_byte |= (1<<7);
										if (digital_inputs_size > 8) {
											if (digital_inputs[8].value) high_byte = (1<<0);
											if (digital_inputs_size > 9) {
												if (digital_inputs[9].value) high_byte |= (1<<1);
												if (digital_inputs_size > 10) {
													if (digital_inputs[10].value) high_byte |= (1<<2);
													if (digital_inputs_size > 11) {
														if (digital_inputs[11].value) high_byte |= (1<<3);
														if (digital_inputs_size > 12) {
															if (digital_inputs[12].value) high_byte |= (1<<4);
															if (digital_inputs_size > 13) {
																if (digital_inputs[13].value) high_byte |= (1<<5);
																if (digital_inputs_size > 14) {
																	if (digital_inputs[14].value) high_byte |= (1<<6);
																	if (digital_inputs_size > 15) {
																		if (digital_inputs[15].value) high_byte |= (1<<7);	
																	}
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
			out[0] = low_byte;
			out[1] = high_byte;
			out += 2;
		}
		
		int i;
		if (analog_inputs) {
			for (i = 0; i < analog_inputs_size; ++i) {
				out[0] = ((uint8_t*)&analog_inputs[i].value)[0];
				out[1] = ((uint8_t*)&analog_inputs[i].value)[1];
				out += 2;
			}
		}
		
		return out - response;
	
	} else if (	message[0] >= TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_OUTPUT && 
				message[0] < TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_OUTPUT + TURAG_FELDBUS_ASEB_MAX_CHANNELS_PER_TYPE) {
		// set request digital output
		
		uint8_t digital_output_index = message[0] - TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_OUTPUT;
		
		if (digital_outputs && digital_output_index < digital_outputs_size) {
			if (message_length == 2) {
				digital_outputs[digital_output_index].value = message[1] ? 1 : 0;
				return 0;
			} else {
				response[0] = digital_outputs[digital_output_index].value;
				return 1;
			}
		} else {
			return TURAG_FELDBUS_NO_ANSWER;
		}
		
	} else if (	message[0] >= TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT && 
				message[0] < TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT + TURAG_FELDBUS_ASEB_MAX_CHANNELS_PER_TYPE) {
		// set request PWM
		
		uint8_t pwm_output_index = message[0] - TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT;
		
		if (pwm_outputs && pwm_output_index < pwm_outputs_size) {
			uint8_t* value = (uint8_t*)&pwm_outputs[pwm_output_index].target_value;
			if (message_length == 3) {
				value[0] = message[1];
				value[1] = message[2];
				return 0;
			} else {
				response[0] = value[0];
				response[1] = value[1];
				return 2;
			}
		} else {
			return TURAG_FELDBUS_NO_ANSWER;
		}
    }else if(message[0] == TURAG_FELDBUS_ASEB_PWM_SPEED){
        uint8_t pwm_output_index = message[1] - TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT;

        if (pwm_outputs && pwm_output_index < pwm_outputs_size) {
            uint8_t* value = (uint8_t*)&pwm_outputs[pwm_output_index].speed;
            if (message_length == 4) {
                value[0] = message[2];
                value[1] = message[3];
                return 0;
            } else {
                response[0] = value[0];
                response[1] = value[1];
                return 2;
            }
        } else {
            return TURAG_FELDBUS_NO_ANSWER;
        }
	} else if (message[0] == TURAG_FELDBUS_ASEB_NUMBER_OF_DIGITAL_INPUTS) {
		response[0] = digital_inputs_size;
		return 1;
	} else if (message[0] == TURAG_FELDBUS_ASEB_NUMBER_OF_DIGITAL_OUTPUTS) {
		response[0] = digital_outputs_size;
		return 1;
	} else if (message[0] == TURAG_FELDBUS_ASEB_NUMBER_OF_ANALOG_INPUTS) {
		response[0] = analog_inputs_size;
		return 1;
	} else if (message[0] == TURAG_FELDBUS_ASEB_ANALOG_INPUT_RESOLUTION) {
		response[0] = analog_resolution;
		return 1;
	} else if (message[0] == TURAG_FELDBUS_ASEB_ANALOG_INPUT_FACTOR) {
		uint8_t analog_index = message[1] - TURAG_FELDBUS_ASEB_INDEX_START_ANALOG_INPUT;
		
		if (analog_inputs && analog_index < analog_inputs_size) {
			uint8_t* value = (uint8_t*)&analog_inputs[analog_index].factor;
			response[0] = value[0];
			response[1] = value[1];
			response[2] = value[2];
			response[3] = value[3];
			return 4;
		} else {
			return TURAG_FELDBUS_NO_ANSWER;
		}
	} else if (message[0] == TURAG_FELDBUS_ASEB_NUMBER_OF_PWM_OUTPUTS) {
		response[0] = pwm_outputs_size;
		return 1;
	} else if (message[0] == TURAG_FELDBUS_ASEB_PWM_OUTPUT_FREQUENCY) {
		uint8_t pwm_index = message[1] - TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT;
		
		if (pwm_outputs && pwm_index < pwm_outputs_size) {
			uint8_t* value = (uint8_t*)&pwm_outputs[pwm_index].frequency;
			response[0] = value[0];
			response[1] = value[1];
			response[2] = value[2];
			response[3] = value[3];
			return 4;
		} else {
			return TURAG_FELDBUS_NO_ANSWER;
		}
	} else if (message[0] == TURAG_FELDBUS_ASEB_PWM_OUTPUT_MAX_VALUE) {
		uint8_t pwm_index = message[1] - TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT;
		
		if (pwm_outputs && pwm_index < pwm_outputs_size) {
			uint8_t* value = (uint8_t*)&pwm_outputs[pwm_index].max_value;
			response[0] = value[0];
			response[1] = value[1];
			return 2;
		} else {
			return TURAG_FELDBUS_NO_ANSWER;
		}
	} else if (message[0] == TURAG_FELDBUS_ASEB_CHANNEL_NAME) {
		const char* name = 0;
		uint8_t index = 0;
		
		if (message[1] < TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_INPUT + TURAG_FELDBUS_ASEB_MAX_CHANNELS_PER_TYPE) {
			index = message[1] - TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_INPUT;
			if (index < digital_inputs_size) name = digital_inputs[index].name;
		} else if (message[1] < TURAG_FELDBUS_ASEB_INDEX_START_ANALOG_INPUT + TURAG_FELDBUS_ASEB_MAX_CHANNELS_PER_TYPE) {
			index = message[1] - TURAG_FELDBUS_ASEB_INDEX_START_ANALOG_INPUT;
			if (index < analog_inputs_size) name = analog_inputs[index].name;
		} else if (message[1] < TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_OUTPUT + TURAG_FELDBUS_ASEB_MAX_CHANNELS_PER_TYPE) {
			index = message[1] - TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_OUTPUT;
			if (index < digital_outputs_size) name = digital_outputs[index].name;
		} else if (message[1] < TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT + TURAG_FELDBUS_ASEB_MAX_CHANNELS_PER_TYPE) {
			index = message[1] - TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT;
			if (index < pwm_outputs_size) name = pwm_outputs[index].name;
		}
		if (!name) return TURAG_FELDBUS_NO_ANSWER;
		
        FeldbusSize_t length = 0;

        length = strlen(name);
		if (length + TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH > TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE) {
			length = TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE - TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH;
		}
		
        memcpy(response, name, length);
        return length;
		
	} else if (message[0] == TURAG_FELDBUS_ASEB_CHANNEL_NAME_LENGTH) {
		const char* name = 0;
		uint8_t index = 0;
		
		if (message[1] < TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_INPUT + TURAG_FELDBUS_ASEB_MAX_CHANNELS_PER_TYPE) {
			index = message[1] - TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_INPUT;
			if (index < digital_inputs_size) name = digital_inputs[index].name;
		} else if (message[1] < TURAG_FELDBUS_ASEB_INDEX_START_ANALOG_INPUT + TURAG_FELDBUS_ASEB_MAX_CHANNELS_PER_TYPE) {
			index = message[1] - TURAG_FELDBUS_ASEB_INDEX_START_ANALOG_INPUT;
			if (index < analog_inputs_size) name = analog_inputs[index].name;
		} else if (message[1] < TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_OUTPUT + TURAG_FELDBUS_ASEB_MAX_CHANNELS_PER_TYPE) {
			index = message[1] - TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_OUTPUT;
			if (index < digital_outputs_size) name = digital_outputs[index].name;
		} else if (message[1] < TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT + TURAG_FELDBUS_ASEB_MAX_CHANNELS_PER_TYPE) {
			index = message[1] - TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT;
			if (index < pwm_outputs_size) name = pwm_outputs[index].name;
		}
		if (!name) return TURAG_FELDBUS_NO_ANSWER;
		
		FeldbusSize_t length = strlen(name);
		if (length + TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH > TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE) {
			length = TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE - TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH;
		}
		
		response[0] = length;
		return 1;
	} else if (message[0] == TURAG_FELDBUS_ASEB_SYNC_SIZE) {
		uint8_t size = 0;
		
		if (digital_inputs && digital_inputs_size > 0) {
			size += 2;
		}
		if (analog_inputs && analog_inputs_size > 0) {
			size += analog_inputs_size * 2;
		}
		response[0] = size + TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH + 1;
		return 1;
	}
	return TURAG_FELDBUS_NO_ANSWER;
}

