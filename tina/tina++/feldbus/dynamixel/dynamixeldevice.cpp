#define LOG_SOURCE "B"

#include <tina++/debug.h>
#include "dynamixeldevice.h"

namespace TURAG {
namespace Feldbus {


//Servo verfügbar?
bool DynamixelDevice::isAvailable(void) {
    if (!hasCheckedAvailabilityYet) {
        while (!hasReachedTransmissionErrorLimit()) {
            if (turag_dxl_ping(myId)) {
                hasCheckedAvailabilityYet = true;
                return true;
            } else {
                ++myTransmissionErrorCounter;
            }
        }
    }

    return !hasReachedTransmissionErrorLimit();
}
//Protected (an sich soweit fertig)
bool DynamixelDevice::readWord(int address, int* word) {
    if (!isAvailable()) {
        return false;
    }

    if (!word) {
        return false;
    }

    bool success = false;
    unsigned attempt = 0;

    while (success == false && attempt < maxTransmissionAttempts) {
        success = turag_dxl_read_word(myId, address, word);
    }

    if (success) {
        myTransmissionErrorCounter = 0;
        printLastDeviceError();
        return true;
    } else {
        warningf("%s: rs485 transceive failed", name);
        myTransmissionErrorCounter += attempt;
        return false;
    }
}

bool DynamixelDevice::readByte(int address, int* byte) {
    if (!isAvailable()) {
        return false;
    }

    if (!byte) {
        return false;
    }

    bool success = false;
    unsigned attempt = 0;

    while (success == false && attempt < maxTransmissionAttempts) {
        success = turag_dxl_read_byte(myId, address, byte);
    }

    if (success) {
        myTransmissionErrorCounter = 0;
        printLastDeviceError();
        return true;
    } else {
        warningf("%s: rs485 transceive failed", name);
        myTransmissionErrorCounter += attempt;
        return false;
    }
}

bool DynamixelDevice::writeWord(int address, int word) {
    if (!isAvailable()) {
        return false;
    }

    bool success = false;
    unsigned attempt = 0;

    while (success == false && attempt < maxTransmissionAttempts) {
        success = turag_dxl_write_word(myId, address, word);
    }

    if (success) {
        myTransmissionErrorCounter = 0;
        printLastDeviceError();
        return true;
    } else {
        warningf("%s: rs485 transceive failed", name);
        myTransmissionErrorCounter += attempt;
        return false;
    }
}

bool DynamixelDevice::writeByte(int address, int byte) {
    if (!isAvailable()) {
        return false;
    }

    bool success = false;
    unsigned attempt = 0;

    while (success == false && attempt < maxTransmissionAttempts) {
        success = turag_dxl_write_byte(myId, address, byte);
    }

    if (success) {
        myTransmissionErrorCounter = 0;
        printLastDeviceError();
        return true;
    } else {
        warningf("%s: rs485 transceive failed", name);
        myTransmissionErrorCounter += attempt;
        return false;
    }
}

bool DynamixelDevice::hasDeviceError(DynamixelDevice::Error index) {
    return (bool)turag_dxl_get_rxpacket_error((int)index);
}

void DynamixelDevice::printLastDeviceError() {
    if (hasDeviceError(Error::voltage))
        criticalf("%s: Input voltage error!", name);

    if (hasDeviceError(Error::angle))
        criticalf("%s: Angle limit error!", name);

    if (hasDeviceError(Error::overheat))
        criticalf("%s: Overheat errorn", name);

    if (hasDeviceError(Error::range))
        criticalf("%s: Out of range error!", name);

    if (hasDeviceError(Error::checksum))
        criticalf("%s: Checksum error!", name);

    if (hasDeviceError(Error::overload))
        criticalf("%s: Overload error!", name);

    if (hasDeviceError(Error::instruction))
        criticalf("%s: Instruction code error!", name);
}

//##############################################################################################################

//Modellnummer
bool DynamixelDevice::getModelNumber(int* modelNumber) {
    if (modelNumber_ == -1) {
        int tempModelNumber = 0;
        if (readWord(TURAG_DXL_ADDRESS_MODEL_NUMBER, &tempModelNumber)) {
            modelNumber_ = tempModelNumber;
        } else {
            return false;
        }
    }

    if (modelNumber) {
        *modelNumber =  modelNumber_;
    }
    return true;
}

//Version der Firmware
/*Beispiel für Verwendung:
 *int out = 0;
 *device->getFirmwareVersion(&out);
 *printf("%d", out);
*/
bool DynamixelDevice::getFirmwareVersion(int* version) {
    if (firmwareVersion_ == -1) {
        int tempVersion = 0;
        if (readWord(TURAG_DXL_ADDRESS_FIRMWARE, &tempVersion)) {
            firmwareVersion_ = tempVersion;
        } else {
            return false;
        }
    }

    if (version) {
        *version =  firmwareVersion_;
    }
    return true;
}

//getTemperature
bool DynamixelDevice::getTemperature(int* temperature) {
    return readByte(TURAG_DXL_ADDRESS_PRESENT_TEMPERATURE, temperature);
}

//setLed
bool DynamixelDevice::setLed(bool on) {
    if (on) {
        return writeByte(TURAG_DXL_ADDRESS_LED, 1);
    } else {
        return writeByte(TURAG_DXL_ADDRESS_LED, 0);
    }
}


bool DynamixelDevice::getCurrentPosition(int* position) {
    return readWord(TURAG_DXL_ADDRESS_PRESENT_POSITION, position);
}
bool DynamixelDevice::setGoalPosition(int position) {
    return writeWord(TURAG_DXL_ADDRESS_GOAL_POSITION, position);
}

//TorqueEnable
bool DynamixelDevice::getTorqueEnable(bool* enabled) {
    return readWord(TURAG_DXL_ADDRESS_TORQUE_ENABLE, (int*)enabled);
}

bool DynamixelDevice::setTorqueEnable(bool enable) {
    return writeWord(TURAG_DXL_ADDRESS_TORQUE_ENABLE, (int)enable);
}

//Ccw AngleLimit
bool DynamixelDevice::getCcwAngleLimit(int* limit) {
    return readWord(TURAG_DXL_ADDRESS_CCW_ANGLE_LIMIT, limit);
}

bool DynamixelDevice::setCcwAngleLimit(int limit) {
    return writeWord(TURAG_DXL_ADDRESS_CCW_ANGLE_LIMIT, limit);
}

// Cw Angle Limit
bool DynamixelDevice::getCwAngleLimit(int* limit) {
    return readWord(TURAG_DXL_ADDRESS_CW_ANGLE_LIMT, limit);
}

bool DynamixelDevice::setCwAngleLimit(int limit) {
    return writeWord(TURAG_DXL_ADDRESS_CW_ANGLE_LIMT, limit);
}

//Present speed
bool DynamixelDevice::getPresentSpeed(int speed){
    return readWord(TURAG_DXL_ADDRESS_PRESENT_SPEED, speed);
}

//Present load
bool DynamixelDevice::getPresentLoad(int load){
    return readWord(TURAG_DXL_ADDRESS_PRESENT_LOAD, load);
}

}
} // namespace TURAG
} // namespace Feldbus

