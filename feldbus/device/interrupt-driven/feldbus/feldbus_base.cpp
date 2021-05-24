
#include "feldbus_base.h"
#include <feldbus/util/murmurhash3.h>

#include <algorithm>


#define BUFFER_CHECK(length) static_assert((length) <= TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE, "Buffer overflow");


static FeldbusSize_t process_request(const uint8_t* message, FeldbusSize_t length, uint8_t* response);
static FeldbusSize_t process_broadcast(const uint8_t* message, FeldbusSize_t length, uint8_t* response, bool* assert_bus_low);
static void turag_feldbus_device_start_transmission(FeldbusAddress_t origin);
static inline bool turag_feldbus_device_uuid_check(const uint8_t* compare);

turag_feldbus_device_t turag_feldbus_device = {
	.transmitLength = 0,
	.txOffset = 0,
	.rxOffset = 0,
	.rx_length = 0,
	.overflow = 0,
	.package_lost_flag = 0,
	.buffer_overflow_flag = 0,
#if TURAG_FELDBUS_DEVICE_CONFIG_DEBUG_ENABLED
	.transmission_active = 0,
#endif	
	.toggleLedBlocked = 0,
	.packet_processor = 0,
	.broadcast_processor = 0,
	.my_address = 0,
	.name = 0,
	.name_length = 0,
	.versioninfo = 0,
	.version_info_length = 0,
	.device_protocol = 0,
	.device_type = 0,
	.packagecount_correct = 0,
	.packagecount_buffer_overflow = 0,
	.packagecount_lost = 0,
	.packagecount_chksum_mismatch = 0,
	.uptime_counter = 0
};



extern "C" void turag_feldbus_device_init(
		FeldbusAddress_t bus_address, uint32_t uuid,
		const char* name, const char* version_info,
		uint8_t device_protocol, uint8_t device_type,
		TuragFeldbusPacketProcessor packetProcessor,
		TuragFeldbusBroadcastProcessor broadcastProcessor)
{
	*(uint32_t*)turag_feldbus_device.uuid = uuid;

	turag_feldbus_device.packet_processor = packetProcessor;
	turag_feldbus_device.broadcast_processor = broadcastProcessor;
	turag_feldbus_device.my_address = bus_address;
	turag_feldbus_device.name = name;
	turag_feldbus_device.name_length = std::strlen(name);
	turag_feldbus_device.versioninfo = version_info;
	turag_feldbus_device.version_info_length = std::strlen(version_info);
	turag_feldbus_device.device_protocol = device_protocol;
	turag_feldbus_device.device_type = device_type;


	turag_feldbus_hardware_init();
	
	turag_feldbus_device_rts_off();
	turag_feldbus_device_activate_rx_interrupt();
	turag_feldbus_device_deactivate_dre_interrupt();
	turag_feldbus_device_deactivate_tx_interrupt();
}


