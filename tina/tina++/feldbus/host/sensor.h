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
	Sensor(const char* name_, unsigned int address) : Device(name_, address) {}
};

} // namespace Feldbus
} // namespace TURAG

#endif /* TURAGFELDBUSSENSOR_H_ */
