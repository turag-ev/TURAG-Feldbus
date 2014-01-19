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

//TorqueEnable
bool DynamixelDevice::getTorqueEnable(bool* enable) {
    int tempEnable=0;
    if(readWord(TURAG_DXL_ADDRESS_TORQUE_ENABLE, &tempEnable)) {
        if (tempEnable==1){
            *enable= true;
        } else {
            *enable= false;
        }
    return true;
    } else {
        return false;
    }
}

bool DynamixelDevice::setTorqueEnable(bool enable) {
    return writeWord(TURAG_DXL_ADDRESS_TORQUE_ENABLE, enable);
}

/*---------------------------------------------------------------------------*/

//Ccw AngleLimit
bool DynamixelDevice::getCcwAngleLimit(float* limit) {
    int tempLimit=0;
    if (readWord(TURAG_DXL_ADDRESS_CCW_ANGLE_LIMIT, &tempLimit)){
        *limit=tempLimit/TURAG_DXL_DEGREE_FACTOR;
    } else{
        return false;
    }
    return true;
}

bool DynamixelDevice::setCcwAngleLimit(float limit) {
    int temp_Limit=0;
    temp_Limit=limit*TURAG_DXL_DEGREE_FACTOR;
    return writeWord(TURAG_DXL_ADDRESS_CCW_ANGLE_LIMIT, temp_Limit);
}

// Cw Angle Limit
bool DynamixelDevice::getCwAngleLimit(float* limit) {
    int tempLimit=0;
    if (readWord(TURAG_DXL_ADDRESS_CW_ANGLE_LIMT, &tempLimit)){
        *limit=tempLimit/TURAG_DXL_DEGREE_FACTOR;
    } else{
        return false;
    }
    return true;
}

bool DynamixelDevice::setCwAngleLimit(float limit) {
    int temp_Limit=0;
    temp_Limit=limit*TURAG_DXL_DEGREE_FACTOR;
    return writeWord(TURAG_DXL_ADDRESS_CW_ANGLE_LIMT, temp_Limit);
}

/*----------------------------------------------------------------------------*/
//Present speed
bool DynamixelDevice::getPresentSpeed(int* speed){
    return readWord(TURAG_DXL_ADDRESS_PRESENT_SPEED, speed);
}

//Present load
bool DynamixelDevice::getPresentLoad(int* load){
    return readWord(TURAG_DXL_ADDRESS_PRESENT_LOAD, load);
}

//Present Voltage
bool DynamixelDevice::getPresentVoltage(int* u){
    return readByte(TURAG_DXL_ADDRESS_PRESENT_VOLTAGE, u);
}

//Baud Rate - muss noch bearbeitet werden...
bool DynamixelDevice::getBaudRate(int* rate){
    return readByte(TURAG_DXL_ADDRESS_BAUDRATE, rate);
}
bool DynamixelDevice::setBaudRate(int rate){
    return writeByte(TURAG_DXL_ADDRESS_BAUDRATE, rate);
}

//Return Delay Time
bool DynamixelDevice::getReturnDelayTime(int* time){
    return readByte(TURAG_DXL_ADDRESS_RETURN_DELAY, time);
}

bool DynamixelDevice::setReturnDelayTime(int time){
    return writeByte(TURAG_DXL_ADDRESS_RETURN_DELAY, time);
}

//Highest Limit Temperature
bool DynamixelDevice::getTemperatureLimit(int* temperature){
    return readByte(TURAG_DXL_ADDRESS_TEMPERATURE_LIMIT, temperature);
}

bool DynamixelDevice::setTemperatureLimit(int temperature){
    return writeByte(TURAG_DXL_ADDRESS_TEMPERATURE_LIMIT, temperature);
}

//Lowest Limit Voltage
bool DynamixelDevice::getVoltageLimitLow(int *voltage){
    return readByte(TURAG_DXL_ADDRESS_VOLTAGE_LIMIT_LOW, voltage);
}
bool DynamixelDevice::setVoltageLimitLow(int voltage){
    return writeByte(TURAG_DXL_ADDRESS_VOLTAGE_LIMIT_LOW, voltage);
}

//Highest Limit Voltage
bool DynamixelDevice::getVoltageLimitHigh(int *voltage){
    return readByte(TURAG_DXL_ADDRESS_VOLTAGE_LIMIT_HIGH, voltage);
}
bool DynamixelDevice::setVoltageLimitHigh(int voltage){
    return writeByte(TURAG_DXL_ADDRESS_VOLTAGE_LIMIT_HIGH, voltage);
}

//Max Torque
bool DynamixelDevice::getTorqueMax(int *max){
    return readWord(TURAG_DXL_ADDRESS_MAX_TORQUE, max);
}
bool DynamixelDevice::setTorqueMax(int max){
    return writeWord(TURAG_DXL_ADDRESS_MAX_TORQUE, max);
}


/*Position
------------------------------------------------------------------*/
//Current Position
bool DynamixelDevice::getCurrentPosition(int* position) {
    return readWord(TURAG_DXL_ADDRESS_PRESENT_POSITION, position);
}

//Goal Position
bool DynamixelDevice::setGoalPosition(int position) {
    return writeWord(TURAG_DXL_ADDRESS_GOAL_POSITION, position);
}

/*----------------------------------------------------------------*/

} // namespace TURAG
} // namespace Feldbus

