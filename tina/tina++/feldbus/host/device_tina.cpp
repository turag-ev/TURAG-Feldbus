/**
 *  @brief		Base class for all TURAG feldbus devices
 *  @file		device_tina.cpp
 *  @date		04.04.2013
 *  @author		Martin Oemus
 *
 */

#define TURAG_DEBUG_LOG_SOURCE "B"

#include "device.h"
#include <tina++/time.h>
#include <tina++/crc/xor.h>
#include <tina++/crc/crc8.h>
#include <tina/debug.h>
#include <tina/rs485.h>


namespace TURAG {
namespace Feldbus {

SystemTime Device::lastTransmission;
unsigned int Device::globalTransmissionErrorCounter = 0;

bool Device::transceive(uint8_t *transmit, int transmit_length, uint8_t *receive, int receive_length) {
    if (hasReachedTransmissionErrorLimit()) {
        static unsigned shit_displayed = 0;
        if (shit_displayed < 5) {
            errorf("DEVICE \"%s\" DYSFUNCTIONAL. PACKAGE DROPPED.", name);
            shit_displayed++;
        }
        return false;
    } else {
        switch (myChecksumType) {
        case ChecksumType::xor_based:
            transmit[transmit_length - 1] = XOR::calculate(transmit, transmit_length - 1);
            break;

        case ChecksumType::crc8_icode:
            transmit[transmit_length - 1] = CRC8::calculate(transmit, transmit_length - 1);
            break;
        }


        // 	turag_infof("%s: transceive tx [", name);
        // 	for (int i = 0; i < transmit_length; ++i) {
        // 		turag_infof("%.2x", transmit[i]);
        // 	}
        // 	info("]\n");


        bool success = false, checksum_correct = false;
        unsigned int attempt = 0;

        // we try to transmit until the transmission succeeds and the checksum is correct
        // or the number of transmission attempts is exceeded.
        while (attempt < maxTransmissionAttempts && !(success && checksum_correct)) {
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
                        checksum_correct = XOR::check(receive, receive_length-1, receive[receive_length-1]);
                        break;

                    case ChecksumType::crc8_icode:
                        checksum_correct = CRC8::check(receive, receive_length-1, receive[receive_length-1]);
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

        // 	turag_infof("%s: transceive rx success(%x|%x) [", name, success, checksum_correct);
        // 	for (int j = 0; j < receive_length; ++j) {
        // 		turag_infof("%.2x", receive[j]);
        // 	}
        // 	info("]\n");

        if (success && checksum_correct) {
            // if we had a successful transmission, we reset the error counter
            // but resetting the error only makes sense if we got a response from the device
            if (receive && receive_length != 0) {
                myTransmissionErrorCounter = 0;
            }
            return true;
        } else {
            turag_warningf("%s: rs485 transceive failed", name);
            myTransmissionErrorCounter += attempt;
            return false;
        }
    }
}


bool Device::isAvailable(void) {
    if (!turag_rs485_ready()) {
        turag_warningf("%s: isAvailable called with uninited RS485 bus", name);
        return false;
    }

	if (!hasCheckedAvailabilityYet) {
		Request<> request;
		Response<> response;

		while (!hasReachedTransmissionErrorLimit()) {
			if (transceive(request, &response)) {
				hasCheckedAvailabilityYet = true;
				return true;
			}
		}
	}

	return !hasReachedTransmissionErrorLimit();
}

bool Device::getDeviceInfo(DeviceInfo* device_info) {
    if (myDeviceInfo.bufferSize == 0) {
        Request<uint8_t> request;
        Response<DeviceInfo> response;

        request.data = 0;

        if (!transceive(request, &response)) {
            return false;
        }
        myDeviceInfo = response.data;
    }
    if (device_info) *device_info = myDeviceInfo;
    return true;
}

bool Device::receiveDeviceRealName(char* out_real_name) {
    if (!out_real_name) {
	return false;
    }
    
    if (myDeviceInfo.bufferSize == 0) {
        if (!getDeviceInfo(nullptr)) {
            return false;
        }
    }

    struct cmd {
        uint8_t a;
        uint8_t b;
    };

    Request<cmd> request;
    request.address = myAddress;
    request.data.a = 0;
    request.data.b = 0;

    if (!transceive(reinterpret_cast<uint8_t*>(std::addressof(request)),
                      sizeof(request),
                      reinterpret_cast<uint8_t*>(out_real_name),
                      myDeviceInfo.nameLength + 2)) {
        return false;
    }

    for (int i = 0; i < myDeviceInfo.nameLength; ++i) {
        out_real_name[i] = out_real_name[i+1];
    }
    out_real_name[myDeviceInfo.nameLength] = 0;

    return true;
}

} // namespace Feldbus
} // namespace TURAG

