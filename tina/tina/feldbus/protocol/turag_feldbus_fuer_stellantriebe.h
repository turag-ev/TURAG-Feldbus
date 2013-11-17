#ifndef TURAG_FELDBUS_FUER_STELLANTRIEBE_H_
#define TURAG_FELDBUS_FUER_STELLANTRIEBE_H_

/* 
 * This header file contains the specification for the protocol
 * "TURAG-FeldBus für Stellantriebe". Refer to the DokuWiki for documentation.
 */

#include "turag_feldbus_bus_protokoll.h"

// device types
#define TURAG_FELDBUS_STELLANTRIEBE_DEVICE_TYPE_DC			0x01
#define TURAG_FELDBUS_STELLANTRIEBE_DEVICE_TYPE_SERVO		0x02


// protocol versions
#define TURAG_FELDBUS_STELLANTRIEBE_DEVICE_PROTOCOL_VERSION_1_0 		0x01


// command keys
#define RS485_MOTOR_COM_NOT_IMPLEMENTED		(0)		// slaves should return this after reception of an unsupported get command

#define RS485_MOTOR_SET_COMMAND_MASK		(0x80)		// bitmask to identify set commands

// set commands
#define RS485_MOTOR_COM_INDEX_DES_ANGLE		(0x80)		// desired angle
#define RS485_MOTOR_COM_INDEX_DES_VEL		(0x81)		// desired velocity
#define RS485_MOTOR_COM_INDEX_DES_CUR		(0x82)		// desired current
#define RS485_MOTOR_COM_INDEX_DES_TOR		(0x83)		// desired torque
#define RS485_MOTOR_COM_INDEX_MAX_VEL		(0x84)		// maximum velocity
#define RS485_MOTOR_COM_INDEX_MAX_CUR		(0x85)		// maximum current
#define RS485_MOTOR_COM_INDEX_MAX_TOR		(0x86)		// maximum torque
#define RS485_MOTOR_COM_INDEX_MAX_PWM		(0x87)		// maximum PWM ratio
#define RS485_MOTOR_COM_INDEX_MAX_ANGLE		(0x88)		// maximum angle
#define RS485_MOTOR_COM_INDEX_MIN_ANGLE		(0x89)	// minimum angle
#define RS485_MOTOR_COM_INDEX_CTRL_STATE	(0x8A)	// control state
#define RS485_MOTOR_COM_INDEX_CTRL_TYPE		(0x8B)	// control type
#define RS485_MOTOR_COM_RETURN_TO_HOME		(0x8C)	// requests the slave to return to some defined position where it adjusts 
													// its current angle appropriately, takes velocity as argument
#define RS485_MOTOR_COM_INDEX_DES_PWM		(0x8D)

//**Commands for BLDC-Driver
#define RS485_BLDC_COM_START				(0x8E)
#define RS485_BLDC_COM_STOP					(0x8F)
													
// low bytes for set command: COM_INDEX_CTRL_STATE 
// turn motor control on/off
#define RS485_MOTOR_CONTROL_STATE_ENABLE		(1)
#define RS485_MOTOR_CONTROL_STATE_DISABLE		(0)

// low bytes for set command: COM_INDEX_CTRL_TYPE
// choose control cascade
#define RS485_MOTOR_CONTROL_TYPE_NONE	(0)
#define RS485_MOTOR_CONTROL_TYPE_POS	(1<<0)
#define RS485_MOTOR_CONTROL_TYPE_VEL	(1<<1)
#define RS485_MOTOR_CONTROL_TYPE_CUR	(1<<2)


// get commands
#define RS485_MOTOR_COM_INDEX_CURRENT_ANGLE		(0x01)	// current angle
#define RS485_MOTOR_COM_INDEX_CURRENT_VEL		(0x02)	// current velocity
#define RS485_MOTOR_COM_INDEX_CURRENT_CUR		(0x03)	// current motor current
#define RS485_MOTOR_COM_INDEX_CURRENT_TORQUE	(0x04)	// current torque
#define RS485_MOTOR_COM_INDEX_CURRENT_DIR		(0x05)	// current direction
#define RS485_MOTOR_COM_INDEX_CURRENT_VBAT		(0x06)	// current battery voltage
#define RS485_MOTOR_COM_INDEX_CURRENT_PWM		(0x07)	// current PWM ratio
#define RS485_MOTOR_COM_INDEX_STATUS			(0x08)	// current status, see details below
#define RS485_MOTOR_COM_INDEX_SWITCH_STATUS		(0x09)

#define RS485_BLDC_STATUS					(0x0A)

// low bytes for COM_INDEX_CURRENT_DIR
#define RS485_MOTOR_CURRENT_DIR_FORWARD			(0x01)		
#define RS485_MOTOR_CURRENT_DIR_BACKWARD		(0xFF)	// (-1)


// low bytes for COM_INDEX_STATUS
// flags are mutually exclusive. The meaning of the error flag is
// implementation specific.
#define RS485_MOTOR_STATUS_NONE					(0)
#define RS485_MOTOR_STATUS_ANGLE_REACHED		(1<<0)
#define RS485_MOTOR_STATUS_VELOCITY_REACHED		(1<<1)
#define RS485_MOTOR_STATUS_CURRENT_REACHED		(1<<2)
#define RS485_MOTOR_STATUS_ERROR				(1<<3)

//Flags for BLDC driver for RS485_BLDC_STATUS
#define RS485_BLDC_FLAG_RUNNING					(1<<0)
#define RS485_BLDC_FLAG_STOPPED					(1<<1)

#endif
