/**
 *  @brief		TURAG feldbus actor base class
 *  @file		TURAGFeldbusAktor.cpp
 *  @date		22.03.2013
 *  @author		Martin Oemus
 *
 */

#include "aktor.h"

#include <tina++/tina.h>
#include <tina/debug.h>

namespace TURAG {
namespace Feldbus {

struct AktorGetData {
	uint8_t key;
	uint16_t value;
} _packed;

bool Aktor::getValue(uint8_t key, uint16_t* value) {
	Request<uint8_t> request;
	Response<AktorGetData> receive;
	request.data = key;

	bool success = transceive(request, &receive);
	if (success) {
		if (value) *value = receive.data.value;
		return true;
	} else {
		return false;
	}
}

struct AktorSetData {
	uint8_t key;
	uint16_t value;
} _packed;

bool Aktor::setValue(uint8_t key, uint16_t value) {
	Request<AktorSetData> request;
	Response<> response;
	
	request.data.key = key;
	request.data.value = value;
	return transceive(request, &response);
}

bool Aktor::hasAngleReached() {
	uint16_t status;

	bool success = getValue(RS485_MOTOR_COM_INDEX_STATUS, &status);
	if (success && static_cast<uint8_t>(status) == RS485_MOTOR_STATUS_ANGLE_REACHED)
		return true;
	else
		return false;
}

bool Aktor::hasVelocityReached() {
	uint16_t status;

	bool success = getValue(RS485_MOTOR_COM_INDEX_STATUS, &status);

	if (success && static_cast<uint8_t>(status) == RS485_MOTOR_STATUS_VELOCITY_REACHED)
		return true;
	else
		return false;
}

bool Aktor::hasCurrentReached() {
	uint16_t status;

	bool success = getValue(RS485_MOTOR_COM_INDEX_STATUS, &status);

	if (success && static_cast<uint8_t>(status) == RS485_MOTOR_STATUS_CURRENT_REACHED)
		return true;
	else
		return false;
}

bool Aktor::hasErrorDetected() {
	uint16_t status = RS485_MOTOR_STATUS_NONE;

	bool success = getValue(RS485_MOTOR_COM_INDEX_STATUS, &status);

	if (success && static_cast<uint8_t>(status) == RS485_MOTOR_STATUS_ERROR)
		return true;
	else
		return false;
}

bool Aktor::enableControl() {
	if (!setValue(RS485_MOTOR_COM_INDEX_CTRL_TYPE, myControlType) ||
			!setValue(RS485_MOTOR_COM_INDEX_CTRL_STATE, RS485_MOTOR_CONTROL_STATE_ENABLE))
		return false;

	if (myHomecomingRequested) {
		myControlType = RS485_MOTOR_CONTROL_TYPE_POS;
		myHomecomingRequested = false;
	}

	return true;
}

bool Aktor::disableControl() {
	if (!setValue(RS485_MOTOR_COM_INDEX_CTRL_STATE, RS485_MOTOR_CONTROL_STATE_DISABLE))
		return false;

	// we set the internal control type regardless of what happens on the bus. It's better to reactivate once to often but to
	// have it disabled without knowing it.
	myControlType = RS485_MOTOR_CONTROL_TYPE_NONE;
	return setValue(RS485_MOTOR_COM_INDEX_CTRL_STATE, RS485_MOTOR_CONTROL_TYPE_NONE);
}

bool Aktor::setAngle(int angle) {
	if (angle < 0) angle = 0;

	if (!setValue(RS485_MOTOR_COM_INDEX_DES_ANGLE, convertAngle(angle)))
		return false;

	if (myControlType != RS485_MOTOR_CONTROL_TYPE_POS) {
		if (!setValue(RS485_MOTOR_COM_INDEX_CTRL_TYPE, RS485_MOTOR_CONTROL_TYPE_POS))
			return false;

		myControlType = RS485_MOTOR_CONTROL_TYPE_POS;
	}
	return true;
}

bool Aktor::setVelocity(int velocity) {
	if (!setValue(RS485_MOTOR_COM_INDEX_DES_VEL, convertVelocity(velocity)))
		return false;

	if (myControlType != RS485_MOTOR_CONTROL_TYPE_VEL) {
		if (!setValue(RS485_MOTOR_COM_INDEX_CTRL_TYPE, RS485_MOTOR_CONTROL_TYPE_VEL))
			return false;

		myControlType = RS485_MOTOR_CONTROL_TYPE_VEL;
	}
	return true;
}

bool Aktor::setCurrent(int current) {
	infof("%s: setCurrent to %d mA", name, current);

	if (!setValue(RS485_MOTOR_COM_INDEX_DES_CUR, convertCurrent(current)))
		return false;

	if (myControlType != RS485_MOTOR_CONTROL_TYPE_CUR) {
		if (!setValue(RS485_MOTOR_COM_INDEX_CTRL_TYPE, RS485_MOTOR_CONTROL_TYPE_CUR))
			return false;

		myControlType = RS485_MOTOR_CONTROL_TYPE_CUR;
	}
	return true;
}

bool Aktor::setTorque(int torque) {
	if (!setValue(RS485_MOTOR_COM_INDEX_DES_TOR, convertTorque(torque)))
		return false;

	if (myControlType != RS485_MOTOR_CONTROL_TYPE_CUR) {
		if (!setValue(RS485_MOTOR_COM_INDEX_CTRL_TYPE, RS485_MOTOR_CONTROL_TYPE_CUR))
			return false;

		myControlType = RS485_MOTOR_CONTROL_TYPE_CUR;
	}
	return true;
}

bool Aktor::getAngle(int* angle) {
	uint16_t temp;

	if (!getValue(RS485_MOTOR_COM_INDEX_CURRENT_ANGLE, &temp))
		return false;

	if (angle)
		*angle = convertAngle(temp);

	return true;
}

bool Aktor::getVelocity(int* velocity) {
	uint16_t temp;

	if (!getValue(RS485_MOTOR_COM_INDEX_CURRENT_VEL, &temp))
		return false;

	if (velocity)
		*velocity = convertVelocity(temp);

	return true;
}

bool Aktor::getCurrent(int* current) {
	uint16_t temp;

	if (!getValue(RS485_MOTOR_COM_INDEX_CURRENT_CUR, &temp))
		return false;

	if (current)
		*current = convertCurrent(temp);

	return true;
}

bool Aktor::getTorque(int* torque) {
	uint16_t temp;

	if (!getValue(RS485_MOTOR_COM_INDEX_CURRENT_TORQUE, &temp))
		return false;

	if (torque)
		*torque = convertTorque(temp);

	return true;
}

bool Aktor::getDirection(int* direction) {
	uint16_t temp;

	if (!getValue(RS485_MOTOR_COM_INDEX_CURRENT_DIR, &temp))
		return false;

	if (direction)
		*direction = static_cast<short>(temp);

	return true;
}

bool Aktor::getVoltage(int* voltage) {
	uint16_t temp;

	if (!getValue(RS485_MOTOR_COM_INDEX_CURRENT_VBAT, &temp))
		return false;

	if (voltage)
		*voltage = convertVoltage(temp);

	return true;
}

bool Aktor::getPWM(int* pwm) {
	uint16_t temp;

	if (!getValue(RS485_MOTOR_COM_INDEX_CURRENT_PWM, &temp))
		return false;

	if (pwm)
		*pwm = convertPWM(temp);

	return true;
}

bool Aktor::setMaxVelocity(int maxVelocity) {
	if (!setValue(RS485_MOTOR_COM_INDEX_MAX_VEL, convertVelocity(maxVelocity)))
		return false;

	myMaxVelocity = maxVelocity;

	return true;
}

bool Aktor::setMaxCurrent(int maxCurrent) {
	if (!setValue(RS485_MOTOR_COM_INDEX_MAX_CUR, convertCurrent(maxCurrent)))
		return false;

	myMaxCurrent = maxCurrent;

	return true;
}

bool Aktor::setMaxPWM(int maxPWM) {
	if (!setValue(RS485_MOTOR_COM_INDEX_MAX_PWM, convertPWM(maxPWM)))
		return false;

	myMaxPWM = maxPWM;

	return true;
}

bool Aktor::setMaxTorque(int maxTorque) {
	if (!setValue(RS485_MOTOR_COM_INDEX_MAX_TOR, convertTorque(maxTorque)))
		return false;

	myMaxTorque = maxTorque;

	return true;
}

bool Aktor::isAvailable(void) {
	if (!hasCheckedAvailabilityYet) {

		while (!getValue(RS485_MOTOR_COM_INDEX_STATUS, nullptr) &&
					 !hasReachedTransmissionErrorLimit())
		{ }

		hasCheckedAvailabilityYet = true;
	}

	return !hasReachedTransmissionErrorLimit();
}

bool Aktor::setMaxAngle(int angle) {
	if (!setValue(RS485_MOTOR_COM_INDEX_MAX_ANGLE, convertAngle(angle)))
		return false;

	myMaxAngle = angle;

	return true;
}

bool Aktor::setMinAngle(int angle) {
	if (!setValue(RS485_MOTOR_COM_INDEX_MIN_ANGLE, convertAngle(angle)))
		return false;

	myMinAngle = angle;

	return true;
}

bool Aktor::driveHome(int velocity) {
	if (!setValue(RS485_MOTOR_COM_RETURN_TO_HOME, convertVelocity(velocity)))
		return false;

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
} // namespace TURAG

