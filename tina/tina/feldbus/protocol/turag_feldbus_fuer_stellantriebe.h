/**
 *  @brief      TURAG-Feldbus für Stellantriebe protocol definitions.
 *  @file       turag_feldbus_fuer_stellantriebe.h
 *  @date       24.02.2014
 *  @author     Martin Oemus <martin@oemus.net>
 *  @ingroup    feldbus-protocol
 *
 */

#ifndef TURAG_FELDBUS_FUER_STELLANTRIEBE_H_
#define TURAG_FELDBUS_FUER_STELLANTRIEBE_H_


#include "turag_feldbus_bus_protokoll.h"

/**
 * @name device types
 * @{
 */
#define TURAG_FELDBUS_STELLANTRIEBE_DEVICE_TYPE_DC          (0x01)
#define TURAG_FELDBUS_STELLANTRIEBE_DEVICE_TYPE_SERVO       (0x02)
#define TURAG_FELDBUS_STELLANTRIEBE_DEVICE_TYPE_STEPPER     (0x03)
#define TURAG_FELDBUS_STELLANTRIEBE_DEVICE_TYPE_POWERPLANE  (0x04)
#define TURAG_FELDBUS_STELLANTRIEBE_DEVICE_TYPE_ESCON       (0x05)
#define TURAG_FELDBUS_STELLANTRIEBE_DEVICE_TYPE_FARBSENSOR  (0x06)
///@}


/**
 * @name special values for command factors
 * @{
 */
#define TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE    (0.0f)
///@}


/**
 * @name command access types
 * @{
 */
#define TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_NO_WRITE_ACCESS  (0x00)
#define TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS     (0x01)
///@}


/**
 * @name command length
 * \note Diese Definitionen dürfen nicht geändert werden, da die Implementierung
 * davon abhängt.
 * @{
 */
#define TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_NONE         (0x00)
#define TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_CHAR         (0x01)
#define TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT        (0x02)
#define TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_LONG         (0x04)
#define TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_NONE_TEXT    (0x05)
#define TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_FLOAT        (0x06)
///@}


/**
 * @name command info
 * @{
 */
#define TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET                    (0x00)
#define TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_COMMANDSET_SIZE    (0x01)
#define TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_NAME_LENGTH        (0x02)
#define TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_NAME               (0x03)
///@}


/**
 * @name structured output
 * @{
 */
#define TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_GET               (0xFF)

#define TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_CONTROL           (0xFF)
#define TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_SET_STRUCTURE     (0x00)
#define TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_GET_BUFFER_SIZE   (0x01)

#define TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_TABLE_OK          (0x01)
#define TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_TABLE_REJECTED    (0x00)
///@}


/**
 * @name Command Keys für alle Stellantriebe
 * @{
 */
#define RS485_STELLANTRIEBE_KEY_CURRENT_ANGLE       (0x01)
#define RS485_STELLANTRIEBE_KEY_DESIRED_ANGLE       (0x02)
#define RS485_STELLANTRIEBE_KEY_MAX_ANGLE           (0x03)
#define RS485_STELLANTRIEBE_KEY_MIN_ANGLE           (0x04)

#define RS485_STELLANTRIEBE_KEY_CURRENT_VELOCITY    (0x05)
#define RS485_STELLANTRIEBE_KEY_DESIRED_VELOCITY    (0x06)
#define RS485_STELLANTRIEBE_KEY_MAX_VELOCITY        (0x07)

#define RS485_STELLANTRIEBE_KEY_CURRENT_CURRENT     (0x08)
#define RS485_STELLANTRIEBE_KEY_DESIRED_CURRENT     (0x09)
#define RS485_STELLANTRIEBE_KEY_MAX_CURRENT         (0x0A)

#define RS485_STELLANTRIEBE_KEY_CURRENT_PWM         (0x0B)
#define RS485_STELLANTRIEBE_KEY_DESIRED_PWM         (0x0C)
#define RS485_STELLANTRIEBE_KEY_MAX_PWM             (0x0D)

#define RS485_STELLANTRIEBE_KEY_VOLTAGE             (0x0E)
#define RS485_STELLANTRIEBE_KEY_STATUS              (0x0F)
#define RS485_STELLANTRIEBE_KEY_CONTROL_STATE       (0x10)

///@}

/**
 * @name low bytes for set command: RS485_STELLANTRIEBE_KEY_CONTROL_STATE
 * @{
 */
#define RS485_STELLANTRIEBE_CONTROL_STATE_ENABLE        (1)
#define RS485_STELLANTRIEBE_CONTROL_STATE_DISABLE       (0)
///@}



/**
 * @name low bytes for RS485_STELLANTRIEBE_KEY_STATUS
 * flags are mutually exclusive. The meaning of the error flag is
 * implementation specific.
 * @{
 */
#define RS485_STELLANTRIEBE_STATUS_NONE                 (0)
#define RS485_STELLANTRIEBE_STATUS_ANGLE_REACHED        (1<<0)
#define RS485_STELLANTRIEBE_STATUS_VELOCITY_REACHED     (1<<1)
#define RS485_STELLANTRIEBE_STATUS_CURRENT_REACHED      (1<<2)
#define RS485_STELLANTRIEBE_STATUS_ERROR                (1<<3)
///@}



// ----------------------------------------------
// ----------------------------------------------
/**
 * @name Command Keys für Servo
 * @{
 */
