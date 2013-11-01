/**
 *  @brief		Base class for all TURAG feldbus devices
 *  @file		TURAGFeldbusDevice.h
 *  @date		22.03.2013
 *  @author		Martin Oemus
 * 
 */


#ifndef TURAGFELDBUSDEVICE_H_
#define TURAGFELDBUSDEVICE_H_

#include <tina++/tina.h>
#include <tina++/thread.h>

namespace TURAG {
namespace Feldbus {

/*
 *
 */
class Device {
private:
	unsigned int myTransmissionErrorCounter;
	static unsigned int globalTransmissionErrorCounter;
	const unsigned int maxTransmissionAttempts;
	const unsigned int maxTransmissionErrors;
  static Mutex rs485_mutex;
  static SystemTime lastTransmission;

protected:
	bool hasCheckedAvailabilityYet;
	const unsigned int myAddress;

	Device(const char* name_, unsigned int address) :
		myTransmissionErrorCounter(0),
		maxTransmissionAttempts(5),
		maxTransmissionErrors(35),
		hasCheckedAvailabilityYet(false),
		myAddress(address),
		name(name_) { }
	virtual ~Device() {}

	bool transceive(uint8_t *input, int input_length, uint8_t *output, int output_length);

	bool hasReachedTransmissionErrorLimit(void) const { if (myTransmissionErrorCounter > maxTransmissionErrors) return true; else return false; }
	void resetTransmissionErrorLimit(void) { myTransmissionErrorCounter = 0; }

public:
	unsigned int getAddress(void) const { return myAddress; }
	virtual bool isAvailable(void) = 0;
	const char* name;
};

} // namespace Feldbus
} // namespace TURAG

#endif /* TURAGFELDBUSDEVICE_H_ */
