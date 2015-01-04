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
#include <tina/math.h>

#include "device.h"

namespace TURAG {
namespace Feldbus {

namespace {

struct DeviceInfoInternal {
    uint8_t deviceProtocolId;
    uint8_t deviceTypeId;
    uint8_t crcType;
    uint8_t bufferSize;
    uint8_t nameLength;
    uint8_t versioninfoLength;
    uint16_t uptimeFrequency;
} _packed;

}

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
		

        // we try to transmit until the transmission succeeds and the checksum is correct
        // or the number of transmission attempts is exceeded.
        while (attempt < maxTransmissionAttempts && !(success && checksum_correct)) {
            int transmit_length_copy = transmit_length;
            int receive_length_copy = receive_length;

			turag_rs485_buffer_clear();
            success = turag_rs485_transceive(transmit, &transmit_length_copy, receive, &receive_length_copy);

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
                if (transmit_length_copy < transmit_length) {
					++myTotalTransmitErrors;
                } else if (receive_length_copy == 0) {
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
		if (!hasCheckedAvailabilityYet || forceUpdate) {
		while (!isDysfunctional()) {
			if (sendPing()) {
				break;
			}
		}
	}

    hasCheckedAvailabilityYet = true;
    return !isDysfunctional();
}

bool Device::getDeviceInfo(DeviceInfo* device_info) {
    if (myDeviceInfo.bufferSize == 0) {
        Request<uint8_t> request;
        Response<DeviceInfoInternal> response;

        request.data = 0;

        if (!transceive(request, &response)) {
            return false;
        }
        myDeviceInfo.deviceProtocolId = response.data.deviceProtocolId;
        myDeviceInfo.deviceTypeId = response.data.deviceTypeId;
        myDeviceInfo.crcType = response.data.crcType & 0x7f;
        myDeviceInfo.bufferSize = response.data.bufferSize;
        myDeviceInfo.nameLength = response.data.nameLength;
        myDeviceInfo.versioninfoLength = response.data.versioninfoLength;
        myDeviceInfo.uptimeFrequency = response.data.uptimeFrequency;
        myDeviceInfo.packageStatisticsAvailable = response.data.crcType & 0x80 ? true : false;
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

bool Device::receiveUptime(float* uptime) {
	if (!uptime) {
		return false;
	}
	
	if (!getDeviceInfo(nullptr)) {
		return false;
	}
	
	if (myDeviceInfo.uptimeFrequency == 0) {
        *uptime = NAN;
        return true;
	}
	
	uint32_t count = 0;
	
	if (!receiveErrorCount(TURAG_FELDBUS_SLAVE_COMMAND_UPTIME_COUNTER, &count)) {
		return false;
	}

    *uptime = (float)count / (float)myDeviceInfo.uptimeFrequency;
    return true;
}

bool Device::receiveNumberOfAcceptedPackages(uint32_t* packageCount) {
	return receiveErrorCount(TURAG_FELDBUS_SLAVE_COMMAND_PACKAGE_COUNT_CORRECT, packageCount);
}

bool Device::receiveNumberOfOverflows(uint32_t* overflowCount) {
	return receiveErrorCount(TURAG_FELDBUS_SLAVE_COMMAND_PACKAGE_COUNT_BUFFEROVERFLOW, overflowCount);
}

bool Device::receiveNumberOfLostPackages(uint32_t* lostPackagesCount) {
	return receiveErrorCount(TURAG_FELDBUS_SLAVE_COMMAND_PACKAGE_COUNT_LOST, lostPackagesCount);
}

bool Device::receiveNumberOfChecksumErrors(uint32_t* checksumErrorCount) {
	return receiveErrorCount(TURAG_FELDBUS_SLAVE_COMMAND_PACKAGE_COUNT_CHKSUM_MISMATCH, checksumErrorCount);
}

bool Device::receiveErrorCount(uint8_t command, uint32_t* buffer) {
	if (!buffer) {
		return false;
	}
	
	struct cmd {
		uint8_t a;
		uint8_t b;
	};

	Request<cmd> request;
	request.data.a = 0;
	request.data.b = command;
	
	// this seems to be required for proper alignment
	struct Value {
		uint32_t value;
	} _packed;

	Response<Value> response;
	
	if (!transceive(request, &response)) {
		return false;
	}
	
	*buffer = response.data.value;
	
	return true;
}

bool Device::receiveAllSlaveErrorCount(uint32_t* counts) {
	if (!counts) {
		return false;
	}
	
	struct cmd {
		uint8_t a;
		uint8_t b;
	};

	Request<cmd> request;
	request.data.a = 0;
	request.data.b = TURAG_FELDBUS_SLAVE_COMMAND_PACKAGE_COUNT_ALL;
	
	struct Value {
		uint32_t packageCount;
		uint32_t overflowCount;
		uint32_t lostPackagesCount;
		uint32_t checksumErrorCount;
	} _packed;

	Response<Value> response;
	
	if (!transceive(request, &response)) {
		return false;
	}

	counts[0] = response.data.packageCount;
	counts[1] = response.data.overflowCount;
	counts[2] = response.data.lostPackagesCount;
	counts[3] = response.data.checksumErrorCount;
	
	return true;
}

bool Device::resetSlaveErrors(void) {
	struct cmd {
		uint8_t a;
		uint8_t b;
	};

	Request<cmd> request;
	request.data.a = 0;
	request.data.b = TURAG_FELDBUS_SLAVE_COMMAND_RESET_PACKAGE_COUNT;

	Response<> response;
	
	return transceive(request, &response);
}

} // namespace Feldbus
} // namespace TURAG

