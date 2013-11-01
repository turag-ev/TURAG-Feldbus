/**
 *  @brief		Base class for all TURAG feldbus devices
 *  @file		TURAGFeldbusDevice.cpp
 *  @date		04.04.2013
 *  @author		Martin Oemus
 * 
 */

#define LOG_SOURCE "B"

#include "TURAGFeldbusDevice.h"
#include <tina++/time.h>
#include <tina/crc/crc.h>
#include <tina/debug.h>
#include <tina/rs485.h>

namespace TURAG {
namespace Feldbus {

Mutex Device::rs485_mutex;
SystemTime Device::lastTransmission;
unsigned int Device::globalTransmissionErrorCounter = 0;

bool Device::transceive(uint8_t* input, int input_length, uint8_t* output, int output_length) {
    if (hasReachedTransmissionErrorLimit()) {
            errorf("DEVICE \"%s\" DYSFUNCTIONAL. PACKAGE DROPPED.", name);
            return false;
    } else {
//		infof("%s: transceive tx [", name);
//		for (int i = 0; i < input_length; ++i) {
//			infof("%.2x", input[i]);
//		}
//		info("]\n");

		bool success = false, checksum_correct = false;
		unsigned int attempt = 0;

		// mutex lock
		 Mutex::Lock lock( rs485_mutex );

		while (attempt < maxTransmissionAttempts && !(success && checksum_correct)) {
//			if (attempt>0) warningf("%s: %d failed transceive attempts", name, attempt);

			success = false;
			checksum_correct = false;

			// introduce smallest possible delay
#warning transmission rate of rs485-bus artificially reduced to avoid errors
			while(lastTransmission > get_current_tick() - ms_to_ticks(1));
			lastTransmission = get_current_tick();

			success = turag_rs485_transceive(input, input_length, output, output_length);

			if (success) {
				if (!output || output_length == 0) {
					checksum_correct = true;
				} else {
					checksum_correct = xor_checksum_check(output, output_length-1, output[output_length-1]);
				}
			} else {
				globalTransmissionErrorCounter++;
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
			if (output && output_length != 0) myTransmissionErrorCounter = 0;
			return true;
		} else {
			warningf("%s: rs485 transceive failed", name);
			myTransmissionErrorCounter += attempt;
			return false;
		}
    }
}

} // namespace Feldbus
} // namespace TURAG

