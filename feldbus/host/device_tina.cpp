/**
 *  @brief		Base class for all TURAG feldbus devices
 *  @file		device_tina.cpp
 *  @date		04.04.2013
 *  @author		Martin Oemus
 *
 */

#define TURAG_DEBUG_LOG_SOURCE "B"

#include <tina++/tina.h>

#if TURAG_USE_TURAG_FELDBUS_HOST

#include <tina++/thread.h>
#include <tina++/time.h>
#include <tina++/crc/xor.h>
#include <tina++/crc/crc.h>
#include <tina/debug/print.h>
#include <tina/math.h>

#include "device.h"


namespace TURAG {
namespace Feldbus {

namespace {

struct DeviceInfoInternal {
    uint8_t deviceProtocolId;
    uint8_t deviceTypeId;
    uint8_t crcType;
	uint16_t bufferSize;
	uint16_t reserved;
    uint8_t nameLength;
    uint8_t versioninfoLength;
    uint16_t uptimeFrequency;
} _packed;

}

Mutex Device::listMutex;
Device* Device::firstDevice(nullptr);


Device::Device(const char* name, unsigned address, FeldbusAbstraction& feldbus, ChecksumType type,
       const AddressLength addressLength,
       unsigned int max_transmission_attempts,
       unsigned int max_transmission_errors) :
	dysFunctionalLog_(SystemTime::fromSec(5)),
	bus_(feldbus),
	myNextDevice(nullptr),
	name_(name),
	myAddress(address),
	maxTransmissionAttempts(max_transmission_attempts),
	maxTransmissionErrors(max_transmission_errors),
	myCurrentErrorCounter(0),
	myTotalTransmissions(0),
	myTotalChecksumErrors(0),
	myTotalNoAnswerErrors(0),
	myTotalMissingDataErrors(0),
	myTotalTransmitErrors(0),
	myChecksumType(type),
	myAddressLength(static_cast<uint8_t>(addressLength)),
	hasCheckedAvailabilityYet(false)
{
	{
		Mutex::Lock lock(listMutex);

		if (firstDevice == nullptr) {
			firstDevice = this;
		} else {
			myNextDevice = firstDevice;
			firstDevice = this;
		}
	}
	if (!name_) {
		name_ = "???";
	}
}

 
bool Device::transceive(uint8_t *transmit, int transmit_length, uint8_t *receive, int receive_length, bool ignoreDysfunctional)
{
	bool bailOutBecauseDysfunctional = isDysfunctional() && !ignoreDysfunctional;

	if (dysFunctionalLog_.doErrorOutput(!bailOutBecauseDysfunctional)) {
		turag_errorf("DEVICE \"%s\" DYSFUNCTIONAL. PACKAGE DROPPED (%u).", name_, dysFunctionalLog_.getErrorCount());
		dysFunctionalLog_.resetErrorCount();
	}
	if (bailOutBecauseDysfunctional) {
        return false;
	}

	// Generate target address.
	// We assume the caller wants to transmit a broadcast, if he does not supply any means to store an answer.
	// Thus we use zero rather than the device's address.
	unsigned useAddress = !receive || !receive_length ? TURAG_FELDBUS_BROADCAST_ADDR : myAddress;

	switch (myAddressLength) {
	case 1:
		transmit[0] = static_cast<uint8_t>(useAddress);
		break;

	case 2:
		transmit[0] = static_cast<uint8_t>(useAddress & 0xff);
		transmit[1] = static_cast<uint8_t>(useAddress >> 8);
		break;

	default: return false;
	}

	// Generate checksum for transmission.
	switch (myChecksumType) {
	case ChecksumType::xor_based:
		transmit[transmit_length - 1] = XOR::calculate(transmit, transmit_length - 1);
		break;

	case ChecksumType::crc8_icode:
		transmit[transmit_length - 1] = CRC8::calculate(transmit, transmit_length - 1);
		break;

	case ChecksumType::none:
		break;
	}


	FeldbusAbstraction::ResultStatus status = FeldbusAbstraction::ResultStatus::TransmissionError;
	unsigned int attempt = 0;

	// If the device is dysfunctional, we can force transmissions with
	// ignoreDysfunctional. But in this case it wouldn't make sense
	// to try more than a single transmission as the reason for a failure
	// is then most likely the device still being dysfunctional.
	unsigned maxAttempts = isDysfunctional() ? 1 : maxTransmissionAttempts;


	// we try to transmit until either
	// - the transmission succeeds and the checksum is correct or
	// - the number of transmission attempts is exceeded or
	// - the first attempt failed when the device is already dysfunctional.
	while (attempt < maxAttempts && status != FeldbusAbstraction::ResultStatus::Success) {
		int transmit_length_copy = transmit_length;
		int receive_length_copy = receive_length;

		// clear buffer from any previous failed transmissions, then send
		bus_.clearBuffer();
		status = bus_.transceive(transmit, &transmit_length_copy, receive, &receive_length_copy, useAddress, myChecksumType);


		switch (status) {
		case FeldbusAbstraction::ResultStatus::TransmissionError:
			if (transmit_length_copy < transmit_length) {
				++myTotalTransmitErrors;
			} else if (receive_length_copy == 0) {
				++myTotalNoAnswerErrors;
			} else {
				++myTotalMissingDataErrors;
			}
			break;

		case FeldbusAbstraction::ResultStatus::ChecksumError:
			++myTotalChecksumErrors;
			break;

        case FeldbusAbstraction::ResultStatus::Success:
            break;
		}

		++attempt;
	}
	myTotalTransmissions += attempt;


    switch (status)
    {
    case FeldbusAbstraction::ResultStatus::Success:
		// if we had a successful transmission, we reset the error counter
		// but resetting the error only makes sense if we got a response from the device
		if (receive && receive_length != 0) {
			myCurrentErrorCounter = 0;

			// Because resetting the error counter essentially
			// means setting the device to be functional again
			// we also have to reset the dysFunctionalLog_ to get
			// an error message if the device fails again within
			// a short time.
			dysFunctionalLog_.resetAll();
        }
        turag_debugf("%s: rs485 transceive successful!", name_);
        return true;
    case FeldbusAbstraction::ResultStatus::TransmissionError:
        turag_warningf("%s: rs485 transceive failed: Transmission error", name_);
        myCurrentErrorCounter += attempt;
        return false;
    case FeldbusAbstraction::ResultStatus::ChecksumError:
        turag_warningf("%s: rs485 transceive failed: Checksum error", name_);
        myCurrentErrorCounter += attempt;
        return false;
    default:
        // C++ specifies that the range of enums (and enum classes) is the same
        // as the range of the underlying type. So without this default case
        // the compiler generates a warning.
        // Anyway, if this case occurs, something is really fucked up.
        turag_errorf("%s: rs485 transceive failed: Unknown error", name_);
        myCurrentErrorCounter += attempt;
        return false;
    }
}

bool Device::sendPing(void) {
	Request<> request;
	Response<> response;

	return transceive(request, &response, true);
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
	if (!myDeviceInfo.isValid()) {
        Request<uint8_t> request;
        Response<DeviceInfoInternal> response;

        request.data = 0;

        if (!transceive(request, &response)) {
            return false;
        }
		myDeviceInfo.deviceProtocolId_ = response.data.deviceProtocolId;
		myDeviceInfo.deviceTypeId_ = response.data.deviceTypeId;
		myDeviceInfo.crcType_ = response.data.crcType;
		myDeviceInfo.bufferSize_ = response.data.bufferSize;
		myDeviceInfo.nameLength_ = response.data.nameLength;
		myDeviceInfo.versioninfoLength_ = response.data.versioninfoLength;
		myDeviceInfo.uptimeFrequency_ = response.data.uptimeFrequency;
    }
    if (device_info) *device_info = myDeviceInfo;
    return true;
}

bool Device::receiveDeviceRealName(char* out_string) {
    // It is possible that nobody called getDeviceInfo() yet,
    // so our device info would be empty. Because the device
    // info contains the length of the string we are about 
	// to receive, we make sure getDeviceInfo() gets called.
	if (!myDeviceInfo.isValid()) {
		if (!getDeviceInfo(nullptr)) {
			return false;
		}
	}

    return receiveString(
		TURAG_FELDBUS_SLAVE_COMMAND_DEVICE_NAME, 
		myDeviceInfo.nameLength(),
		out_string);
}

bool Device::receiveVersionInfo(char* out_string) {
    // It is possible that nobody called getDeviceInfo() yet,
    // so our device info would be empty. Because the device
    // info contains the length of the string we are about 
	// to receive, we make sure getDeviceInfo() gets called.
	if (!myDeviceInfo.isValid()) {
		if (!getDeviceInfo(nullptr)) {
			return false;
		}
	}

    return receiveString(
		TURAG_FELDBUS_SLAVE_COMMAND_VERSIONINFO, 
		myDeviceInfo.versionInfoLength(),
		out_string);
}

bool Device::receiveString(uint8_t command, uint8_t stringLength, char* out_string) {
    if (!out_string) {
		return false;
    }
    
    // because we receive a packet including address and checksum
    // we need a slightly bigger array.
	char recvBuffer[stringLength + myAddressLength + 1];

    struct cmd {
        uint8_t a;
        uint8_t b;
    };

    const int request_len = myAddressLength + 2 + 1;
	uint8_t request[2 + 2 + 1];
	request[myAddressLength] = 0;
	request[myAddressLength + 1] = command;
	
    if (!transceive(request,
                      request_len,
                      reinterpret_cast<uint8_t*>(recvBuffer),
                      stringLength + myAddressLength + 1)) {
        return false;
    }

    for (int i = 0; i < stringLength; ++i) {
        out_string[i] = recvBuffer[i + myAddressLength];
    }
    out_string[stringLength] = 0;

    return true;	
}

bool Device::receiveUptime(float* uptime) {
	if (!uptime) {
		return false;
	}
	
	if (!myDeviceInfo.isValid()) {
		if (!getDeviceInfo(nullptr)) {
			return false;
		}
	}

	if (myDeviceInfo.uptimeFrequency() == 0) {
        *uptime = NAN;
        return true;
	}
	
	uint32_t count = 0;
	
	if (!receiveErrorCount(TURAG_FELDBUS_SLAVE_COMMAND_UPTIME_COUNTER, &count)) {
		return false;
	}

	*uptime = static_cast<float>(count) / static_cast<float>(myDeviceInfo.uptimeFrequency());
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

#endif // TURAG_USE_TURAG_FELDBUS_HOST
