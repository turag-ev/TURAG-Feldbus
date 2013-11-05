/**
 *  @brief		Base class for all TURAG feldbus devices
 *  @file		device.h
 *  @date		22.03.2013
 *  @author		Martin Oemus
 * 
 */


#ifndef TURAGFELDBUSDEVICE_H_
#define TURAGFELDBUSDEVICE_H_

#include <tina++/tina.h>
#include <tina++/thread.h>
#include <tina/feldbus/protocol/turag_feldbus_bus_protokoll.h>


// -------------------------------------------------------------
// - config
// -------------------------------------------------------------
// these values could be relocated to an external header to make
// the whole thing configurable.

// number of trials before we let the transmission fail
#define TURAG_FELDBUS_DEVICE_CONFIG_MAX_TRANSMISSION_ATTEMPTS		5

// specifies how many unsuccessful transmission attempts in a row are accepted
// before defining the device as dysfunctional
#define TURAG_FELDBUS_DEVICE_CONFIG_MAX_TRANSMISSION_ERRORS			35

#define TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE			TURAG::Feldbus::Device::ChecksumType::xor_based



namespace TURAG {
namespace Feldbus {

/*
 *
 */
class Device {
public:
	enum class ChecksumType {
		xor_based = TURAG_FELDBUS_CHECKSUM_XOR,
		crc8_icode = TURAG_FELDBUS_CHECKSUM_CRC8_ICODE,
	};

private:
	const unsigned int maxTransmissionAttempts;
	const unsigned int maxTransmissionErrors;

	ChecksumType myChecksumType;
	unsigned int myTransmissionErrorCounter;

	static unsigned int globalTransmissionErrorCounter;
	static Mutex rs485_mutex;
	static SystemTime lastTransmission;

protected:
	const unsigned int myAddress;
	bool hasCheckedAvailabilityYet;

	Device(const char* name_, unsigned int address) :
		maxTransmissionAttempts(TURAG_FELDBUS_DEVICE_CONFIG_MAX_TRANSMISSION_ATTEMPTS),
		maxTransmissionErrors(TURAG_FELDBUS_DEVICE_CONFIG_MAX_TRANSMISSION_ERRORS),
		myChecksumType(TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE),
		myTransmissionErrorCounter(0),
		myAddress(address),
		hasCheckedAvailabilityYet(false),
		name(name_) { }
	Device(const char* name_, unsigned int address, ChecksumType type) :
		maxTransmissionAttempts(TURAG_FELDBUS_DEVICE_CONFIG_MAX_TRANSMISSION_ATTEMPTS),
		maxTransmissionErrors(TURAG_FELDBUS_DEVICE_CONFIG_MAX_TRANSMISSION_ERRORS),
		myChecksumType(type),
		myTransmissionErrorCounter(0),
		myAddress(address),
		hasCheckedAvailabilityYet(false),
		name(name_) { }
	virtual ~Device() {}

	// this funtion will always overwrite the last byte with the checksum of the preceeding ones
	// so take care of the right buffer size!
	bool transceive(uint8_t *transmit, int transmit_length, uint8_t *receive, int receive_length);

	bool hasReachedTransmissionErrorLimit(void) const { if (myTransmissionErrorCounter > maxTransmissionErrors) return true; else return false; }
	void clearTransmissionErrors(void) { myTransmissionErrorCounter = 0; }

public:
	const char* name;

	unsigned int getAddress(void) const { return myAddress; }
	virtual bool isAvailable(void);
};

} // namespace Feldbus
} // namespace TURAG

#endif /* TURAGFELDBUSDEVICE_H_ */