extern "C" void turag_feldbus_do_processing(void) {
	// One might think it is unnecessary to disable interrupts just for
	// reading rx_length. But because rx_length is volatile the following
	// scenario is quite possible: we copy the the value of rx_length from memory
	// to our register. Then we receive a byte and turag_feldbus_device_byte_received() is called.
	// This function is executed in interrupt context and clears rx_length. Then we continue here
	// and evaluate the now invalid value of rx_length which was buffered
	// in our register. For this reason we need to disable all interrupts before
	// reading rx_length.
	turag_feldbus_device_begin_interrupt_protect();

	if (turag_feldbus_device.rx_length == 0) {
		turag_feldbus_device_end_interrupt_protect();
		return;
	}

	// we have to disable the receive interrupt to ensure that
	// the rx-buffer does not get corrupted by incoming data
	// (even though the protocol already enforces this and it should
	// not happen at all)
	turag_feldbus_device_deactivate_rx_interrupt();

	// we copy the value of rx_length because it is volatile and accessing
	// it is expensive.
	FeldbusSize_t length = turag_feldbus_device.rx_length;
	turag_feldbus_device.rx_length = 0;

	// we release the blinking to indicate that the user program is
	// still calling turag_feldbus_do_processing() as required.
	turag_feldbus_device.toggleLedBlocked = false;
	turag_feldbus_device_end_interrupt_protect();

	// if we are here, we have a package (with length > 1 that is addressed to us) safe in our buffer
	// and we can start working on it


	// calculate checksum
#if TURAG_FELDBUS_DEVICE_CONFIG_CRC_TYPE == TURAG_FELDBUS_CHECKSUM_XOR
	if (!xor_checksum_check(turag_feldbus_device.rxbuf, length - 1, turag_feldbus_device.rxbuf[length - 1]))
#elif TURAG_FELDBUS_DEVICE_CONFIG_CRC_TYPE == TURAG_FELDBUS_CHECKSUM_CRC8
	if (!turag_crc8_check(turag_feldbus_device.rxbuf, length - 1, turag_feldbus_device.rxbuf[length - 1]))
#endif
	{
		++turag_feldbus_device.packagecount_chksum_mismatch;
		turag_feldbus_device_activate_rx_interrupt();
		return;
	}

	++turag_feldbus_device.packagecount_correct;

	// The address is already checked in turag_feldbus_device_receive_timeout_occured().
	// Thus the second check is only necessary
	// to distinguish broadcasts from regular packages.
	if (*((FeldbusAddress_t*)turag_feldbus_device.rxbuf) != TURAG_FELDBUS_BROADCAST_ADDR) {

		turag_feldbus_device.transmitLength = process_request(
			turag_feldbus_device.rxbuf + TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH,
			length - (1 + TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH),
			turag_feldbus_device.txbuf + TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH) + TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH;


		// this happens if the device protocol or the user code returned TURAG_FELDBUS_NO_ANSWER.
		if (turag_feldbus_device.transmitLength == 0) {
			turag_feldbus_device_activate_rx_interrupt();
		} else {
			turag_feldbus_device_start_transmission(turag_feldbus_device.my_address);
		}
	} else {
		bool assert_bus_low = false;

		// broadcasts
		turag_feldbus_device.transmitLength = process_broadcast(
			turag_feldbus_device.rxbuf + TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH,
			length - (1 + TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH),
			turag_feldbus_device.txbuf + TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH,
			&assert_bus_low) + TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH;

		if (assert_bus_low) {
			turag_feldbus_slave_assert_low();
		}

		// this happens if the device protocol or the user code returned TURAG_FELDBUS_NO_ANSWER.
		if (turag_feldbus_device.transmitLength == 0) {
			turag_feldbus_device_activate_rx_interrupt();
		} else {
			turag_feldbus_device_start_transmission(TURAG_FELDBUS_BROADCAST_ADDR);
		}
	}
}


extern "C" uint32_t turag_feldbus_device_hash_uuid(const uint8_t* key, size_t length) {
	uint32_t default_seed = 0x55555555;
	uint32_t uuid = murmurhash3_x86_32(key, length, default_seed);

	// prevent the unlikely case of getting 0, because we use this as an indicator for an old device
	// not supporting UUIDs
	if (uuid == 0) {
		uuid = murmurhash3_x86_32(key, length, default_seed + 1);
	}

	return uuid;
}


static void turag_feldbus_device_start_transmission(FeldbusAddress_t origin) {
	// set correct return address
	*(FeldbusAddress_t*)turag_feldbus_device.txbuf = TURAG_FELDBUS_MASTER_ADDR | origin;

	// calculate correct checksum and initiate transmission
#if TURAG_FELDBUS_DEVICE_CONFIG_CRC_TYPE == TURAG_FELDBUS_CHECKSUM_XOR
	turag_feldbus_device.txbuf[turag_feldbus_device.transmitLength] = xor_checksum_calculate(turag_feldbus_device.txbuf, turag_feldbus_device.transmitLength);
	turag_feldbus_device.transmitLength += 1;
#elif TURAG_FELDBUS_DEVICE_CONFIG_CRC_TYPE == TURAG_FELDBUS_CHECKSUM_CRC8
	turag_feldbus_device.txbuf[turag_feldbus_device.transmitLength] = turag_crc8_calculate(turag_feldbus_device.txbuf, turag_feldbus_device.transmitLength);
	turag_feldbus_device.transmitLength += 1;
#endif


#if TURAG_FELDBUS_DEVICE_CONFIG_DEBUG_ENABLED
		turag_feldbus_device.transmission_active = 1;
#endif

	turag_feldbus_device.txOffset = 0;

	turag_feldbus_device_deactivate_rx_interrupt();
	turag_feldbus_device_rts_on();
	turag_feldbus_device_activate_dre_interrupt();
}


