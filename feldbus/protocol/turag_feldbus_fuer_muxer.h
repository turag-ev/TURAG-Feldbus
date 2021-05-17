/**
 *  @brief		Muxer protocol definitions.
 *  @file		muxer-protocol.h
 *  @date		29.08.2018
 *  @author		Martin Oemus <martin@oemus.net>
 *  @ingroup	feldbus-protocol
 *
 */

#ifndef MUXER_PROTOCOL_H_
#define MUXER_PROTOCOL_H_


#include <tina/feldbus/protocol/turag_feldbus_bus_protokoll.h>


/**
 * @name device types
 * @{
 */
#define TURAG_FELDBUS_MUXER_GENERIC		0x01
///@}




/**
 * @name commands
 * @{
 */

/**
 * set: <0x01><Lowbyte><Highbyte>  	response: <0 error / 1 success>
 *
 * Defines the number of triggers, after which the cycle is repeated
 */
#define TURAG_FELDBUS_MUXER_CYCLE_LENGTH  0x01

/**
 * get: <0x02>						response: <Lowbyte><Highbyte>
 *
 * Hardware defined maximum of the cycle length.
 */
#define TURAG_FELDBUS_MUXER_MAX_CYCLE_LENGTH  0x02



/**
 * set: <0x03><Lowbyte><Highbyte>  	response: <0 error / 1 success>
 * get: <0x03>						response: <Lowbyte><Highbyte>
 *
 * Represents the current position in the cycle. Automatically increased
 * on trigger and automatically reset to zero once it hits cycle length.
 */
#define TURAG_FELDBUS_MUXER_CYCLE_INDEX  0x03

/**
 * set: <0x04><mode>  	response: <0 error / 1 success>
 *
 * Sets the trigger mode. Possible below.
 */
#define TURAG_FELDBUS_MUXER_TRIGGER_MODE 0x04

#define TURAG_FELDBUS_MUXER_TRIGGER_MODE_NONE 0x00
#define TURAG_FELDBUS_MUXER_TRIGGER_MODE_FALLING 0x01
#define TURAG_FELDBUS_MUXER_TRIGGER_MODE_RISING 0x02
#define TURAG_FELDBUS_MUXER_TRIGGER_MODE_BOTH 0x03

/**
 * set: <0x05><Lowbyte index><Highbyte index><transmitter><receiver>  	response: <0 error / 1 success>
 *
 * Sets the active transmitter and receiver for the given cycle. 0 means no transmitter/receiver is active.
 */
#define TURAG_FELDBUS_MUXER_CONFIG 0x05

/**
 * set: <0x06><0x01><0x01><0x01>
 *
 * Resets the configuration for every cycle.
 */
#define TURAG_FELDBUS_MUXER_CLEAR_CONFIG 0x06

/**
 * set: <0x07><transmitter><receiver>  	response: <0 error / 1 success>
 *
 * sets the currently active transmitter and receiver. 0 to deactivate transmitter/receiver.
 */
#define TURAG_FELDBUS_MUXER_SET_MANUAL_OUTPUT 0x07


/**
 * <0x08>		response: empty
 *
 * Enables analog output.
 */
#define TURAG_FELDBUS_MUXER_ENABLE  0x08

/**
 * <0x09>		response: empty
 *
 * Disables analog output.
 */
#define TURAG_FELDBUS_MUXER_DISABLE  0x09



///@}



#endif /* MUXER_PROTOCOL_H_ */
