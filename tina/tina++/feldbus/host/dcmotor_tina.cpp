
#include "dcmotor.h"

namespace TURAG {
namespace Feldbus {

bool DCMotorBase::enableControl() {
    if (!ServoBase::enableControl()) return false;

    if (myHomecomingRequested) {
        myControlType = RS485_STELLANTRIEBE_CONTROL_TYPE_POS;
        myHomecomingRequested = false;
    }

    return true;
}


bool DCMotorBase::driveHome(float velocity) {
    if (!setValue(RS485_STELLANTRIEBE_DC_KEY_RETURN_TO_HOME, velocity))
        return false;

    myControlType = RS485_STELLANTRIEBE_CONTROL_TYPE_VEL;
    myHomecomingRequested = true;

    return true;
}


} // namespace Feldbus
} // namespace TURAG