static inline bool turag_feldbus_device_uuid_check(const uint8_t* compare) {
	return
			turag_feldbus_device.uuid[0] == compare[0] &&
			turag_feldbus_device.uuid[1] == compare[1] &&
			turag_feldbus_device.uuid[2] == compare[2] &&
			turag_feldbus_device.uuid[3] == compare[3];
}


static FeldbusSize_t process_request(const uint8_t* message, FeldbusSize_t length, uint8_t* response) {
	if (length == 0) {
		// we received a ping request -> respond with empty packet
		return 0;
	} else if (message[0] == 0) {
		// first data byte is zero -> reserved packet
		if (length == 1) {
			// received a device info request packet
			BUFFER_CHECK(11);

			FeldbusSize_t extInfo_length = std::min((size_t)TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE, turag_feldbus_device.name_length + turag_feldbus_device.version_info_length + 5);

			response[0] = turag_feldbus_device.device_protocol;
			response[1] = turag_feldbus_device.device_type;
			response[2] = TURAG_FELDBUS_DEVICE_CONFIG_CRC_TYPE | 0x88;
			response[3] = extInfo_length & 0xff;
			response[4] = (extInfo_length >> 8) & 0xff;
			response[5] = turag_feldbus_device.uuid[0];
			response[6] = turag_feldbus_device.uuid[1];
			response[7] = turag_feldbus_device.uuid[2];
			response[8] = turag_feldbus_device.uuid[3];
			response[9] = TURAG_FELDBUS_DEVICE_CONFIG_UPTIME_FREQUENCY & 0xff;
			response[10] = TURAG_FELDBUS_DEVICE_CONFIG_UPTIME_FREQUENCY >> 8;
			return 11;
		} else if (length == 2) {
			switch (message[1]) {
			case TURAG_FELDBUS_DEVICE_COMMAND_DEVICE_NAME: {
				FeldbusSize_t name_length = std::min(turag_feldbus_device.name_length, (size_t)(TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE - TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH));
				memcpy(response, turag_feldbus_device.name, name_length);
				return name_length;
			}
			case TURAG_FELDBUS_DEVICE_COMMAND_UPTIME_COUNTER:
#if (TURAG_FELDBUS_DEVICE_CONFIG_UPTIME_FREQUENCY>0) && (TURAG_FELDBUS_DEVICE_CONFIG_UPTIME_FREQUENCY<=65535)
				BUFFER_CHECK(sizeof(turag_feldbus_device.uptime_counter));
				memcpy(response, &turag_feldbus_device.uptime_counter, sizeof(turag_feldbus_device.uptime_counter));
				return sizeof(turag_feldbus_device.uptime_counter);
#else
				static_assert(sizeof(uint32_t) + TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH <= TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE, "Buffer overflow");
				return = sizeof(uint32_t);
#endif
				break;

			case TURAG_FELDBUS_DEVICE_COMMAND_VERSIONINFO: {
				FeldbusSize_t version_length = std::min(turag_feldbus_device.version_info_length, (size_t)TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE);
				memcpy(response, turag_feldbus_device.versioninfo, version_length);
				return version_length;
			}
			case TURAG_FELDBUS_DEVICE_COMMAND_PACKAGE_COUNT_CORRECT:
				BUFFER_CHECK(sizeof(turag_feldbus_device.packagecount_correct));
				memcpy(response, &turag_feldbus_device.packagecount_correct, sizeof(turag_feldbus_device.packagecount_correct));
				return sizeof(turag_feldbus_device.packagecount_correct);

			case TURAG_FELDBUS_DEVICE_COMMAND_PACKAGE_COUNT_BUFFEROVERFLOW:
				BUFFER_CHECK(sizeof(turag_feldbus_device.packagecount_buffer_overflow));
				memcpy(response, &turag_feldbus_device.packagecount_buffer_overflow, sizeof(turag_feldbus_device.packagecount_buffer_overflow));
				return sizeof(turag_feldbus_device.packagecount_buffer_overflow);

			case TURAG_FELDBUS_DEVICE_COMMAND_PACKAGE_COUNT_LOST:
				BUFFER_CHECK(sizeof(turag_feldbus_device.packagecount_lost));
				memcpy(response, &turag_feldbus_device.packagecount_lost, sizeof(turag_feldbus_device.packagecount_lost));
				return sizeof(turag_feldbus_device.packagecount_lost);

			case TURAG_FELDBUS_DEVICE_COMMAND_PACKAGE_COUNT_CHKSUM_MISMATCH:
				BUFFER_CHECK(sizeof(turag_feldbus_device.packagecount_chksum_mismatch));
				memcpy(response, &turag_feldbus_device.packagecount_chksum_mismatch, sizeof(turag_feldbus_device.packagecount_chksum_mismatch));
				return sizeof(turag_feldbus_device.packagecount_chksum_mismatch);

			case TURAG_FELDBUS_DEVICE_COMMAND_PACKAGE_COUNT_ALL: {
				constexpr size_t size = sizeof(turag_feldbus_device.packagecount_correct) + sizeof(turag_feldbus_device.packagecount_buffer_overflow) + sizeof(turag_feldbus_device.packagecount_lost) + sizeof(turag_feldbus_device.packagecount_chksum_mismatch);
				BUFFER_CHECK(size);
				memcpy(response, &turag_feldbus_device.packagecount_correct, size);
				return size;
			}
			case TURAG_FELDBUS_DEVICE_COMMAND_RESET_PACKAGE_COUNT:
				turag_feldbus_device.packagecount_correct = 0;
				turag_feldbus_device.packagecount_buffer_overflow = 0;
				turag_feldbus_device.packagecount_lost = 0;
				turag_feldbus_device.packagecount_chksum_mismatch = 0;
				return 0;

			case TURAG_FELDBUS_DEVICE_COMMAND_GET_UUID:
				BUFFER_CHECK(sizeof(turag_feldbus_device.uuid));
				memcpy(response, turag_feldbus_device.uuid, sizeof(turag_feldbus_device.uuid));
				return sizeof(turag_feldbus_device.uuid);

			case TURAG_FELDBUS_DEVICE_COMMAND_GET_EXTENDED_INFO: {
				BUFFER_CHECK(7);

				uint8_t name_length = std::min(turag_feldbus_device.name_length, (size_t)(TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE - 6));
				uint8_t version_info_length = std::min(turag_feldbus_device.version_info_length, (size_t)(TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE - 5 - name_length));

				response[0] = 0;
				response[1] = name_length;
				response[2] = version_info_length;
				response[3] = TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE & 0xff;
				response[4] = (TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE >> 8) & 0xff;
				memcpy(response + 5, turag_feldbus_device.name, name_length);
				memcpy(response + 5 + name_length, turag_feldbus_device.versioninfo, version_info_length);

				return 5 + name_length + version_info_length;
			}

			default:
				// unhandled reserved packet with length == 3
				return TURAG_FELDBUS_NO_ANSWER;
			}
		} else {
			// unhandled reserved packet with length > 3
			return TURAG_FELDBUS_NO_ANSWER;
		}
	} else {
		// received some other packet --> let somebody else process it
		if (turag_feldbus_device.packet_processor) {
			return turag_feldbus_device.packet_processor(message, length, response);
		}
	}
	return TURAG_FELDBUS_NO_ANSWER;
}


