/**
 *  @brief		TURAG feldbus servo class
 *  @file		servo.h
 *  @date		22.03.2013
 *  @author		Martin Oemus
 * 
 */


#ifndef TURAGFELDBUSSERVO_H_
#define TURAGFELDBUSSERVO_H_

#include "aktor.h"

namespace TURAG {
namespace Feldbus {

/*
 *
 */
class ServoBase : public TURAG::Feldbus::Aktor {
public:
    ServoBase(const char* name_, int address, ChecksumType type = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE) :
        Aktor(name_, address, type) {}

    bool getCurrentAngle(float* angle) { return getValue(RS485_STELLANTRIEBE_KEY_CURRENT_ANGLE, angle); }

    bool getDesiredAngle(float* angle) { return getValue(RS485_STELLANTRIEBE_KEY_DESIRED_ANGLE, angle); }
    virtual bool setDesiredAngle(float angle) { return setValue(RS485_STELLANTRIEBE_KEY_DESIRED_ANGLE, angle); }

    bool getMaxAngle(float* angle) { return getValue(RS485_STELLANTRIEBE_KEY_MAX_ANGLE, angle); }
    bool setMaxAngle(float angle) { return setValue(RS485_STELLANTRIEBE_KEY_MAX_ANGLE, angle); }

    bool getMinAngle(float* angle) { return getValue(RS485_STELLANTRIEBE_KEY_MIN_ANGLE, angle); }
    bool setMinAngle(float angle) { return setValue(RS485_STELLANTRIEBE_KEY_MIN_ANGLE, angle); }

    bool getCurrentVelocity(float* velocity) { return getValue(RS485_STELLANTRIEBE_KEY_CURRENT_VELOCITY, velocity); }

    bool getDesiredVelocity(float* velocity) { return getValue(RS485_STELLANTRIEBE_KEY_DESIRED_VELOCITY, velocity); }
    virtual bool setDesiredVelocity(float velocity) { return setValue(RS485_STELLANTRIEBE_KEY_DESIRED_VELOCITY, velocity); }

    bool getMaxVelocity(float* velocity) { return getValue(RS485_STELLANTRIEBE_KEY_MAX_VELOCITY, velocity); }
    bool setMaxVelocity(float velocity) { return setValue(RS485_STELLANTRIEBE_KEY_MAX_VELOCITY, velocity); }

    bool getCurrentCurrent(float* current) { return getValue(RS485_STELLANTRIEBE_KEY_CURRENT_CURRENT, current); }

    bool getDesiredCurrent(float* current) { return getValue(RS485_STELLANTRIEBE_KEY_DESIRED_CURRENT, current); }
    //virtual entfernt - 22.04.14 by Richard R.
    bool setDesiredCurrent(float current) { return setValue(RS485_STELLANTRIEBE_KEY_DESIRED_CURRENT, current); }

    bool getMaxCurrent(float* current) { return getValue(RS485_STELLANTRIEBE_KEY_MAX_CURRENT, current); }
    bool setMaxCurrent(float current) { return setValue(RS485_STELLANTRIEBE_KEY_MAX_CURRENT, current); }

    bool getCurrentPwm(float* pwm) { return getValue(RS485_STELLANTRIEBE_KEY_CURRENT_PWM, pwm); }

    bool getDesiredPwm(float* pwm) { return getValue(RS485_STELLANTRIEBE_KEY_DESIRED_PWM, pwm); }
    bool setDesiredPwm(float pwm) { return setValue(RS485_STELLANTRIEBE_KEY_DESIRED_PWM, pwm); }

    bool getMaxPwm(float* pwm) { return getValue(RS485_STELLANTRIEBE_KEY_MAX_PWM, pwm); }
    bool setMaxPwm(float pwm) { return setValue(RS485_STELLANTRIEBE_KEY_MAX_PWM, pwm); }

    bool getVoltage(float* voltage) { return getValue(RS485_STELLANTRIEBE_KEY_VOLTAGE, voltage); }

    virtual bool enableControl() {
        return setValue(RS485_STELLANTRIEBE_KEY_CONTROL_STATE, static_cast<int32_t>(RS485_STELLANTRIEBE_CONTROL_STATE_ENABLE));
    }
    virtual bool disableControl() {
        return setValue(RS485_STELLANTRIEBE_KEY_CONTROL_STATE, static_cast<int32_t>(RS485_STELLANTRIEBE_CONTROL_STATE_DISABLE));
    }

    virtual bool setHold(void);

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
};


class Servo : public ServoBase {
protected:
    Aktor::Command_t command_set[17];

public:
    Servo(const char* name_, int address, ChecksumType type = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE) :
        ServoBase(name_, address, type) {}

    bool initialize(void) { return populateCommandSet(command_set, 17); }
};

} // namespace Feldbus
} // namespace TURAG

#endif /* TURAGFELDBUSSERVO_H_ */
