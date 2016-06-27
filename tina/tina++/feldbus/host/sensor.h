/**
 *  @brief		TURAG feldbus sensor base class
 *  @file		sensor.h
 *  @date		22.03.2013
 *  @author		Martin Oemus
 * 
 */


#ifndef TURAGFELDBUSSENSOR_H_
#define TURAGFELDBUSSENSOR_H_

#include "device.h"

namespace TURAG {
namespace Feldbus {

/*
 *
 */
class Sensor : public TURAG::Feldbus::Device {
public:
    Sensor(const char* name_, unsigned int address, FeldbusAbstraction* feldbus, ChecksumType type = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE,
		const AddressLength addressLength = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_ADDRESS_LENGTH) : 
        Device(name_, address, feldbus, type, addressLength) {}
};

} // namespace Feldbus
} // namespace TURAG

#endif /* TURAGFELDBUSSENSOR_H_ */