static FeldbusSize_t process_broadcast(const uint8_t* message, FeldbusSize_t length, uint8_t* response, bool* assert_bus_low) {
	// estimate for buffer requirements
	BUFFER_CHECK(20);

	if (length == 0) {
		// compatibility mode to support deprecated Broadcasts without protocol-ID
		if (turag_feldbus_device.broadcast_processor) {
			turag_feldbus_device.broadcast_processor(0, 0, TURAG_FELDBUS_DEVICE_PROTOCOL_LOKALISIERUNGSSENSOREN);
		}
		return TURAG_FELDBUS_NO_ANSWER;
	} else if (message[0] == turag_feldbus_device.device_protocol) {
		// defer processing of device protocol broadcasts
		if (turag_feldbus_device.broadcast_processor) {
			turag_feldbus_device.broadcast_processor(message + 1, length - 1, message[0]);
		}
		return TURAG_FELDBUS_NO_ANSWER;
	} else if (message[0] == TURAG_FELDBUS_BROADCAST_TO_ALL_DEVICES && length > 1) {
		// basic protocol broadcasts
		switch (message[1]) {
		case TURAG_FELDBUS_DEVICE_BROADCAST_UUID: {
			if (length == 2 && turag_feldbus_device.my_address == 0) {
				// ping request for all devices without valid bus address -> return UUID
				memcpy(response, turag_feldbus_device.uuid, sizeof(turag_feldbus_device.uuid));
				return sizeof(turag_feldbus_device.uuid);
			} else if (length >= 6 && turag_feldbus_device_uuid_check(message + 2)) {
				// packets directed at devices with a specific UUID
				switch (length) {
				case 6:
					// ping request -> respond with empty packet
					return 0;

				case 7:
					switch (message[6]) {
					case TURAG_FELDBUS_DEVICE_BROADCAST_UUID_ADDRESS:
						// return Bus address
						response[0] = turag_feldbus_device.my_address & 0xFF;
#if TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH == 2
						response[1] = (turag_feldbus_device.my_address >> 8) & 0xFF;
#endif
						return TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH;

					case TURAG_FELDBUS_DEVICE_BROADCAST_UUID_RESET_ADDRESS:
						// reset bus address
						turag_feldbus_device.my_address = 0;
						return 0;
					}
					break;

				case 7 + sizeof(FeldbusAddress_t):
					switch (message[6]) {
					case TURAG_FELDBUS_DEVICE_BROADCAST_UUID_ADDRESS: {
						// set Bus address
# if TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH == 1
						FeldbusAddress_t new_address = message[7];
						if (new_address > 0 && new_address < TURAG_FELDBUS_MASTER_ADDR) {
							turag_feldbus_device.my_address = new_address;
							response[0] = 1;
						}
# elif TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH == 2
						FeldbusAddress_t new_address = (uint16_t)message[7] + ((uint16_t)message[8] << 8);
						if (new_address > 0 && new_address < TURAG_FELDBUS_MASTER_ADDR2) {
							turag_feldbus_device.my_address = new_address;
							response[0] = 1;
						}
# endif
						else {
							response[0] = 0;
						}
						return 1;
					}
					}
					break;
				}
			}
		}
		case TURAG_FELDBUS_DEVICE_BROADCAST_ENABLE_NEIGHBOURS:
			turag_feldbus_device_enable_bus_neighbours();
			return TURAG_FELDBUS_NO_ANSWER;

		case TURAG_FELDBUS_DEVICE_BROADCAST_DISABLE_NEIGHBOURS:
			turag_feldbus_device_disable_bus_neighbours();
			return TURAG_FELDBUS_NO_ANSWER;

		case TURAG_FELDBUS_DEVICE_BROADCAST_RESET_ADDRESSES:
			turag_feldbus_device.my_address = 0;
			return TURAG_FELDBUS_NO_ANSWER;

		case TURAG_FELDBUS_DEVICE_BROADCAST_REQUEST_BUS_ASSERTION:
			if (length > 2) {
				uint8_t mask_length = message[2];
				if (mask_length < 1 || mask_length > 32) {
					return TURAG_FELDBUS_NO_ANSWER;
				}

				uint32_t search_mask = 0;
				for (int i = 0; i < mask_length; ++i) {
					search_mask |= (1 << i);
				}

				uint32_t search_address = 0;
				if (length > 3) {
					search_address |= message[3];

					if (length > 4) {
						search_address |= (uint32_t)message[4] << 8;

						if (length > 5) {
							search_address |= (uint32_t)message[5] << 16;

							if (length > 6) {
								search_address |= (uint32_t)message[6] << 24;
							}
						}
					}
				}

				if ((*(uint32_t*)turag_feldbus_device.uuid & search_mask) == search_address) {
					*assert_bus_low = true;
				}
			}
		}
	}
	return TURAG_FELDBUS_NO_ANSWER;
}

