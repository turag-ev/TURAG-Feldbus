#ifndef DYNAMIXELDEVICE_H
#define DYNAMIXELDEVICE_H

#include <tina/feldbus/dynamixel/dynamixel.h>
#include <tina++/feldbus/host/device.h>

namespace TURAG {
namespace Feldbus {

/**
 * @brief Dynamixel Communication 1.0-Implementierung.
 *
 * Diese Klasse kann benutzt werden, um mit Dynamixel-Servos zu
 * kommunizieren, die das Communication 1.0-Protokoll benutzen.
 *
 * \see https://intern.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus;dynamixel-servos
 * \see http://support.robotis.com/en/techsupport_eng.htm#product/dynamixel/dxl_communication.htm
 * \see http://support.robotis.com/en/techsupport_eng.htm#product/dynamixel/rx_series/rx-10.htm
 *
 * \note Die aktuelle Implementierung wurde für Servos der RX-Reihe entwickelt,
 * sollte aber problemlos erweiterbar sein.
 *
 * \bug nicht thread-safe
 * \bug der globalTransmissionErrorCounter von Device wird nicht hochgezählt, obwohl das sinnvoll wäre.
 *
 */
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

    //Konstruktor für die Klasse DynamixelDevice
	/**
	 * @brief Konstruktor
	 * @param name_ Bezeichnung des Gerätes
	 * @param id Adresse
	 * @param max_transmission_attempts
	 * @param max_transmission_errors
	 */
    DynamixelDevice(const char* name_, int id,
                    unsigned int max_transmission_attempts = TURAG_FELDBUS_DEVICE_CONFIG_MAX_TRANSMISSION_ATTEMPTS,
                    unsigned int max_transmission_errors = TURAG_FELDBUS_DEVICE_CONFIG_MAX_TRANSMISSION_ERRORS) :
        /*Initialisierungen */
        name(name_), myId(id), maxTransmissionAttempts(max_transmission_attempts), maxTransmissionErrors(max_transmission_errors),
        myTransmissionErrorCounter(0), myTotalTransmissionErrors(0), myTotalTransmissions(0), hasCheckedAvailabilityYet(false),
        modelNumber_(-1), firmwareVersion_(-1)  {}

#if TURAG_USE_LIBSUPCPP_RUNTIME_SUPPORT
    virtual ~DynamixelDevice() { }
#endif

    bool hasReachedTransmissionErrorLimit(void) const { return myTransmissionErrorCounter >= maxTransmissionErrors; }
    void clearTransmissionErrors(void) { myTransmissionErrorCounter = 0; }
    unsigned int getTotalTransmissionErrors(void) { return myTotalTransmissionErrors; }
    unsigned int getTotalTransmissions(void) { return myTotalTransmissions; }

    unsigned int getID(void) const { return myId; }
    bool setID(int address);

    virtual bool getModelNumber(int* modelNumber);
    virtual bool getFirmwareVersion(int* version);
    virtual bool isAvailable(void);
    virtual bool isMoving(bool* movement);

    virtual bool getTemperature(int* temperature);
    virtual bool setLed(bool on);
    virtual bool getCurrentPosition(float* position);
    virtual bool setGoalPosition(float position);
    virtual bool resetGoalPosition(void);
    virtual bool getGoalPosition(float* position);

    virtual bool getTorqueEnable(bool* enabled);
    virtual bool setTorqueEnable(bool enable);

    virtual bool getCcwAngleLimit(float* limit);
    virtual bool setCcwAngleLimit(float limit);

    virtual bool getCwAngleLimit(float* limit);
    virtual bool setCwAngleLimit(float limit);

    virtual bool getCwComplianceMargin(int* margin);
    virtual bool setCwComplianceMargin(int margin);
    virtual bool getCcwComplianceMargin(int* margin);
    virtual bool setCcwComplianceMargin(int margin);

    virtual bool getCwComplianceSlope(int* slope);
    virtual bool setCwComplianceSlope(int slope);
    virtual bool getCcwComplianceSlope(int* slope);
    virtual bool setCcwComplianceSlope(int slope);

    virtual bool getPresentSpeed(float* speed, int* direction);
    virtual bool getPresentLoad(int* load, int* direction);
    virtual bool getPresentVoltage(float* u);

    virtual bool getBaudRate(float* rate);
    virtual bool setBaudRate(float targetRate);

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
    virtual bool getTorqueLimit(int* max);
    virtual bool setTorqueLimit(int max);

    virtual bool isOverload();

    virtual bool getAlarmShutdown(int* value);
    virtual bool setAlarmShutdown(int value);
    virtual bool getAlarmLED(int* value);
    virtual bool setAlarmLED(int value);

    virtual bool getMovingSpeed(int* speed);
    virtual bool setMovingSpeed(int speed);

    virtual bool getPunch(int* punch);
    virtual bool setPunch(int punch);

	const char* name;

protected:
	bool readByte(int address, int* byte);
	bool readWord(int address, int* word);
	bool writeByte(int address, int byte);
	bool writeWord(int address, int word);
	void printLastDeviceError(void);
	bool hasDeviceError(DynamixelDevice::Error index);

	int myId;
    unsigned int maxTransmissionAttempts;
    const unsigned int maxTransmissionErrors;
    unsigned int myTransmissionErrorCounter;
    unsigned int myTotalTransmissionErrors;
    unsigned int myTotalTransmissions;

    bool hasCheckedAvailabilityYet;

    int modelNumber_;
    int firmwareVersion_;
    int servoID_;

    int targetPosition;
};


} // namespace TURAG
} // namespace Feldbus

#endif // DYNAMIXELDEVICE_H
