/*
 * feldbus_slave_base.cpp
 *
 *  Created on: 20.11.2016
 *      Author: martin
 */


#include <tina++/tina.h>

#if TURAG_USE_TURAG_FELDBUS_SLAVE


#include "feldbus_slave_base.h"
#include <tina++/feldbus_slave_driver.h>

#include <tina++/crc.h>
#include <tina++/time.h>

#include <cstring>

#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
# include <cstdarg>
# include <tina/utils/mini-printf.h>
#endif


namespace TURAG {
namespace Feldbus {
namespace Slave {


Base::Info Base::info = {
		TURAG_FELDBUS_DEVICE_NAME,
		TURAG_FELDBUS_DEVICE_VERSIONINFO,
		#if TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE
			0, 0, 0, 0
		#endif
};

Base::PacketProcessor Base::packetProcessor = nullptr;
#if TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE
	Base::BroadcastProcessor Base::broadcastProcessor = nullptr;
#endif




void Base::init(PacketProcessor packetProcessor_, BroadcastProcessor broadcastProcessor_) {
	packetProcessor = packetProcessor_;
#if TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE
	broadcastProcessor = broadcastProcessor_;
#else
	(void)broadcastProcessor_;
#endif
}




FeldbusSize_t Base::processPacket(const uint8_t* message, FeldbusSize_t length, uint8_t* response) {
	// if we are here, we have a package (with length>1 that is adressed to us) safe in our buffer
	// and we can start working on it
	FeldbusSize_t responseLength;

	// Calculate checksum. If it's wrong, ignore packet.
#if (TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE == TURAG_FELDBUS_CHECKSUM_XOR)
	if (!XOR::check(message, length - 1, message[length - 1]))
#elif (TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE == TURAG_FELDBUS_CHECKSUM_CRC8_ICODE)
	if (!CRC8::check(message, length - 1, message[length - 1]))
#endif
	{
#if TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE
		++info.packetcount_chksum_mismatch;
#endif
		return 0;
	}

	// packet is correct.
#if TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE
	++info.packetcount_correct;
#endif

	// The address is already checked in the driver.
	// Thus the second check is only necessary
	// to distinguish broadcasts from regular packages.
#if TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE
	// message addressed to me?
# if TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 1
	if (message[0] == MY_ADDR) {
# elif TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 2
	if (message[0] == (MY_ADDR & 0xff) && message[1] == (MY_ADDR >> 8)) {
# endif
#endif
		if (length == 1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH) {
			// we received a ping request -> respond with empty packet (address + checksum)
			responseLength = TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1;
		} else if (message[TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] == 0) {
			// first data byte is zero -> reserved packet
			if (length == 2 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH) {
				// received a debug packet
				static_assert(11 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1 <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
				response[0 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = TURAG_FELDBUS_DEVICE_PROTOCOL;
				response[1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = TURAG_FELDBUS_DEVICE_TYPE_ID;
#if TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE
				response[2 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE | 0x80;
#else
				response[2 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE;
#endif
				response[3 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE & 0xff;
#if TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE > 255
				response[4 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = (TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE >> 8) & 0xff;
#else
				response[4 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = 0;
#endif
				// reserved bytes don't need to be written.
//				response[5 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = 0;
//				response[6 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = 0;

				response[7 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = sizeof(TURAG_FELDBUS_DEVICE_NAME) - 1;
				response[8 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = sizeof(TURAG_FELDBUS_DEVICE_VERSIONINFO) - 1;
				response[9 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = SystemTime::frequency() & 0xff;
				response[10 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = (SystemTime::frequency() >> 8) & 0xff;
				responseLength = 11 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1;
			} else if (length == 3 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH) {
				switch (message[1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH]) {
				case TURAG_FELDBUS_SLAVE_COMMAND_DEVICE_NAME: {
					static_assert(sizeof(TURAG_FELDBUS_DEVICE_NAME) - 1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1 <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
					std::memcpy(response + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH, info.name, sizeof(TURAG_FELDBUS_DEVICE_NAME) - 1);
					responseLength = sizeof(TURAG_FELDBUS_DEVICE_NAME) - 1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1;
					break;
				}
				case TURAG_FELDBUS_SLAVE_COMMAND_UPTIME_COUNTER: {
					static_assert(4 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1 <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
					TuragSystemTicks time = SystemTime::now().toTicks();
					response[0 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = time & 0xFF;
					response[1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = (time >> 8) & 0xFF;
					response[2 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = (time >> 16) & 0xFF;
					response[3 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = (time >> 24) & 0xFF;
					responseLength = 4 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1;
					break;
				}
				case TURAG_FELDBUS_SLAVE_COMMAND_VERSIONINFO: {
					static_assert(sizeof(TURAG_FELDBUS_DEVICE_VERSIONINFO) - 1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1 <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
					std::memcpy(response + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH, info.versioninfo, sizeof(TURAG_FELDBUS_DEVICE_VERSIONINFO) - 1);
					responseLength = sizeof(TURAG_FELDBUS_DEVICE_VERSIONINFO) - 1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1;
					break;
				}
#if TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE
				case TURAG_FELDBUS_SLAVE_COMMAND_PACKAGE_COUNT_CORRECT: {
					static_assert(sizeof(info.packetcount_correct) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1 <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
					std::memcpy(response + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH, &info.packetcount_correct, sizeof(info.packetcount_correct));
					responseLength = sizeof(info.packetcount_correct) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1;
					break;
				}
				case TURAG_FELDBUS_SLAVE_COMMAND_PACKAGE_COUNT_BUFFEROVERFLOW: {
					static_assert(sizeof(info.packetcount_buffer_overflow) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1 <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
					std::memcpy(response + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH, &info.packetcount_buffer_overflow, sizeof(info.packetcount_buffer_overflow));
					responseLength = sizeof(info.packetcount_buffer_overflow) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1;
					break;
				}
				case TURAG_FELDBUS_SLAVE_COMMAND_PACKAGE_COUNT_LOST: {
					static_assert(sizeof(info.packetcount_lost) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1 <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
					std::memcpy(response + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH, &info.packetcount_lost, sizeof(info.packetcount_lost));
					responseLength = sizeof(info.packetcount_lost) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1;
					break;
				}
				case TURAG_FELDBUS_SLAVE_COMMAND_PACKAGE_COUNT_CHKSUM_MISMATCH: {
					static_assert(sizeof(info.packetcount_chksum_mismatch) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1 <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
					std::memcpy(response + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH, &info.packetcount_chksum_mismatch, sizeof(info.packetcount_chksum_mismatch));
					responseLength = sizeof(info.packetcount_chksum_mismatch) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1;
					break;
				}
#else
				case TURAG_FELDBUS_SLAVE_COMMAND_PACKAGE_COUNT_CORRECT:
				case TURAG_FELDBUS_SLAVE_COMMAND_PACKAGE_COUNT_BUFFEROVERFLOW:
				case TURAG_FELDBUS_SLAVE_COMMAND_PACKAGE_COUNT_LOST:
				case TURAG_FELDBUS_SLAVE_COMMAND_PACKAGE_COUNT_CHKSUM_MISMATCH: {
					static_assert(sizeof(uint32_t) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH  + 1 <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
					responseLength = sizeof(uint32_t) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1;
				}
#endif
				case TURAG_FELDBUS_SLAVE_COMMAND_PACKAGE_COUNT_ALL: {
#if TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE
					static_assert(sizeof(info.packetcount_correct) + sizeof(info.packetcount_buffer_overflow) + sizeof(info.packetcount_lost) + sizeof(info.packetcount_chksum_mismatch) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1 <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
					std::memcpy(response + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH, &info.packetcount_correct, sizeof(info.packetcount_correct) + sizeof(info.packetcount_buffer_overflow) + sizeof(info.packetcount_lost) + sizeof(info.packetcount_chksum_mismatch));
					responseLength = sizeof(info.packetcount_correct) + sizeof(info.packetcount_buffer_overflow) + sizeof(info.packetcount_lost) + sizeof(info.packetcount_chksum_mismatch) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1;
#else
					static_assert(sizeof(uint32_t) * 4 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1 <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
					responseLength = sizeof(uint32_t) * 4 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1;
#endif
					break;
				}
				case TURAG_FELDBUS_SLAVE_COMMAND_RESET_PACKAGE_COUNT: {
					static_assert(TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1 <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
#if TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE
					info.packetcount_correct = 0;
					info.packetcount_buffer_overflow = 0;
					info.packetcount_lost = 0;
					info.packetcount_chksum_mismatch = 0;
#endif
					responseLength = TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1;
					break;
				}
				default:
					// unhandled reserved packet with length == 3 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH
					return 0;
				}
			} else {
				// unhandled reserved packet with length > 3 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH
				return 0;
			}
		} else {
			// received some other packet --> let somebody else process it
			if (packetProcessor) {
				responseLength = packetProcessor(
						message + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH,
						length - (1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH),
						response + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH)
					+ TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1;

				// this happens if the device protocol or the user code returned TURAG_FELDBUS_IGNORE_PACKAGE.
				if (responseLength == 0) {
					return 0;
				}
			} else {
				// no custom packet processor -> ignore package
				return 0;
			}
		}

		// calculate correct checksum
#if TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE == TURAG_FELDBUS_CHECKSUM_XOR
		response[responseLength-1] = XOR::calculate(response, responseLength-1);
#elif TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE == TURAG_FELDBUS_CHECKSUM_CRC8_ICODE
		response[responseLength-1] = CRC8::calculate(response, responseLength-1);
#endif
		return responseLength;

	// not every device protocol requires broadcasts, so we can save a few bytes here
#if TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE
	} else {
		if (broadcastProcessor) {
			if (length == 1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH) {
				// compatibility mode to support deprecated Broadcasts without protocol-ID
				broadcastProcessor(0, 0, TURAG_FELDBUS_DEVICE_PROTOCOL_LOKALISIERUNGSSENSOREN);
			} else if (message[TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] == TURAG_FELDBUS_BROADCAST_TO_ALL_DEVICES ||
					message[TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] == TURAG_FELDBUS_DEVICE_PROTOCOL) {
				// otherwise process only the correct broadcasts
				broadcastProcessor(
						message + 1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH,
						length - (2 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH),
						message[TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH]);
			}
		}
		return 0;
	}
#endif
}

#if TURAG_FELDBUS_SLAVE_CONFIG_FLASH_LED
void Base::doLedPattern(unsigned frequency) {
	if (frequency >= 12) {
		unsigned COUNT_MAX = (frequency / 12 - 1);
		static unsigned count = 0;
		static uint8_t subcount = 0;

		++count;
		if (count > COUNT_MAX) {
			if (subcount == 0) {
				Driver::toggleLed();
			} else if (subcount == 1) {
				Driver::toggleLed();
			}
			subcount = (subcount+1) & 7;
			count = 0;
		}
	} else if (frequency >= 2) {
		static uint8_t count = 0;

		++count;
		if (count > (frequency / 2 - 1)) {
			Driver::toggleLed();
			count = 0;
		}
	} else {
		Driver::toggleLed();
	}
}
#endif

#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED

// public debug functions
void printf(const char *fmt, ...) {
	char buf[100];

	va_list arglist;
	va_start( arglist, fmt );
	unsigned length = mini_vsnprintf( buf, sizeof(buf), fmt, arglist );
	va_end( arglist );

	unsigned written = length > sizeof(buf) ? sizeof(buf) : length;
	Driver::transmitDebugData(buf, written);
}

void puts(const char* s) {
	Driver::transmitDebugData(s, std::strlen(s));
}

#endif

}
}
}


#endif // TURAG_USE_TURAG_FELDBUS_SLAVE
