/**
 *  @brief		TURAG feldbus actor base class
 *  @file		TURAGFeldbusAktor.cpp
 *  @date		22.03.2013
 *  @author		Martin Oemus
 * 
 */


#include "TURAGFeldbusAktor.h"
#include <tina/crc/xor_checksum.h>
#include <tina/debug.h>

namespace TURAG {
namespace Feldbus {

bool Aktor::aktorGet(uint8_t key, uint16_t* value) {
	if (!value) return false;

	uint8_t msg[3];
	uint8_t data_package_receive[5] = { 0 };

	msg[0] = myAddress;
	msg[1] = key;
	msg[2] = xor_checksum_calculate(msg, 2);

	bool success = transceive(msg, sizeof(msg), data_package_receive, sizeof(data_package_receive));

	if (success) {
		*value = (((uint16_t)data_package_receive[3]) << 8) + data_package_receive[2];
		return true;
	} else {
		return false;
	}
}

bool Aktor::aktorSet(uint8_t key, uint16_t value) {
	uint8_t msg[5];
	uint8_t data_package_receive[2];

	msg[0] = myAddress;
	msg[1] = key;
	msg[2] = (uint8_t) value;
	msg[3] = (uint8_t) (value >> 8);
	msg[4] = xor_checksum_calculate(msg, 4);


	bool success = transceive(msg, sizeof(msg), data_package_receive, sizeof(data_package_receive));

	if (success) {
		return true;
	} else {
		return false;
	}
}


bool Aktor::hasAngleReached() {
	uint16_t status = RS485_MOTOR_STATUS_NONE;

	bool success = aktorGet(RS485_MOTOR_COM_INDEX_STATUS, &status);

	if (success && (uint8_t)status == RS485_MOTOR_STATUS_ANGLE_REACHED)
		return true;
	else
		return false;
}

bool Aktor::hasVelocityReached() {
	uint16_t status = RS485_MOTOR_STATUS_NONE;

	bool success = aktorGet(RS485_MOTOR_COM_INDEX_STATUS, &status);

	if (success && (uint8_t)status == RS485_MOTOR_STATUS_VELOCITY_REACHED)
		return true;
	else
		return false;
}

bool Aktor::hasCurrentReached() {
	uint16_t status = RS485_MOTOR_STATUS_NONE;

	bool success = aktorGet(RS485_MOTOR_COM_INDEX_STATUS, &status);

	if (success && (uint8_t)status == RS485_MOTOR_STATUS_CURRENT_REACHED)
		return true;
	else
		return false;
}

bool Aktor::hasErrorDetected() {
	uint16_t status = RS485_MOTOR_STATUS_NONE;

	bool success = aktorGet(RS485_MOTOR_COM_INDEX_STATUS, &status);

	if (success && (uint8_t)status == RS485_MOTOR_STATUS_ERROR)
		return true;
	else
		return false;
}

bool Aktor::enableControl() {
	if (!aktorSet(RS485_MOTOR_COM_INDEX_CTRL_TYPE, myControlType)) return false;
	if (!aktorSet(RS485_MOTOR_COM_INDEX_CTRL_STATE, RS485_MOTOR_CONTROL_STATE_ENABLE)) return false;

	if (myHomecomingRequested) {
		myControlType = RS485_MOTOR_CONTROL_TYPE_POS;
		myHomecomingRequested = false;
	}

	return true;
}

bool Aktor::disableControl() {
	if (!aktorSet(RS485_MOTOR_COM_INDEX_CTRL_STATE, RS485_MOTOR_CONTROL_STATE_DISABLE)) return false;

	// we set the internal control type regardless of what happens on the bus. It's better to reactivate once to often but to
	// have it disabled without knowing it.
	myControlType = RS485_MOTOR_CONTROL_TYPE_NONE;
	return aktorSet(RS485_MOTOR_COM_INDEX_CTRL_STATE, RS485_MOTOR_CONTROL_TYPE_NONE);
}

bool Aktor::setAngle(int angle) {
	if (angle < 0) angle = 0;

	if (!aktorSet(RS485_MOTOR_COM_INDEX_DES_ANGLE, convertAngle(angle))) return false;

	if (myControlType != RS485_MOTOR_CONTROL_TYPE_POS) {
		if (!aktorSet(RS485_MOTOR_COM_INDEX_CTRL_TYPE, RS485_MOTOR_CONTROL_TYPE_POS)) return false;
		myControlType = RS485_MOTOR_CONTROL_TYPE_POS;
	}
	return true;
}

bool Aktor::setVelocity(int velocity) {
	if (!aktorSet(RS485_MOTOR_COM_INDEX_DES_VEL, convertVelocity(velocity))) return false;

	if (myControlType != RS485_MOTOR_CONTROL_TYPE_VEL) {
		if (!aktorSet(RS485_MOTOR_COM_INDEX_CTRL_TYPE, RS485_MOTOR_CONTROL_TYPE_VEL)) return false;
		myControlType = RS485_MOTOR_CONTROL_TYPE_VEL;
	}
	return true;
}

bool Aktor::setCurrent(int current) {
	infof("%s: setCurrent to %d mA", name, current);

	if (!aktorSet(RS485_MOTOR_COM_INDEX_DES_CUR, convertCurrent(current))) return false;

	if (myControlType != RS485_MOTOR_CONTROL_TYPE_CUR) {
		if (!aktorSet(RS485_MOTOR_COM_INDEX_CTRL_TYPE, RS485_MOTOR_CONTROL_TYPE_CUR)) return false;
		myControlType = RS485_MOTOR_CONTROL_TYPE_CUR;
	}
	return true;
}

bool Aktor::setTorque(int torque) {
	if (!aktorSet(RS485_MOTOR_COM_INDEX_DES_TOR, convertTorque(torque))) return false;

	if (myControlType != RS485_MOTOR_CONTROL_TYPE_CUR) {
		if (!aktorSet(RS485_MOTOR_COM_INDEX_CTRL_TYPE, RS485_MOTOR_CONTROL_TYPE_CUR)) return false;
		myControlType = RS485_MOTOR_CONTROL_TYPE_CUR;
	}
	return true;
}

bool Aktor::getAngle(int* angle) {
	if (!angle) return false;

	unsigned short temp;

	if (!aktorGet(RS485_MOTOR_COM_INDEX_CURRENT_ANGLE, &temp)) return false;
	*angle = convertAngle(temp);
	return true;
}

bool Aktor::getVelocity(int* velocity) {
	if (!velocity) return false;

	unsigned short temp;

	if (!aktorGet(RS485_MOTOR_COM_INDEX_CURRENT_VEL, &temp)) return false;
	*velocity = convertVelocity(temp);
	return true;
}

bool Aktor::getCurrent(int* current) {
	if (!current) return false;

	unsigned short temp;

	if (!aktorGet(RS485_MOTOR_COM_INDEX_CURRENT_CUR, &temp)) return false;
	*current = convertCurrent(temp);
	return true;
}

bool Aktor::getTorque(int* torque) {
	if (!torque) return false;

	unsigned short temp;

	if (!aktorGet(RS485_MOTOR_COM_INDEX_CURRENT_TORQUE, &temp)) return false;
	*torque = convertTorque(temp);
	return true;
}

bool Aktor::getDirection(int* direction) {
	if (!direction) return false;

	unsigned short temp;

	if (!aktorGet(RS485_MOTOR_COM_INDEX_CURRENT_DIR, &temp)) return false;
	*direction = (short)temp;
	return true;
}

bool Aktor::getVoltage(int* voltage) {
	if (!voltage) return false;

	unsigned short temp;

	if (!aktorGet(RS485_MOTOR_COM_INDEX_CURRENT_VBAT, &temp)) return false;
	*voltage = convertVoltage(temp);
	return true;
}

bool Aktor::getPWM(int* pwm) {
	if (!pwm) return false;

	unsigned short temp;

	if (!aktorGet(RS485_MOTOR_COM_INDEX_CURRENT_PWM, &temp)) return false;
	*pwm = convertPWM(temp);
	return true;
}

bool Aktor::setMaxVelocity(int maxVelocity) {
	if (!aktorSet(RS485_MOTOR_COM_INDEX_MAX_VEL, convertVelocity(maxVelocity))) return false;
	myMaxVelocity = maxVelocity;
	return true;
}

bool Aktor::setMaxCurrent(int maxCurrent) {
	if (!aktorSet(RS485_MOTOR_COM_INDEX_MAX_CUR, convertCurrent(maxCurrent))) return false;
	myMaxCurrent = maxCurrent;
	return true;
}

bool Aktor::setMaxPWM(int maxPWM) {
	if (!aktorSet(RS485_MOTOR_COM_INDEX_MAX_PWM, convertPWM(maxPWM))) return false;
	myMaxPWM = maxPWM;
	return true;
}

bool Aktor::setMaxTorque(int maxTorque) {
	if (!aktorSet(RS485_MOTOR_COM_INDEX_MAX_TOR, convertTorque(maxTorque))) return false;
	myMaxTorque = maxTorque;
	return true;
}

bool Aktor::isAvailable(void) {
	uint16_t dummy __attribute__((unused)) = 0 ;

	if (!hasCheckedAvailabilityYet) {
		while (!aktorGet(RS485_MOTOR_COM_INDEX_STATUS, &dummy) && !hasReachedTransmissionErrorLimit());
		hasCheckedAvailabilityYet = true;
	}

	return !hasReachedTransmissionErrorLimit();
}

bool Aktor::setMaxAngle(int angle) {
	if (!aktorSet(RS485_MOTOR_COM_INDEX_MAX_ANGLE, convertAngle(angle))) return false;
	myMaxAngle = angle;
	return true;
}

bool Aktor::setMinAngle(int angle) {
	if (!aktorSet(RS485_MOTOR_COM_INDEX_MIN_ANGLE, convertAngle(angle))) return false;
	myMinAngle = angle;
	return true;
}

bool Aktor::driveHome(int velocity) {
	bool success = aktorSet(RS485_MOTOR_COM_RETURN_TO_HOME, convertVelocity(velocity));

	if (!success) return false;

	myControlType = RS485_MOTOR_CONTROL_TYPE_VEL;
	myHomecomingRequested = true;
	return true;
}

bool Aktor::setHold(void) {
	int angle = 0;

	if (!getAngle(&angle)) return false;

	return setAngle(angle);
}

} // namespace Feldbus
} // ns TURAG

