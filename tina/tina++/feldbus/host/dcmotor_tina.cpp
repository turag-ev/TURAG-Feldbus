
#include <tina++/tina.h>
#include <tina/debug.h>
#if TURAG_USE_TURAG_FELDBUS_HOST

#include "dcmotor.h"

namespace TURAG {
namespace Feldbus {

bool DCMotor::hasAngleReached() {
    int32_t status = RS485_STELLANTRIEBE_STATUS_NONE;
    bool success = getValue(RS485_STELLANTRIEBE_KEY_STATUS, &status);

	if (success && static_cast<uint8_t>(status) == RS485_STELLANTRIEBE_STATUS_ANGLE_REACHED) {
        turag_debugf("%s hasAngleReached = true", this->name());
        return true;
	} else {
        turag_debugf("%s hasAngleReached = false", this->name());
        return false;
	}
}

bool DCMotor::hasVelocityReached() {
    int32_t status = RS485_STELLANTRIEBE_STATUS_NONE;

    bool success = getValue(RS485_STELLANTRIEBE_KEY_STATUS, &status);

    if (success && static_cast<uint8_t>(status) == RS485_STELLANTRIEBE_STATUS_VELOCITY_REACHED)
        return true;
    else
        return false;
}

bool DCMotor::hasCurrentReached() {
    int32_t status = RS485_STELLANTRIEBE_STATUS_NONE;

    bool success = getValue(RS485_STELLANTRIEBE_KEY_STATUS, &status);

    if (success && static_cast<uint8_t>(status) == RS485_STELLANTRIEBE_STATUS_CURRENT_REACHED)
        return true;
    else
        return false;
}

bool DCMotor::hasErrorDetected() {
    int32_t status = RS485_STELLANTRIEBE_STATUS_NONE;

    bool success = getValue(RS485_STELLANTRIEBE_KEY_STATUS, &status);

    if (success && static_cast<uint8_t>(status) == RS485_STELLANTRIEBE_STATUS_ERROR)
        return true;
    else
        return false;
}


bool DCMotor::setHold(void) {
    float angle = 0;

    if (!getCurrentAngle(&angle)) return false;

    return setDesiredAngle(angle);
}



} // namespace Feldbus
} // namespace TURAG

#endif // TURAG_USE_TURAG_FELDBUS_HOST