extern "C" void __attribute__((weak)) turag_feldbus_device_toggle_led(void) {

}



#if TURAG_FELDBUS_DEVICE_CONFIG_DEBUG_ENABLED
static uint8_t digit_to_hex(uint8_t dig) {
	dig &= 0xF;
	dig += '0';
	if (dig > '9') dig += 'A' - '0' - 10;
	return dig;
}


// public debug functions
void print_text(const char *buf) {
	while (turag_feldbus_device.transmission_active);

	uint8_t i = 0;

	while(*buf) {
		turag_feldbus_device.txbuf[i] = *buf;
		++i;
		++buf;

		if (i >= TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE) {
			i = 1;
			break;
		}
	}
	turag_feldbus_device.transmitLength = i;
	start_transmission();
}

void print_char(uint8_t x) {
	while (turag_feldbus_device.transmission_active);

	turag_feldbus_device.txbuf[0] = '0';
	turag_feldbus_device.txbuf[1] = 'x';
	turag_feldbus_device.txbuf[2] = digit_to_hex(x >> 4);
	turag_feldbus_device.txbuf[3] = digit_to_hex(x);

	turag_feldbus_device.transmitLength = 4;
	start_transmission();
}

void print_short(uint16_t x) {
	while (turag_feldbus_device.transmission_active);

	turag_feldbus_device.txbuf[0] = '0';
	turag_feldbus_device.txbuf[1] = 'x';
	turag_feldbus_device.txbuf[2] = digit_to_hex(x >> 12);
	turag_feldbus_device.txbuf[3] = digit_to_hex(x >> 8);
	turag_feldbus_device.txbuf[4] = digit_to_hex(x >> 4);
	turag_feldbus_device.txbuf[5] = digit_to_hex(x);
	turag_feldbus_device.txbuf[6] = '\r';
	turag_feldbus_device.txbuf[7] = '\n';

	turag_feldbus_device.transmitLength = 8;
	start_transmission();
}
void print_short_nn(uint16_t x) {
	while (turag_feldbus_device.transmission_active);

	turag_feldbus_device.txbuf[0] = '0';
	turag_feldbus_device.txbuf[1] = 'x';
	turag_feldbus_device.txbuf[2] = digit_to_hex(x >> 12);
	turag_feldbus_device.txbuf[3] = digit_to_hex(x >> 8);
	turag_feldbus_device.txbuf[4] = digit_to_hex(x >> 4);
	turag_feldbus_device.txbuf[5] = digit_to_hex(x);

	turag_feldbus_device.transmitLength = 6;
	turag_feldbus_device.transmission_active = 1;
	start_transmission();
}

