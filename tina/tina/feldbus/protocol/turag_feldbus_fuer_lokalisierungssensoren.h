
#ifndef TURAG_FELDBUS_FUER_LOKALISIERUNGSSENSOREN_H_
#define TURAG_FELDBUS_FUER_LOKALISIERUNGSSENSOREN_H_

/* 
 * This header file contains the specification for the protocol
 * "TURAG-FeldBus für Lokalisierungssensoren". Refer to the DokuWiki for documentation.
 */


#include "turag_feldbus_bus_protokoll.h"


#define TURAG_FELDBUS_LOKALISIERUNGSSENSOREN_DEVICE_TYPE_COLORSENSOR			0x01
#define TURAG_FELDBUS_LOKALISIERUNGSSENSOREN_DEVICE_TYPE_ENCODER				0x02
#define TURAG_FELDBUS_LOKALISIERUNGSSENSOREN_DEVICE_TYPE_GEGI					0x03

// color sensor
#define CMD_CLR_CLR									(0)
#define CMD_CLR_HSV									(1)
#define CMD_CLR_RGB									(2)
#define CMD_CLR_REQUEST_CONTROL_DATA_TRANSMISSION	(3)
#define CMD_CLR_TRANSMIT_CONTROL_DATA				(4)


// encoder
#define CMD_ENC_READ_INCREMENTS						(0)


// GeGi
#define CMD_GEGI_READ_DATA		(0x00)
#define CMD_GEGI_OPPO_POS		(0x01)
#define CMD_GEGI_LED			(0x02)
#define CMD_GEGI_MEASURE		(0x40)			

#define CMD_GEGI_ACK			(0)		//0 for ack, other for nack

#endif
