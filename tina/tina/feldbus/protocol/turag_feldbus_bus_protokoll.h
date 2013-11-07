
#ifndef TURAG_FELDBUS_BUS_PROTOKOLL_H_
#define TURAG_FELDBUS_BUS_PROTOKOLL_H_

/* 
 * This header file contains the defines for the basic protocol specifikation of the 
 * "TURAG-FeldBus". Refer to the DokuWiki for documentation.
 */

#define TURAG_FELDBUS_MASTER_ADDR		(0x80)
#define TURAG_FELDBUS_BROADCAST_ADDR		(0x00)


#define TURAG_FELDBUS_CHECKSUM_XOR			0x00
#define TURAG_FELDBUS_CHECKSUM_CRC8_ICODE		0x01

#endif