void print_sshort(int16_t x) {
	while (turag_feldbus_device.transmission_active);

	if (x<0) {
		turag_feldbus_device.txbuf[0] = '-';
		x=-x;
	} else turag_feldbus_device.txbuf[0] = '+';

	turag_feldbus_device.txbuf[1] = '0';
	turag_feldbus_device.txbuf[2] = 'x';
	turag_feldbus_device.txbuf[3] = digit_to_hex(x >> 12);
	turag_feldbus_device.txbuf[4] = digit_to_hex(x >> 8);
	turag_feldbus_device.txbuf[5] = digit_to_hex(x >> 4);
	turag_feldbus_device.txbuf[6] = digit_to_hex(x);
	turag_feldbus_device.txbuf[7] = '\r';
	turag_feldbus_device.txbuf[8] = '\n';

	turag_feldbus_device.transmitLength = 9;
	start_transmission();
}
void print_sshort_nn(int16_t x) {
	while (turag_feldbus_device.transmission_active);

	if (x<0) {
		turag_feldbus_device.txbuf[0] = '-';
		x=-x;
	} else turag_feldbus_device.txbuf[0] = '+';

	turag_feldbus_device.txbuf[1] = '0';
	turag_feldbus_device.txbuf[2] = 'x';
	turag_feldbus_device.txbuf[3] = digit_to_hex(x >> 12);
	turag_feldbus_device.txbuf[4] = digit_to_hex(x >> 8);
	turag_feldbus_device.txbuf[5] = digit_to_hex(x >> 4);
	turag_feldbus_device.txbuf[6] = digit_to_hex(x);

	turag_feldbus_device.transmitLength = 7;
	start_transmission();
}

