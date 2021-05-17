/**
 *  @brief		TURAG-Feldbus für ASEBs protocol definitions.
 *  @file		turag_feldbus_fuer_aseb.h
 *  @date		12.01.2014
 *  @author		Martin Oemus <martin@oemus.net>
 *  @ingroup	feldbus-protocol
 *
 */

#ifndef TURAG_FELDBUS_FUER_ASEB_H_
#define TURAG_FELDBUS_FUER_ASEB_H_


#include "turag_feldbus_bus_protokoll.h"

/**
 * @name device types
 * @{
 */
#define TURAG_FELDBUS_ASEB_GENERIC		0x01
///@}


/**
 * @name channel index range
 * @{
 */

/// Maximale Anzahl der Channels pro Typ.
#define TURAG_FELDBUS_ASEB_MAX_CHANNELS_PER_TYPE		16
/// Channel-Offset für digitale Eingänge.
#define TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_INPUT	1
/// Channel-Offset für analoge Eingänge.
#define TURAG_FELDBUS_ASEB_INDEX_START_ANALOG_INPUT		17
/// Channel-Offset für digitale Ausgänge.
#define TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_OUTPUT	33
/// Channel-Offset für PWM-Ausgänge.
#define TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT		49
///@}


/**
 * @name commands
 * @{
 */
#define TURAG_FELDBUS_ASEB_SYNC							0xff
#define TURAG_FELDBUS_ASEB_PWM_SPEED                    243
#define TURAG_FELDBUS_ASEB_SYNC_SIZE					244
#define TURAG_FELDBUS_ASEB_CHANNEL_NAME					245
#define TURAG_FELDBUS_ASEB_CHANNEL_NAME_LENGTH			246
#define TURAG_FELDBUS_ASEB_NUMBER_OF_DIGITAL_INPUTS		247
#define TURAG_FELDBUS_ASEB_NUMBER_OF_DIGITAL_OUTPUTS	248
#define TURAG_FELDBUS_ASEB_NUMBER_OF_ANALOG_INPUTS		249
#define TURAG_FELDBUS_ASEB_ANALOG_INPUT_RESOLUTION		250
#define TURAG_FELDBUS_ASEB_ANALOG_INPUT_FACTOR			251
#define TURAG_FELDBUS_ASEB_NUMBER_OF_PWM_OUTPUTS		252
#define TURAG_FELDBUS_ASEB_PWM_OUTPUT_FREQUENCY			253
#define TURAG_FELDBUS_ASEB_PWM_OUTPUT_MAX_VALUE			254
///@}


#endif
