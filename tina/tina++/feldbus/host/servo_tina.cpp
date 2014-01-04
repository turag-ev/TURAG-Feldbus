
#include "servo.h"
#include <tina/tina.h>

namespace TURAG {
namespace Feldbus {


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


bool ServoBase::setHold(void) {
    float angle = 0;

    if (!getCurrentAngle(&angle)) return false;

    return setDesiredAngle(angle);
}

} // namespace Feldbus
} // namespace TURAG
