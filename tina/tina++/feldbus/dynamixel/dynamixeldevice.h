#ifndef DYNAMIXELDEVICE_H
#define DYNAMIXELDEVICE_H

#include <tina/feldbus/dynamixel/dynamixel.h>

namespace TURAG {
namespace Feldbus {

// -------------------------------------------------------------
// - config
// -------------------------------------------------------------
// these values could be relocated to an external header to make
// the whole thing configurable.

// number of trials before we let the transmission fail
#define TURAG_DYNAMIXEL_DEVICE_CONFIG_MAX_TRANSMISSION_ATTEMPTS		5

// specifies how many unsuccessful transmission attempts in a row are accepted
// before defining the device as dysfunctional
#define TURAG_DYNAMIXEL_DEVICE_CONFIG_MAX_TRANSMISSION_ERRORS		35


class DynamixelDevice
{
public:
    enum class Error {
        voltage = TURAG_DXL_ERRBIT_VOLTAGE,
        angle = TURAG_DXL_ERRBIT_ANGLE,
        overheat = TURAG_DXL_ERRBIT_OVERHEAT,
        range = TURAG_DXL_ERRBIT_RANGE,
        checksum = TURAG_DXL_ERRBIT_CHECKSUM,
        overload = TURAG_DXL_ERRBIT_OVERLOAD,
        instruction = TURAG_DXL_ERRBIT_INSTRUCTION
    };

protected:
    int myId;
    unsigned int maxTransmissionAttempts;
    const unsigned int maxTransmissionErrors;
    unsigned int myTransmissionErrorCounter;

    bool hasCheckedAvailabilityYet;

    int modelNumber_;
    int firmwareVersion_;
    int servoID_;

    bool readWord(int address, int* word);
    bool readByte(int address, int* byte);
    bool writeWord(int address, int word);
    bool writeByte(int address, int byte);
    void printLastDeviceError(void);
    bool hasDeviceError(DynamixelDevice::Error index);
    bool hasReachedTransmissionErrorLimit(void) const { return myTransmissionErrorCounter >= maxTransmissionErrors; }
    void clearTransmissionErrors(void) { myTransmissionErrorCounter = 0; }


public:
    const char* name;

    //Konstruktor für die Klasse DynamixelDevice
    DynamixelDevice(const char* name_, int id,
                    unsigned int max_transmission_attempts = TURAG_DYNAMIXEL_DEVICE_CONFIG_MAX_TRANSMISSION_ATTEMPTS,
                    unsigned int max_transmission_errors = TURAG_DYNAMIXEL_DEVICE_CONFIG_MAX_TRANSMISSION_ERRORS) :
        /*Initialisierungen */
        myId(id), maxTransmissionAttempts(max_transmission_attempts), maxTransmissionErrors(max_transmission_errors),
        myTransmissionErrorCounter(0), hasCheckedAvailabilityYet(false),
        modelNumber_(-1), firmwareVersion_(-1), name(name_)   {}
    virtual ~DynamixelDevice() {}

    //deklariert und implementiert
    unsigned int getID(void) const { return myId; }
    void setMaxTransmissionAttempts(int attempts) { maxTransmissionAttempts = attempts; myTransmissionErrorCounter= 0; }

    //deklariert
    virtual bool getModelNumber(int* modelNumber);
    virtual bool getFirmwareVersion(int* version);
    virtual bool isAvailable(void);
    virtual bool isMoving(bool* movement);

    virtual bool getTemperature(int* temperature);
    virtual bool setLed(bool on);
    virtual bool getCurrentPosition(int* position);
    virtual bool setGoalPosition(int position);

    virtual bool getTorqueEnable(bool* enabled);
    virtual bool setTorqueEnable(bool enable);

    virtual bool getCcwAngleLimit(float* limit);
    virtual bool setCcwAngleLimit(float limit);

    virtual bool getCwAngleLimit(float* limit);
    virtual bool setCwAngleLimit(float limit);

    virtual bool getPresentSpeed(int* speed);
    virtual bool getPresentLoad(int* load, int* direction);
    virtual bool getPresentVoltage(int* u);

    virtual bool getBaudRate(int* rate);
    virtual bool setBaudRate(int rate);

    virtual bool getReturnDelayTime(int* time);
    virtual bool setReturnDelayTime(int time);

    virtual bool getTemperatureLimit(int* temperature);
    virtual bool setTemperatureLimit(int temperature);

    virtual bool getVoltageLimitLow(int* voltage);
    virtual bool setVoltageLimitLow(int voltage);

    virtual bool getVoltageLimitHigh(int* voltage);
    virtual bool setVoltageLimitHigh(int voltage);

    virtual bool getTorqueMax(int* max);
    virtual bool setTorqueMax(int max);

    virtual bool isOverload(bool* overload);
};


} // namespace TURAG
} // namespace Feldbus

#endif // DYNAMIXELDEVICE_H
