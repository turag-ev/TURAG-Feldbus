/**
 *  @brief      TURAG feldbus DC motor class
 *  @file       dcmotor.h
 *  @date       22.03.2013
 *  @author     Martin Oemus
 * 
 */


#ifndef TURAGFELDBUSDCMOTOR_H_
#define TURAGFELDBUSDCMOTOR_H_

#include "legacystellantriebedevice.h"

namespace TURAG {
namespace Feldbus {

/*
 *
 */
class DCMotor : public TURAG::Feldbus::LegacyStellantriebeDevice {
public:
    DCMotor(const char* name, unsigned address, FeldbusAbstraction& feldbus, ChecksumType type = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE,
        const AddressLength addressLength = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_ADDRESS_LENGTH) :
        LegacyStellantriebeDevice(name, address, feldbus, type, addressLength) {}

    bool initialize(void) { return populateCommandSet(command_set, 19); }

    /**
     * @brief Liest den aktuellen Winkel in mRad.
     * @param[out] angle Puffer für den Winkel.
     * @return True bei Erfolg, ansonsten false.
     */
    bool getCurrentAngle(float* angle) { return getValue(RS485_STELLANTRIEBE_KEY_CURRENT_ANGLE, angle); }

    /**
     * @brief Gibt den Sollwinkel in mRad zurück.
     * @param[in] angle Sollwinkel.
     * @return True bei Erfolg, ansonsten false.
     *
     * Verursacht keine Buslast.
     */
    bool getDesiredAngle(float* angle) { return getValue(RS485_STELLANTRIEBE_KEY_DESIRED_ANGLE, angle); }
    /**
     * @brief Setzt den Sollwinkel.
     * @param[in] angle Sollwinkel in mRad.
     * @return True bei Erfolg, ansonsten false.
     * @post Aktiviert die Regelung.
     */
    virtual bool setDesiredAngle(float angle) { return setValue(RS485_STELLANTRIEBE_KEY_DESIRED_ANGLE, angle); }

    /**
     * @brief Gibt den maximalen Winkel in mRad zurück.
     * @param[out] angle Maximaler Winkel.
     * @return True bei Erfolg, ansonsten false.
     *
     * Verursacht keine Buslast.
     */
    bool getMaxAngle(float* angle) { return getValue(RS485_STELLANTRIEBE_KEY_MAX_ANGLE, angle); }
    /**
     * @brief Setzt den maximalen Winkel in mRad.
     * @param[in] angle Maximaler Winkel.
     * @return True bei Erfolg, ansonsten false.
     */
    bool setMaxAngle(float angle) { return setValue(RS485_STELLANTRIEBE_KEY_MAX_ANGLE, angle); }

    /**
     * @brief Gibt den minimalen Winkel in mRad zurück.
     * @param[out] angle Minimaler Winkel.
     * @return True bei Erfolg, ansonsten false.
     *
     * Verursacht keine Buslast.
     */
    bool getMinAngle(float* angle) { return getValue(RS485_STELLANTRIEBE_KEY_MIN_ANGLE, angle); }
    /**
     * @brief Setzt den minimalen Winkel in mRad.
     * @param[in] angle Minimaler Winkel.
     * @return True bei Erfolg, ansonsten false.
     */
    bool setMinAngle(float angle) { return setValue(RS485_STELLANTRIEBE_KEY_MIN_ANGLE, angle); }

    /**
     * @brief Gibt die aktuelle Geschwindigkeit in mRad/s zurück.
     * @param[out] velocity Geschwindigkeit.
     * @return True bei Erfolg, ansonsten false.
     */
    bool getCurrentVelocity(float* velocity) { return getValue(RS485_STELLANTRIEBE_KEY_CURRENT_VELOCITY, velocity); }

    /**
     * @brief Gibt die Sollgeschwindigkeit in mRad/s zurück.
     * @param velocity Sollgeschwindigkeit.
     * @return True bei Erfolg, ansonsten false.
     *
     * Verursacht keine Buslast.
     */
    bool getDesiredVelocity(float* velocity) { return getValue(RS485_STELLANTRIEBE_KEY_DESIRED_VELOCITY, velocity); }
    /**
     * @brief Setzt die Sollgeschwindigkeit in mRad/s.
     * @param velocity Sollgeschwindigkeit.
     * @return True bei Erfolg, ansonsten false.
     * @post Aktiviert die Regelung.
     */
    bool setDesiredVelocity(float velocity) { return setValue(RS485_STELLANTRIEBE_KEY_DESIRED_VELOCITY, velocity); }

