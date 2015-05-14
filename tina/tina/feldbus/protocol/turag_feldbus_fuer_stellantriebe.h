/**
 *  @brief		TURAG-Feldbus für Stellantriebe protocol definitions.
 *  @file		turag_feldbus_fuer_stellantriebe.h
 *  @date		24.02.2014
 *  @author		Martin Oemus <martin@oemus.net>
 *  @ingroup	feldbus-protocol
 *
 */

#ifndef TURAG_FELDBUS_FUER_STELLANTRIEBE_H_
#define TURAG_FELDBUS_FUER_STELLANTRIEBE_H_


#include "turag_feldbus_bus_protokoll.h"

/**
 * @name device types
 * @{
 */
#define TURAG_FELDBUS_STELLANTRIEBE_DEVICE_TYPE_DC          0x01
#define TURAG_FELDBUS_STELLANTRIEBE_DEVICE_TYPE_SERVO		0x02
///@}

/**
 * @name special values for command factors
 * @{
 */
#define TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE  	0.0f
///@}


/**
 * @name command access types
 * @{
 */
#define TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_NO_WRITE_ACCESS	0x00
#define TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS		0x01
///@}


/**
 * @name command length
 * \note Diese Definitionen dürfen nicht geändert werden, da die Implementierung
 * davon abhängt.
 * @{
 */
#define TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_NONE		0x00
#define TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_CHAR		0x01
#define TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT	0x02
#define TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_LONG		0x04
///@}


/**
 * @name command info
 * @{
 */
#define TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET                    0x00
#define TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_COMMANDSET_SIZE	0x01
#define TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_NAME_LENGTH        0x02
#define TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_NAME               0x03
///@}


/**
 * @name structured output
 * @{
 */
#define TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_GET               0xFF

#define TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_CONTROL			0xFF
#define TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_SET_STRUCTURE     0x00
#define TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_GET_BUFFER_SIZE   0x01

#define TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_TABLE_OK          0x01
#define TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_TABLE_REJECTED	0x00
///@}


/**
 * @name command keys for servo device
 * @{
 */
#define RS485_STELLANTRIEBE_KEY_CURRENT_ANGLE		(0x01)	// current angle
#define RS485_STELLANTRIEBE_KEY_DESIRED_ANGLE		(0x02)
#define RS485_STELLANTRIEBE_KEY_MAX_ANGLE           (0x03)
#define RS485_STELLANTRIEBE_KEY_MIN_ANGLE           (0x04)
#define RS485_STELLANTRIEBE_KEY_CURRENT_VELOCITY    (0x05)	// current velocity
#define RS485_STELLANTRIEBE_KEY_DESIRED_VELOCITY    (0x06)	// current velocity
#define RS485_STELLANTRIEBE_KEY_MAX_VELOCITY		(0x07)	// current velocity
#define RS485_STELLANTRIEBE_KEY_CURRENT_CURRENT		(0x08)	// current motor current
#define RS485_STELLANTRIEBE_KEY_DESIRED_CURRENT		(0x09)	// current motor current
#define RS485_STELLANTRIEBE_KEY_MAX_CURRENT         (0x0A)	// current motor current
#define RS485_STELLANTRIEBE_KEY_CURRENT_PWM         (0x0B)	// current PWM ratio
#define RS485_STELLANTRIEBE_KEY_DESIRED_PWM         (0x0C)	// current PWM ratio
#define RS485_STELLANTRIEBE_KEY_MAX_PWM             (0x0D)	// current PWM ratio
#define RS485_STELLANTRIEBE_KEY_VOLTAGE             (0x0E)	// current supply voltage
#define RS485_STELLANTRIEBE_KEY_STATUS              (0x0F)	// current status, see details below
#define RS485_STELLANTRIEBE_KEY_CONTROL_STATE	    (0x10)	// control state
#define RS485_STELLANTRIEBE_KEY_CTRL_POS_GAIN_P     (0x11)	// P-Gain for Position control
#define RS485_STELLANTRIEBE_KEY_CTRL_POS_GAIN_I     (0x12)	// I-Gain Position control
#define RS485_STELLANTRIEBE_KEY_CTRL_VEL_GAIN_P     (0x13)	// P-Gain Velocity
#define RS485_STELLANTRIEBE_KEY_CTRL_VEL_GAIN_I     (0x14)	// I-Gain Velocity
#define RS485_STELLANTRIEBE_KEY_CTRL_CUR_GAIN_P     (0x15)	// P-Gain Current control
#define RS485_STELLANTRIEBE_KEY_CTRL_CUR_GAIN_I     (0x16)	// I-Gain Current control
#define RS485_STELLANTRIEBE_KEY_CTRL_CUR_STATIC_FF  (0x17)	// current static feed forward

///@}
													
/**
 * @name low bytes for set command: RS485_STELLANTRIEBE_KEY_CONTROL_STATE
 * @{
 */
#define RS485_STELLANTRIEBE_CONTROL_STATE_ENABLE		(1)
#define RS485_STELLANTRIEBE_CONTROL_STATE_DISABLE       (0)
///@}



/**
 * @name low bytes for RS485_STELLANTRIEBE_KEY_STATUS
 * flags are mutually exclusive. The meaning of the error flag is
 * implementation specific.
 * @{
 */
#define RS485_STELLANTRIEBE_STATUS_NONE                 (0)
#define RS485_STELLANTRIEBE_STATUS_ANGLE_REACHED		(1<<0)
#define RS485_STELLANTRIEBE_STATUS_VELOCITY_REACHED     (1<<1)
#define RS485_STELLANTRIEBE_STATUS_CURRENT_REACHED      (1<<2)
#define RS485_STELLANTRIEBE_STATUS_ERROR                (1<<3)
///@}



// ----------------------------------------------
// ----------------------------------------------
/**
 * @name DC motor command keys
 * @{
 */
#define RS485_STELLANTRIEBE_DC_KEY_SWITCH_STATUS    (0x11)
#define RS485_STELLANTRIEBE_DC_KEY_RETURN_TO_HOME	(0x12)	// requests the slave to return to some defined position where it adjusts 
///@}														// its current angle appropriately, takes velocity as argument



#endif
