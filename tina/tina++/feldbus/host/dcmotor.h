/**
 *  @brief		TURAG feldbus DC motor class
 *  @file		dcmotor.h
 *  @date		22.03.2013
 *  @author		Martin Oemus
 * 
 */


#ifndef TURAGFELDBUSDCMOTOR_H_
#define TURAGFELDBUSDCMOTOR_H_

#include "aktor.h"

namespace TURAG {
namespace Feldbus {

/*
 *
 */
class DCMotor : public TURAG::Feldbus::Aktor {
protected:
	virtual inline int convertAngle(unsigned short from) const { return (int)10 * (short)from; }
	virtual inline unsigned short convertAngle(int from) const { return (unsigned short)((from + 5) / 10); }
	virtual inline int convertVelocity(unsigned short from) const { return (int)2 * (short)from; }
	virtual inline unsigned short convertVelocity(int from) const { return (unsigned short)((from + 1) / 2); }
	virtual inline int convertCurrent(unsigned short from) const { return (int)((short)from); }
	virtual inline unsigned short convertCurrent(int from) const { return (unsigned short)from; }
	virtual inline int convertPWM(unsigned short from) const { return (100 * (int)from + 391/2) / 391; }
	virtual inline unsigned short convertPWM(int from) const { return (unsigned short)((391 * from + 100/2) / 100); }
	virtual inline int convertTorque(unsigned short from) const { return (int)((short)from); }
	virtual inline unsigned short convertTorque(int from) const { return (unsigned short)from; }
	virtual inline int convertVoltage(unsigned short from) const { return (int)((short)from); }
	virtual inline unsigned short convertVoltage(int from) const { return (unsigned short)from; }

public:

	DCMotor(const char* name_, int address) : Aktor(name_, address) {}
	virtual bool setMinAngle(int angle) { (void)angle; return false; }
	virtual bool setMaxAngle(int angle) { (void)angle; return false; }
	virtual bool initThresholds(unsigned int maxVelocity, unsigned int maxCurrent, unsigned int maxPWM, unsigned int maxTorque) {
			if (!setMaxVelocity(maxVelocity)) return false;
			if (!setMaxCurrent(maxCurrent)) return false;
			if (!setMaxPWM(maxPWM)) return false;
			if (!setMaxTorque(maxTorque)) return false;
			return true;
		}
	bool getSwitchStatus(uint16_t* status) { return aktorGet(RS485_MOTOR_COM_INDEX_SWITCH_STATUS, status); }
};


} // namespace Feldbus
} // namespace TURAG

#endif /* TURAGFELDBUSDCMOTOR_H_ */


