/**
 *  @brief		Base class for all TURAG feldbus devices
 *  @file		device_tina.cpp
 *  @date		04.04.2013
 *  @author		Martin Oemus
 *
 */

#define TURAG_DEBUG_LOG_SOURCE "B"

#include <tina++/thread.h>
#include <tina++/time.h>
#include <tina++/crc/xor.h>
#include <tina++/crc/crc8.h>
#include <tina/debug/print.h>
#include <tina/feldbus/host/rs485.h>

#include "device.h"

namespace TURAG {
namespace Feldbus {

 std::atomic_int Device::globalTransmissionErrorCounter{0};
 
	
bool Device::transceive(uint8_t *transmit, int transmit_length, uint8_t *receive, int receive_length) {
    if (isDysfunctional()) {
        static unsigned dysfunctionalMessageDisplayed = 0;
        if (dysfunctionalMessageDisplayed < 5) {
            turag_errorf("DEVICE \"%s\" DYSFUNCTIONAL. PACKAGE DROPPED.", name);
            dysfunctionalMessageDisplayed++;
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
		
		int transmit_length_backup = transmit_length;

        // we try to transmit until the transmission succeeds and the checksum is correct
        // or the number of transmission attempts is exceeded.
        while (attempt < maxTransmissionAttempts && !(success && checksum_correct)) {
            success = turag_rs485_transceive(transmit, &transmit_length, receive, &receive_length);

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
                    
                    if (!checksum_correct) {
						++myTotalChecksumErrors;
						
						++globalTransmissionErrorCounter;
						if (!(globalTransmissionErrorCounter.load() % 25)) {
							turag_criticalf("%d failed transmissions on the bus so far.", globalTransmissionErrorCounter.load());
						}
                    }
                }
            } else {
				if (transmit_length < transmit_length_backup) {
					++myTotalTransmitErrors;
				} else if (receive_length == 0) {
					++myTotalNoAnswerErrors;
				} else {
					++myTotalMissingDataErrors;
				}
				
                ++globalTransmissionErrorCounter;
                if (!(globalTransmissionErrorCounter.load() % 25)) {
                    turag_criticalf("%d failed transmissions on the bus so far.", globalTransmissionErrorCounter.load());
                }
            }
            ++attempt;
        }

        // 	turag_infof("%s: transceive rx success(%x|%x) [", name, success, checksum_correct);
        // 	for (int j = 0; j < receive_length; ++j) {
        // 		turag_infof("%.2x", receive[j]);
        // 	}
        // 	info("]\n");

        myTotalTransmissions += attempt;

        if (success && checksum_correct) {
            // if we had a successful transmission, we reset the error counter
            // but resetting the error only makes sense if we got a response from the device
            if (receive && receive_length != 0) {
                myCurrentErrorCounter = 0;
            }
            return true;
        } else {
            turag_warningf("%s: rs485 transceive failed", name);
            myCurrentErrorCounter += attempt;
            return false;
        }
    }
}

bool Device::sendPing(void) {
	Request<> request;
	Response<> response;

	return transceive(request, &response);
}

bool Device::isAvailable(bool forceUpdate) {
	hasCheckedAvailabilityYet = true;
	
	if (!hasCheckedAvailabilityYet || forceUpdate) {
		while (!isDysfunctional()) {
			if (sendPing()) {
				break;
			}
		}
	}

	return !isDysfunctional();
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

bool Device::receiveDeviceRealName(char* out_string) {
    // It is possible that nobody called getDeviceInfo() yet,
    // so our device info would be empty. Because the device
    // info contains the length of the string we are about 
    // to receive, we make sure getDeviceInfo() get called.
    if (myDeviceInfo.bufferSize == 0) {
        if (!getDeviceInfo(nullptr)) {
            return false;
        }
    }
    
    return receiveString(
		TURAG_FELDBUS_SLAVE_COMMAND_DEVICE_NAME, 
		myDeviceInfo.nameLength,
		out_string);
}

bool Device::receiveVersionInfo(char* out_string) {
    // It is possible that nobody called getDeviceInfo() yet,
    // so our device info would be empty. Because the device
    // info contains the length of the string we are about 
    // to receive, we make sure getDeviceInfo() get called.
    if (myDeviceInfo.bufferSize == 0) {
        if (!getDeviceInfo(nullptr)) {
            return false;
        }
    }
    
    return receiveString(
		TURAG_FELDBUS_SLAVE_COMMAND_VERSIONINFO, 
		myDeviceInfo.versioninfoLength,
		out_string);
}

bool Device::receiveString(uint8_t command, uint8_t stringLength, char* out_string) {
    if (!out_string) {
		return false;
    }
    
    // because we receive a packet including address and checksum
    // we need a slightly bigger array.
    char recvBuffer[stringLength + sizeof(FeldbusAddressType) + 1];

    struct cmd {
        uint8_t a;
        uint8_t b;
    };

    Request<cmd> request;
    request.address = myAddress;
    request.data.a = 0;
    request.data.b = command;

    if (!transceive(reinterpret_cast<uint8_t*>(std::addressof(request)),
                      sizeof(request),
                      reinterpret_cast<uint8_t*>(recvBuffer),
                      stringLength + sizeof(FeldbusAddressType) + 1)) {
        return false;
    }

    for (int i = 0; i < stringLength; ++i) {
        out_string[i] = recvBuffer[i + sizeof(FeldbusAddressType)];
    }
    out_string[stringLength] = 0;

    return true;	
}

} // namespace Feldbus
} // namespace TURAG