#define RS485_STELLANTRIEBE_SERVO_KEY_CTRL_POS_GAIN_P     (0x11)    // P-Gain for Position control
#define RS485_STELLANTRIEBE_SERVO_KEY_CTRL_POS_GAIN_I     (0x12)    // I-Gain Position control
#define RS485_STELLANTRIEBE_SERVO_KEY_CTRL_CUR_GAIN_P     (0x13)    // P-Gain Current control
#define RS485_STELLANTRIEBE_SERVO_KEY_CTRL_CUR_GAIN_I     (0x14)    // I-Gain Current control
#define RS485_STELLANTRIEBE_SERVO_KEY_CTRL_CUR_STATIC_FF  (0x15)    // current static feed forward

///@}




/**
 * @name Command Keys für DC Motor
 * @{
 */
#define RS485_STELLANTRIEBE_DC_KEY_SWITCH_STATUS    (0x11)
#define RS485_STELLANTRIEBE_DC_KEY_RETURN_TO_HOME   (0x12)  // requests the slave to return to some defined position where it adjusts
                                                            // its current angle appropriately, takes velocity as argument
///@}

/**
 * @name Command Keys für Stepper
 * @{
 */
#define TURAG_FELDBUS_STELLANTRIEBE_STEPPER_DESIRED_ANGLE "desiredAngle"
#define TURAG_FELDBUS_STELLANTRIEBE_STEPPER_CURRENT_ANGLE "currentAngle"
#define TURAG_FELDBUS_STELLANTRIEBE_STEPPER_MAX_VELOCITY  "maxVelocity"
#define TURAG_FELDBUS_STELLANTRIEBE_STEPPER_HOME          "home"
#define TURAG_FELDBUS_STELLANTRIEBE_STEPPER_STATUS        "status"
#define TURAG_FELDBUS_STELLANTRIEBE_STEPPER_SYNC_ANGLE    "syncAngle"
#define TURAG_FELDBUS_STELLANTRIEBE_STEPPER_SYNC_HOME     "syncHome"
#define TURAG_FELDBUS_STELLANTRIEBE_STEPPER_ENABLE        "enable"
///@}

/**
 * @name Werte für Stepper Status
 * @{
 */
#define TURAG_FELDBUS_STELLANTRIEBE_STEPPER_STATUS_ANGLE_REACHED    (0x01)
///@}

/**
 * @name Broadcasts für Stepper
 * @{
 */
#define TURAG_FELDBUS_STELLANTRIEBE_BROADCAST_SYNC_ANGLE    (0x01)
#define TURAG_FELDBUS_STELLANTRIEBE_BROADCAST_SYNC_HOME     (0x02)
///@}

/**
 * @name Command Keys für ESCON-Motherboard
 * @{
 */
#define TURAG_FELDBUS_STELLANTRIEBE_ESCON_CURRENT_CURRENT   "current_current"
#define TURAG_FELDBUS_STELLANTRIEBE_ESCON_CURRENT_RPM       "current_rpm"
#define TURAG_FELDBUS_STELLANTRIEBE_ESCON_CURRENT_STATUS    "current_status"
#define TURAG_FELDBUS_STELLANTRIEBE_ESCON_SET_ENABLE        "set_enable"
#define TURAG_FELDBUS_STELLANTRIEBE_ESCON_SET_DIRECTION     "set_direction"
#define TURAG_FELDBUS_STELLANTRIEBE_ESCON_DESIRED_CURRENT   "desired_current"
///@}

/**
 * @name Broadcasts für ESCON-Motherboard
 * @{
 */
#define TURAG_FELDBUS_STELLANTRIEBE_BROADCAST_SYNC_ESCON    (0x03)
///@}


/**
 * @name Command Keys für Servo
 * @{
 */
#define TURAG_FELDBUS_STELLANTRIEBE_SERVO_MEASURED_ANGLE    "measuredAngle"
#define TURAG_FELDBUS_STELLANTRIEBE_SERVO_DESIRED_ANGLE     "desiredAngle"
#define TURAG_FELDBUS_STELLANTRIEBE_SERVO_MAX_ANGLE         "maxAngle"
#define TURAG_FELDBUS_STELLANTRIEBE_SERVO_MIN_ANGLE         "minAngle"
#define TURAG_FELDBUS_STELLANTRIEBE_SERVO_MEASURED_CURRENT  "measuredCurrent"
#define TURAG_FELDBUS_STELLANTRIEBE_SERVO_DESIRED_CURRENT   "desiredCurrent"
#define TURAG_FELDBUS_STELLANTRIEBE_SERVO_MAX_CURRENT       "maxCurrent"
#define TURAG_FELDBUS_STELLANTRIEBE_SERVO_CTR_MODE          "ctrMode"
#define TURAG_FELDBUS_STELLANTRIEBE_SERVO_ENABLE            "enable"
#define TURAG_FLEDBUD_STELLANTRIEBE_SERVO_INVERT            "invert"
///@}

/**
 * @name Werte für Servo Status
 * @{
 */
#define TURAG_FELDBUS_STELLANTRIEBE_SERVO_STATUS_ANGLE_REACHED  (0x01)
///@}

/**
 * @name Broadcasts für Servo
 * @{
 */
#define TURAG_FELDBUS_STELLANTRIEBE_BROADCAST_SERVO_SYNC_ANGLE  (0x01)
#define TURAG_FELDBUS_STELLANTRIEBE_BROADCAST_SERVO_SYNC_HOME   (0x02)
///@}

#endif
