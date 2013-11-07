/**
 *  @brief		TURAG feldbus actor base class
 *  @file		aktor.h
 *  @date		22.03.2013
 *  @author		Martin Oemus
 *
 */


#ifndef TURAGFELDBUSAKTOR_H_
#define TURAGFELDBUSAKTOR_H_

#include "device.h"
#include <tina++/tina.h>
#include <tina/feldbus/protocol/turag_feldbus_fuer_stellantriebe.h>

namespace TURAG {
namespace Feldbus {

/*
 *
 */
class Aktor : public TURAG::Feldbus::Device {
private:
	unsigned int myControlType;
	bool myHomecomingRequested;
	unsigned int myMaxVelocity;
	unsigned int myMaxCurrent;
	unsigned int myMaxPWM;
	unsigned int myMaxTorque;
	int myMaxAngle;
	int myMinAngle;

protected:
	Aktor(const char* name_, unsigned int address) :
				Device(name_, address),
				myControlType(RS485_MOTOR_CONTROL_TYPE_NONE),
				myHomecomingRequested(0),
				myMaxVelocity(0xffffffff),
				myMaxCurrent(0xffffffff),
				myMaxPWM(0xffffffff),
				myMaxTorque(0xffffffff),
				myMaxAngle(0x7fffffff),
				myMinAngle(0x80000000)
	{ }

	virtual ~Aktor() {}

	bool getValue(uint8_t key, uint16_t* value);
	bool setValue(uint8_t key, uint16_t value);

	virtual int convertAngle(unsigned short from) const = 0;
	virtual unsigned short convertAngle(int from) const = 0;
	virtual int convertVelocity(unsigned short from) const = 0;
	virtual unsigned short convertVelocity(int from) const = 0;
	virtual int convertCurrent(unsigned short from) const = 0;
	virtual unsigned short convertCurrent(int from) const = 0;
	virtual int convertPWM(unsigned short from) const = 0;
	virtual unsigned short convertPWM(int from) const = 0;
	virtual int convertTorque(unsigned short from) const = 0;
	virtual unsigned short convertTorque(int from) const = 0;
	virtual int convertVoltage(unsigned short from) const = 0;
	virtual unsigned short convertVoltage(int from) const = 0;

public:
	virtual bool isAvailable(void);

	/** Status request
	 * @return		True if angle reached, otherwise false.
	 */
	virtual bool hasAngleReached();

	/** Status request
	 * @return		True if velocity reached, otherwise false.
	 */
	virtual bool hasVelocityReached();

	/** Status request
	 * @return		True if current reached, otherwise false.
	 */
	virtual bool hasCurrentReached();

	/** Status request
	 * @return		True if error detected, otherwise false.
	 */
	virtual bool hasErrorDetected();

	virtual bool enableControl();
	virtual bool disableControl();
	virtual bool driveHome(int velocity);

	virtual bool getAngle(int* angle);
	virtual bool getVelocity(int* velocity);
	virtual bool getCurrent(int* current);
	virtual bool getTorque(int* torque);
	virtual bool getDirection(int* direction);
	virtual bool getVoltage(int* voltage);
	virtual bool getPWM(int* pwm);

	virtual bool setAngle(int angle);
	virtual bool setVelocity(int velocity);
	virtual bool setCurrent(int current);
	virtual bool setTorque(int torque);

	virtual bool setMaxVelocity(int maxVelocity);
	virtual bool setMaxCurrent(int maxCurrent);
	virtual bool setMaxPWM(int maxPWM);
	virtual bool setMaxTorque(int maxTorque);
	virtual bool setMinAngle(int angle);
	virtual bool setMaxAngle(int angle);

	virtual int getMaxVelocity() const 	{ return myMaxVelocity; }
	virtual int getMaxCurrent() const	{ return myMaxCurrent; }
	virtual int getMaxPWM() const 		{ return myMaxPWM; }
	virtual int getMaxTorque() const 	{ return myMaxTorque; }
	virtual int getMinAngle() const 	{ return myMinAngle; }
	virtual int getMaxAngle() const 	{ return myMaxAngle; }

	virtual bool setHold(void);

};

} // namespace Feldbus
} // namespace TURAG

#endif /* TURAGFELDBUSAKTOR_H_ */
