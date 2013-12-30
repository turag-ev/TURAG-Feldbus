
#include "servo.h"
#include <tina/tina.h>

namespace TURAG {
namespace Feldbus {

bool ServoBase::enableControl() {
    if (!setValue(RS485_STELLANTRIEBE_KEY_CONTROL_TYPE, static_cast<int32_t>(myControlType)) ||
            !setValue(RS485_STELLANTRIEBE_KEY_CONTROL_STATE, static_cast<int32_t>(RS485_STELLANTRIEBE_CONTROL_STATE_ENABLE)))
        return false;

    return true;
}

bool ServoBase::disableControl() {
    if (!setValue(RS485_STELLANTRIEBE_KEY_CONTROL_STATE, static_cast<int32_t>(RS485_STELLANTRIEBE_CONTROL_STATE_DISABLE)))
        return false;

    // we set the internal control type regardless of what happens on the bus. It's better to reactivate once to often but to
    // have it disabled without knowing it.
    myControlType = RS485_STELLANTRIEBE_CONTROL_TYPE_NONE;
    return setValue(RS485_STELLANTRIEBE_KEY_CONTROL_TYPE, static_cast<int32_t>(RS485_STELLANTRIEBE_CONTROL_TYPE_NONE));
}



bool ServoBase::hasAngleReached() {
    int32_t status = RS485_STELLANTRIEBE_STATUS_NONE;

    bool success = getValue(RS485_STELLANTRIEBE_KEY_STATUS, &status);
    if (success && static_cast<uint8_t>(status) == RS485_STELLANTRIEBE_STATUS_ANGLE_REACHED)
        return true;
    else
        return false;
}

bool ServoBase::hasVelocityReached() {
    int32_t status = RS485_STELLANTRIEBE_STATUS_NONE;

    bool success = getValue(RS485_STELLANTRIEBE_KEY_STATUS, &status);

    if (success && static_cast<uint8_t>(status) == RS485_STELLANTRIEBE_STATUS_VELOCITY_REACHED)
        return true;
    else
        return false;
}

bool ServoBase::hasCurrentReached() {
    int32_t status = RS485_STELLANTRIEBE_STATUS_NONE;

    bool success = getValue(RS485_STELLANTRIEBE_KEY_STATUS, &status);

    if (success && static_cast<uint8_t>(status) == RS485_STELLANTRIEBE_STATUS_CURRENT_REACHED)
        return true;
    else
        return false;
}

bool ServoBase::hasErrorDetected() {
    int32_t status = RS485_STELLANTRIEBE_STATUS_NONE;

    bool success = getValue(RS485_STELLANTRIEBE_KEY_STATUS, &status);

    if (success && static_cast<uint8_t>(status) == RS485_STELLANTRIEBE_STATUS_ERROR)
        return true;
    else
        return false;
}

bool ServoBase::setDesiredAngle(float angle) {
    if (!setValue(RS485_STELLANTRIEBE_KEY_DESIRED_ANGLE, angle))
        return false;

    if (myControlType != RS485_STELLANTRIEBE_CONTROL_TYPE_POS) {
        if (!setValue(RS485_STELLANTRIEBE_KEY_CONTROL_TYPE, static_cast<int32_t>(RS485_STELLANTRIEBE_CONTROL_TYPE_POS)))
            return false;

        myControlType = RS485_STELLANTRIEBE_CONTROL_TYPE_POS;
    }
    return true;
}

bool ServoBase::setDesiredVelocity(float velocity) {
    if (!setValue(RS485_STELLANTRIEBE_KEY_DESIRED_VELOCITY, velocity))
        return false;

    if (myControlType != RS485_STELLANTRIEBE_CONTROL_TYPE_VEL) {
        if (!setValue(RS485_STELLANTRIEBE_KEY_CONTROL_TYPE, static_cast<int32_t>(RS485_STELLANTRIEBE_CONTROL_TYPE_VEL)))
            return false;

        myControlType = RS485_STELLANTRIEBE_CONTROL_TYPE_VEL;
    }
    return true;
}

bool ServoBase::setDesiredCurrent(float current) {
    if (!setValue(RS485_STELLANTRIEBE_KEY_DESIRED_CURRENT, current))
        return false;

    if (myControlType != RS485_STELLANTRIEBE_CONTROL_TYPE_CUR) {
        if (!setValue(RS485_STELLANTRIEBE_KEY_CONTROL_TYPE, static_cast<int32_t>(RS485_STELLANTRIEBE_CONTROL_TYPE_CUR)))
            return false;

        myControlType = RS485_STELLANTRIEBE_CONTROL_TYPE_CUR;
    }
    return true;
}

bool ServoBase::setHold(void) {
    float angle = 0;

    if (!getCurrentAngle(&angle)) return false;

    return setDesiredAngle(angle);
}

} // namespace Feldbus
} // namespace TURAG