void print_long(uint32_t x) {
	while (turag_feldbus_device.transmission_active);

	turag_feldbus_device.txbuf[0] = '0';
	turag_feldbus_device.txbuf[1] = 'x';
	turag_feldbus_device.txbuf[2] = digit_to_hex(x >> 28);
	turag_feldbus_device.txbuf[3] = digit_to_hex(x >> 24);
	turag_feldbus_device.txbuf[4] = digit_to_hex(x >> 20);
	turag_feldbus_device.txbuf[5] = digit_to_hex(x >> 16);
	turag_feldbus_device.txbuf[6] = digit_to_hex(x >> 12);
	turag_feldbus_device.txbuf[7] = digit_to_hex(x >> 8);
	turag_feldbus_device.txbuf[8] = digit_to_hex(x >> 4);
	turag_feldbus_device.txbuf[9] = digit_to_hex(x);
	turag_feldbus_device.txbuf[10] = '\r';
	turag_feldbus_device.txbuf[11] = '\n';


	turag_feldbus_device.transmitLength = 12;
	start_transmission();
}

void print_short_d(int16_t x) {
	while (turag_feldbus_device.transmission_active);

	turag_feldbus_device.txbuf[0] = ' ';
	turag_feldbus_device.txbuf[1] = ' ';
	turag_feldbus_device.txbuf[2] = ' ';
	turag_feldbus_device.txbuf[3] = ' ';
	turag_feldbus_device.txbuf[4] = ' ';
	turag_feldbus_device.txbuf[5] = ' ';
	turag_feldbus_device.txbuf[6] = '\r';
	turag_feldbus_device.txbuf[7] = '\n';
	itoa(x, (char*)turag_feldbus_device.txbuf, 10);

	turag_feldbus_device.transmitLength = 8;
	start_transmission();
}

void print_slong(int32_t x) {
	while (turag_feldbus_device.transmission_active);

	if (x<0) {
		turag_feldbus_device.txbuf[0] = '-';
		x=-x;
	} else turag_feldbus_device.txbuf[0] = '+';

	turag_feldbus_device.txbuf[1] = '0';
	turag_feldbus_device.txbuf[2] = 'x';
	turag_feldbus_device.txbuf[3] = digit_to_hex(x >> 28);
	turag_feldbus_device.txbuf[4] = digit_to_hex(x >> 24);
	turag_feldbus_device.txbuf[5] = digit_to_hex(x >> 20);
	turag_feldbus_device.txbuf[6] = digit_to_hex(x >> 16);
	turag_feldbus_device.txbuf[7] = digit_to_hex(x >> 12);
	turag_feldbus_device.txbuf[8] = digit_to_hex(x >> 8);
	turag_feldbus_device.txbuf[9] = digit_to_hex(x >> 4);
	turag_feldbus_device.txbuf[10] = digit_to_hex(x);
	turag_feldbus_device.txbuf[11] = '\r';
	turag_feldbus_device.txbuf[12] = '\n';

	turag_feldbus_device.transmitLength = 13;
	start_transmission();
}
#endif

