/**
 *  @brief		TURAG feldbus servo class
 *  @file		TURAGFeldbusServo.h
 *  @date		22.03.2013
 *  @author		Martin Oemus
 * 
 */


#ifndef TURAGFELDBUSSERVO_H_
#define TURAGFELDBUSSERVO_H_

#include "TURAGFeldbusAktor.h"

namespace TURAG {
namespace Feldbus {

/*
 *
 */
class Servo : public TURAG::Feldbus::Aktor {
protected:
	virtual inline int convertAngle(unsigned short from) const { return (int)((unsigned short)from); }
	virtual inline unsigned short convertAngle(int from) const { return (unsigned short)from; }
	virtual inline int convertVelocity(unsigned short from) const { return (int)((short)from); }
	virtual inline unsigned short convertVelocity(int from) const { return (unsigned short)from; }
	virtual inline int convertCurrent(unsigned short from) const { return (int)((short)from); }
	virtual inline unsigned short convertCurrent(int from) const { return (unsigned short)from; }
	virtual inline int convertPWM(unsigned short from) const { return (100 * (int)from + 255/2) / 255; }
	virtual inline unsigned short convertPWM(int from) const { return (unsigned short)((255 * from + 100/2) / 100); }
	virtual inline int convertTorque(unsigned short from) const {(void)from; return 0; }
	virtual inline unsigned short convertTorque(int from) const {(void)from; return 0; }
	virtual inline int convertVoltage(unsigned short from) const { return (int)((short)from); }
	virtual inline unsigned short convertVoltage(int from) const { return (unsigned short)from; }
public:
	Servo(const char* name_, int address) : Aktor(name_, address) {}
	virtual bool getTorque(int* torque) {(void)torque; return false; }
	virtual bool setTorque(int torque) {(void)torque; return false; }
	virtual bool initThresholds(unsigned int maxVelocity, unsigned int maxCurrent, unsigned int maxPWM, int minAngle, int maxAngle) {
			if (!setMaxVelocity(maxVelocity)) return false;
			if (!setMaxCurrent(maxCurrent)) return false;
			if (!setMaxPWM(maxPWM)) return false;
			if (!setMinAngle(minAngle)) return false;
			if (!setMaxAngle(maxAngle)) return false;
			return true;
		}
	virtual bool setMaxTorque(int maxTorque) {(void)maxTorque; return false; }
};



} // namespace Feldbus
} // namespace TURAG

#endif /* TURAGFELDBUSSERVO_H_ */
