/* Servodokumentation verfügbar unter http://support.robotis.com/en/techsupport_eng.htm#product/dynamixel/rx_series/rx-10.htm */
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

    if (hasReachedTransmissionErrorLimit()) {
        turag_warningf("%s: transmission error limit reached", name);
    }

    return !hasReachedTransmissionErrorLimit();
}
//Protected
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
        ++attempt;
        success = turag_dxl_read_word(myId, address, word);
    }

    if (success) {
        myTransmissionErrorCounter = 0;
        printLastDeviceError();
        return true;
    } else {
        turag_warningf("%s: rs485 transceive failed", name);
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
        ++attempt;
        success = turag_dxl_read_byte(myId, address, byte);
    }

    if (success) {
        myTransmissionErrorCounter = 0;
        printLastDeviceError();
        return true;
    } else {
        turag_warningf("%s: rs485 transceive failed", name);
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
        ++attempt;
        success = turag_dxl_write_word(myId, address, word);
    }

    if (success) {
        myTransmissionErrorCounter = 0;
        printLastDeviceError();
        return true;
    } else {
        turag_warningf("%s: rs485 transceive failed", name);
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
        ++attempt;
        success = turag_dxl_write_byte(myId, address, byte);
    }

    if (success) {
        myTransmissionErrorCounter = 0;
        printLastDeviceError();
        return true;
    } else {
        turag_warningf("%s: rs485 transceive failed", name);
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

//Moving
/*function: checks for any movements
 *output: true/false
 */
bool DynamixelDevice::isMoving(bool *movement){
    if (!movement) return false;
    int tempMove=0;
    if(readWord(TURAG_DXL_ADDRESS_MOVING, &tempMove)) {
        if (tempMove==1){
            *movement= true;
        } else {
            *movement= false;
        }
    return true;
    } else {
        return false;
    }
}

//isOverload
/*function: checks for device overload errror
 *output: true/false
 */
bool DynamixelDevice::isOverload(){
    if(hasDeviceError(Error::overload)){
        return true;
    } else {
        return false;
    }
}

//Punch
bool DynamixelDevice::getPunch(int *punch){
    return readWord(TURAG_DXL_ADDRESS_PUNCH, punch);
}

bool DynamixelDevice::setPunch(int punch){
    return writeWord(TURAG_DXL_ADDRESS_PUNCH, punch);
}


//AlarmShutdown
/*function: Dynamixel can protect itself by detecting errors occur during the operation. The errors can be set are as the table below.
 *ouput: 8-bit, website for more information
 *input: 8-bit, same as output
 */
bool DynamixelDevice::getAlarmShutdown(int *value){
    return readByte(TURAG_DXL_ADDRESS_ALARM_SHUTDOWN, value);
}

bool DynamixelDevice::setAlarmShutdown(int value){
    return writeByte(TURAG_DXL_ADDRESS_ALARM_SHUTDOWN, value);
}

//AlarmLED
/*see Alarmshutdown */
bool DynamixelDevice::getAlarmLED(int *value){
    return readByte(TURAG_DXL_ADDRESS_ALARM_LED, value);
}

bool DynamixelDevice::setAlarmLED(int value){
    return writeByte(TURAG_DXL_ADDRESS_ALARM_LED, value);
}

//TorqueEnable
/*function: 1 = Generates Torque by impressing the power to the motor.
 * 0 =  Keeps Torque from generating by interrupting the power of motor.
 *output: true/false
 *input: true/false
 */
bool DynamixelDevice::getTorqueEnable(bool* enable) {
    if (!enable) return false;
    int tempEnable = 0;
    if (readByte(TURAG_DXL_ADDRESS_TORQUE_ENABLE, &tempEnable)) {
        if (tempEnable == 1){
            *enable = true;
        } else {
            *enable = false;
        }
        return true;
    } else {
        return false;
    }
}

bool DynamixelDevice::setTorqueEnable(bool enable) {
    return writeByte(TURAG_DXL_ADDRESS_TORQUE_ENABLE, enable);
}

/*---------------------------------------------------------------------------*/

//Ccw AngleLimit
/*function: The angle limit allows the motion to be restrained.
 *output: AngleLimit(degree)
 *input: AngleLimit (degree)
 */
bool DynamixelDevice::getCcwAngleLimit(float* limit) {
    if (!limit) return false;
    int tempLimit=0;
    if (readWord(TURAG_DXL_ADDRESS_CCW_ANGLE_LIMIT, &tempLimit)){
        *limit=(float)tempLimit*TURAG_DXL_FACTOR_DEGREE;
    } else{
        return false;
    }
    return true;
}

bool DynamixelDevice::setCcwAngleLimit(float limit) {
    if((150<=limit)&&(limit<=300)){
        int temp_Limit=0;
        temp_Limit=limit/TURAG_DXL_FACTOR_DEGREE;
        return writeWord(TURAG_DXL_ADDRESS_CCW_ANGLE_LIMIT, temp_Limit);
    } else {
       return false;
    }
}

// Cw Angle Limit
bool DynamixelDevice::getCwAngleLimit(float* limit) {
    if (!limit) return false;
    int tempLimit=0;
    if (readWord(TURAG_DXL_ADDRESS_CW_ANGLE_LIMT, &tempLimit)){
        *limit=(float)tempLimit*TURAG_DXL_FACTOR_DEGREE;
    } else{
        return false;
    }
    return true;
}

bool DynamixelDevice::setCwAngleLimit(float limit) {
    if((0<=limit)&&(limit<=150)){
    int temp_Limit=0;
    temp_Limit=limit/TURAG_DXL_FACTOR_DEGREE;
    return writeWord(TURAG_DXL_ADDRESS_CW_ANGLE_LIMT, temp_Limit);
    } else {
            return false;
    }
}

//Compliance Margin
/*It exists in each direction of CW/CCW and means the error between goal position and present position.
The range of the value is 0~255, and the unit is the same as Goal Position.(Address 30,31).
The greater the value, the more difference occurs.
*/
bool DynamixelDevice::getCwComplianceMargin(int *margin){
    return readByte(TURAG_DXL_ADDRESS_CW_COMPLIANCE_MARGIN, margin);
}

bool DynamixelDevice::setCwComplianceMargin(int margin){
    return writeByte(TURAG_DXL_ADDRESS_CW_COMPLIANCE_MARGIN, margin);
}

bool DynamixelDevice::getCcwComplianceMargin(int *margin){
    return readByte(TURAG_DXL_ADDRESS_CCW_COMPLIANCE_MARGIN, margin);
}

bool DynamixelDevice::setCcwComplianceMargin(int margin){
    return writeByte(TURAG_DXL_ADDRESS_CCW_COMPLIANCE_MARGIN, margin);
}

//Compliance Slope
bool DynamixelDevice::getCwComplianceSlope(int *slope){
    return readByte(TURAG_DXL_ADDRESS_CW_COMPLIANCE_MARGIN, slope);
}

bool DynamixelDevice::setCwComplianceSlope(int slope){
    return writeByte(TURAG_DXL_ADDRESS_CW_COMPLIANCE_MARGIN, slope);
}

bool DynamixelDevice::getCcwComplianceSlope(int *slope){
    return readByte(TURAG_DXL_ADDRESS_CCW_COMPLIANCE_MARGIN, slope);
}

bool DynamixelDevice::setCcwComplianceSlope(int slope){
    return writeByte(TURAG_DXL_ADDRESS_CCW_COMPLIANCE_MARGIN, slope);
}
/*----------------------------------------------------------------------------*/
//Moving Speed
bool DynamixelDevice::getMovingSpeed(int* speed) {
    if (!speed) return false;
    readWord(TURAG_DXL_ADDRESS_MOVING_SPEED, speed);
    /*int value=0;
    if (readWord(TURAG_DXL_ADDRESS_MOVING_SPEED, &value)){
        *speed=(float)value*TURAG_DXL_FACTOR_MOVING_SPEED;
        return true;
    } else {
        return false;
    }*/
    return true;
}

/* Achtung, es findet KEINE Einheitenumrechung statt*/
bool DynamixelDevice::setMovingSpeed(int speed){
        return (writeWord(TURAG_DXL_ADDRESS_MOVING_SPEED, speed));
}

//Present speed
bool DynamixelDevice::getPresentSpeed(float* speed, int* direction) {
    if ((!speed)||(!direction)) return false;
    int value = 0;
    if (readWord(TURAG_DXL_ADDRESS_PRESENT_SPEED, &value)) {
        *direction = (value & (1<<10)) ? 1 : 0;
        *speed = (value & 1023) * TURAG_DXL_FACTOR_PRESENT_SPEED;
        return true;
    } else {
        return false;
    }

}

//Present load
bool DynamixelDevice::getPresentLoad(int* load, int* direction) {
    if ((!load) || (!direction)) return false;
    int value = 0;
    if (readWord(TURAG_DXL_ADDRESS_PRESENT_LOAD, &value)) {
        *direction = (value & (1<<10)) ? 1 : 0;
        *load = (value & 1023) * TURAG_DXL_FACTOR_PRESENT_LOAD;
        return true;
    } else {
        return false;
    }
}

//Present Voltage
bool DynamixelDevice::getPresentVoltage(float* u) {
    if (!u) return false;
    int voltage = 0;
    if (readByte(TURAG_DXL_ADDRESS_PRESENT_VOLTAGE, &voltage)) {
        *u = (float)(voltage / TURAG_DXL_FACTOR_VOLTAGE);
        return true;
    } else {
        return false;
    }
}

//Baud Rate
bool DynamixelDevice::getBaudRate(float* rate) {
    if (!rate) return false;

    int baud_data = 0;
    if (!readByte(TURAG_DXL_ADDRESS_BAUDRATE, &baud_data)) return false;

    *rate = 2000000.0f/((float)baud_data+1.0f);

    return true;
}

bool DynamixelDevice::setBaudRate(float targetRate){
    int data = (2000000 + targetRate / 2) / targetRate - 1;
    float setRate= 2000000.0f / ((float)data + 1.0f);
    float tolerance = (targetRate - setRate) / targetRate;
    if ((tolerance >= -0.03) && (tolerance<=0.03)) {
        return writeByte(TURAG_DXL_ADDRESS_BAUDRATE, data);
    } else {
        return false;
    }
}

//Return Delay Time
bool DynamixelDevice::getReturnDelayTime(int* time) {
    int result = 0;

    if (readByte(TURAG_DXL_ADDRESS_RETURN_DELAY, &result)) {
        *time = result * 2;
        return true;
    } else {
        return false;
    }
}

bool DynamixelDevice::setReturnDelayTime(int time){
    return writeByte(TURAG_DXL_ADDRESS_RETURN_DELAY, time / 2);
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
    if (!voltage) return false;
    if (readByte(TURAG_DXL_ADDRESS_VOLTAGE_LIMIT_LOW, voltage)){
        *voltage=*voltage/TURAG_DXL_FACTOR_VOLTAGE;
        return true;
    } else {
        return false;
    }
}

bool DynamixelDevice::setVoltageLimitLow(int voltage){
    voltage=voltage*TURAG_DXL_FACTOR_VOLTAGE;
    if((50<=voltage) && (voltage<=250)){
        return writeByte(TURAG_DXL_ADDRESS_VOLTAGE_LIMIT_LOW, voltage);
    }
    else {
        return false;
    }
}

//Highest Limit Voltage
bool DynamixelDevice::getVoltageLimitHigh(int *voltage){
    if (!voltage) return false;
    if (readByte(TURAG_DXL_ADDRESS_VOLTAGE_LIMIT_HIGH, voltage)){
        *voltage=*voltage/TURAG_DXL_FACTOR_VOLTAGE;
        return true;
    } else {
        return false;
    }
}

bool DynamixelDevice::setVoltageLimitHigh(int voltage){
    voltage=voltage*TURAG_DXL_FACTOR_VOLTAGE;
    if((50<=voltage) && (voltage<=250)){
        return writeByte(TURAG_DXL_ADDRESS_VOLTAGE_LIMIT_HIGH, voltage);
    }
    else {
        return false;
    }
}

//Max Torque
bool DynamixelDevice::getTorqueMax(int *max){
    if (!max) return false;
    if (readWord(TURAG_DXL_ADDRESS_MAX_TORQUE, max)){
        *max=*max/TURAG_DXL_FACTOR_TORQUE;
        return true;
    } else {
        return false;
    }
}

bool DynamixelDevice::setTorqueMax(int max){
    max=max*TURAG_DXL_FACTOR_TORQUE;
    return writeWord(TURAG_DXL_ADDRESS_MAX_TORQUE, max);
}

//Torque Limit
bool DynamixelDevice::getTorqueLimit(int *max){
    if(!max) return false;
    if (readWord(TURAG_DXL_ADDRESS_TORQUE_LIMIT, max)){
        *max=*max/TURAG_DXL_FACTOR_TORQUE;
        return true;
    } else {
        return false;
    }
}

bool DynamixelDevice::setTorqueLimit(int max){
    max=max*TURAG_DXL_FACTOR_TORQUE;
    return writeWord(TURAG_DXL_ADDRESS_TORQUE_LIMIT, max);
}


/*Position
------------------------------------------------------------------*/
//Current Position
bool DynamixelDevice::getCurrentPosition(float* position) {
    if (!position) return false;

    int presentPosition=0;
    if (readWord(TURAG_DXL_ADDRESS_PRESENT_POSITION, &presentPosition)){
        *position=(float)presentPosition*TURAG_DXL_FACTOR_DEGREE;
        return true;
    } else {
        return false;
    }
}

//Goal Position
bool DynamixelDevice::setGoalPosition(float position) {
    if ((0<=position)&&(position<=300)){
        int targetPosition=(int)position/TURAG_DXL_FACTOR_DEGREE;
        return writeWord(TURAG_DXL_ADDRESS_GOAL_POSITION, targetPosition);
    } else {
        return false;
    }
}

bool DynamixelDevice::getGoalPosition(float* position) {
    if (!position) return false;

    int goalPosition=0;
    if (readWord(TURAG_DXL_ADDRESS_GOAL_POSITION, &goalPosition)){
        *position=(float)goalPosition*TURAG_DXL_FACTOR_DEGREE;
        return true;
    } else {
        return false;
    }
}

/*----------------------------------------------------------------*/

} // namespace TURAG
} // namespace Feldbus

