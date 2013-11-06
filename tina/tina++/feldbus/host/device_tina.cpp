/**
 *  @brief		Base class for all TURAG feldbus devices
 *  @file		TURAGFeldbusDevice.cpp
 *  @date		04.04.2013
 *  @author		Martin Oemus
 * 
 */

#define LOG_SOURCE "B"

#include "device.h"
#include <tina++/time.h>
#include <tina/crc/xor_checksum.h>
#include <tina/crc/crc8_icode/crc8_icode.h>
#include <tina/debug.h>
#include <tina/rs485.h>


namespace TURAG {
namespace Feldbus {

Mutex Device::rs485_mutex;
SystemTime Device::lastTransmission;
unsigned int Device::globalTransmissionErrorCounter = 0;

bool Device::transceive(uint8_t *transmit, int transmit_length, uint8_t *receive, int receive_length) {
    if (hasReachedTransmissionErrorLimit()) {
            errorf("DEVICE \"%s\" DYSFUNCTIONAL. PACKAGE DROPPED.", name);
            return false;
    } else {
//		infof("%s: transceive tx [", name);
//		for (int i = 0; i < input_length; ++i) {
//			infof("%.2x", input[i]);
//		}
//		info("]\n");

		switch (myChecksumType) {
		case ChecksumType::xor_based:
			transmit[transmit_length - 1] = xor_checksum_calculate(transmit, transmit_length - 1);
			break;

		case ChecksumType::crc8_icode:
			transmit[transmit_length - 1] = turag_crc8_calculate(transmit, transmit_length - 1);
			break;
		}

		// mutex lock
		 Mutex::Lock lock( rs485_mutex );


		bool success = false, checksum_correct = false;
		unsigned int attempt = 0;

		// we try to transmit until the transmission succeeds and the checksum is correct
		// or the number of transmission attempts is exceeded.
		while (attempt < maxTransmissionAttempts && !(success && checksum_correct)) {
//			if (attempt>0) warningf("%s: %d failed transceive attempts", name, attempt);

			success = false;
			checksum_correct = false;

			// introduce smallest possible delay
//#warning transmission rate of rs485-bus artificially reduced to avoid errors
//			while(lastTransmission > get_current_tick() - ms_to_ticks(1));
//			lastTransmission = get_current_tick();

			success = turag_rs485_transceive(transmit, transmit_length, receive, receive_length);

			if (success) {
				if (!receive || receive_length == 0) {
					// if the caller doesn't want to receive anything and the transmission was successful
					// we leave the loop (that happens in the case of broadcasts)
					checksum_correct = true;
				} else {
					// transmission seems fine, lets look at the checksum
					switch (myChecksumType) {
					case ChecksumType::xor_based:
						checksum_correct = xor_checksum_check(receive, receive_length-1, receive[receive_length-1]);
						break;

					case ChecksumType::crc8_icode:
						checksum_correct = turag_crc8_check(receive, receive_length-1, receive[receive_length-1]);
						break;
					}
				}
			} else {
				++globalTransmissionErrorCounter;
				if (!(globalTransmissionErrorCounter % 25)) {
					criticalf("%d failed transmissions on the bus so far.", globalTransmissionErrorCounter);
				}
			}
			++attempt;
		}

//		infof("%s: transceive rx success(%x|%x) [", name, success, checksum_correct);
//		for (int j = 0; j < output_length; ++j) {
//			infof("%.2x", output[j]);
//		}
//		info("]\n");

		if (success && checksum_correct) {
			// if we had a successful transmission, we reset the error counter
			// but resetting the error only makes sense if we got a response from the device
			if (receive && receive_length != 0) {
				myTransmissionErrorCounter = 0;
			}
			return true;
		} else {
			warningf("%s: rs485 transceive failed", name);
			myTransmissionErrorCounter += attempt;
			return false;
		}
    }
}


bool Device::isAvailable(void) {
	if (!hasCheckedAvailabilityYet) {
		unsigned char msg[2];
		unsigned char recv_buffer[2];
		msg[0] = myAddress;

		while (!hasCheckedAvailabilityYet && !hasReachedTransmissionErrorLimit()) {
			if (transceive(msg, sizeof(msg), recv_buffer, sizeof(recv_buffer))) {
				hasCheckedAvailabilityYet = true;
			}
		}
	}

	return !hasReachedTransmissionErrorLimit();
}

} // namespace Feldbus
} // namespace TURAG