    bool getMaxVelocity(float* velocity) { return getValue(RS485_STELLANTRIEBE_KEY_MAX_VELOCITY, velocity); }
    bool setMaxVelocity(float velocity) { return setValue(RS485_STELLANTRIEBE_KEY_MAX_VELOCITY, velocity); }

    bool getCurrentCurrent(float* current) { return getValue(RS485_STELLANTRIEBE_KEY_CURRENT_CURRENT, current); }

    bool getDesiredCurrent(float* current) { return getValue(RS485_STELLANTRIEBE_KEY_DESIRED_CURRENT, current); }
    /**
     * @brief Setzt den Sollstrom in mA.
     * @param current Sollstrom.
     * @return True bei Erfolg, ansonsten false.
     * @post Aktiviert die Regelung.
     */
    bool setDesiredCurrent(float current) { return setValue(RS485_STELLANTRIEBE_KEY_DESIRED_CURRENT, current); }

    bool getMaxCurrent(float* current) { return getValue(RS485_STELLANTRIEBE_KEY_MAX_CURRENT, current); }
    bool setMaxCurrent(float current) { return setValue(RS485_STELLANTRIEBE_KEY_MAX_CURRENT, current); }

    bool getCurrentPwm(float* pwm) { return getValue(RS485_STELLANTRIEBE_KEY_CURRENT_PWM, pwm); }

    bool getDesiredPwm(float* pwm) { return getValue(RS485_STELLANTRIEBE_KEY_DESIRED_PWM, pwm); }
    bool setDesiredPwm(float pwm) { return setValue(RS485_STELLANTRIEBE_KEY_DESIRED_PWM, pwm); }

    bool getMaxPwm(float* pwm) { return getValue(RS485_STELLANTRIEBE_KEY_MAX_PWM, pwm); }
    bool setMaxPwm(float pwm) { return setValue(RS485_STELLANTRIEBE_KEY_MAX_PWM, pwm); }

    /**
     * @brief Gibt die aktuelle Versorgungsspannung in Volt zurück.
     * @param voltage Puffer für die Versorgungsspannung.
     * @return True bei Erfolg, ansonsten false.
     */
    bool getVoltage(float* voltage) { return getValue(RS485_STELLANTRIEBE_KEY_VOLTAGE, voltage); }

    /**
     * @brief Aktiviert die Regelung.
     * @return True bei Erfolg, ansonsten false.
     *
     * Die Regelung wird durch die die drei Sollwert-Stellbefehle setDesiredCurrent(), setDesiredVelocity()
     * und setDesiredAngle() automatisch aktiviert.
     */
    bool enableControl() {
        return setValue(RS485_STELLANTRIEBE_KEY_CONTROL_STATE, static_cast<int32_t>(RS485_STELLANTRIEBE_CONTROL_STATE_ENABLE));
    }
    /**
     * @brief Deaktiviert die Regelung.
     * @return True bei Erfolg, ansonsten false.
     */
    bool disableControl() {
        return setValue(RS485_STELLANTRIEBE_KEY_CONTROL_STATE, static_cast<int32_t>(RS485_STELLANTRIEBE_CONTROL_STATE_DISABLE));
    }

    /**
     * @brief Hält den Antrieb an und regelt auf die aktuelle Position.
     * @return True bei Erfolg, ansonsten false.
     */
    bool setHold(void);

    /** Status request
     * @return        True if angle reached, otherwise false.
     */
    bool hasAngleReached();

    /** Status request
     * @return        True if velocity reached, otherwise false.
     */
    bool hasVelocityReached();

    /** Status request
     * @return        True if current reached, otherwise false.
     */
    bool hasCurrentReached();

    /** Status request
     * @return        True if error detected, otherwise false.
     */
    bool hasErrorDetected();

    bool driveHome(float velocity) { return setValue(RS485_STELLANTRIEBE_DC_KEY_RETURN_TO_HOME, velocity); }

private:
    LegacyStellantriebeDevice::Command_t command_set[19];
};

} // namespace Feldbus
} // namespace TURAG

#endif /* TURAGFELDBUSDCMOTOR_H_ */


