/**
 *  @file		feldbus_config.h
 *  @date		09-2014
 *  @author		Martin Oemus <martin@oemus.net>
 *  @ingroup feldbus-slave-base
 * 
 * @brief Sample feldbus config header for a slave device. Copy this file to your device's sources and adjust to your needs.
 * 
 * The base implementation and the protocol implementations
 * for TURAG Feldbus slave devices are configurable. Thus a 
 * header is required to choose the right configuration.
 * 
 * For more details regarding the meaning of those config values
 * please refer to https://www.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus/
 * 
 * It is recommended to use this file to include the header containing
 * the device implementation if available or feldbus.h if the former
 * is not available.
 * 
 * The following paragraphs contain listings of possible 
 * configuration values. These values do not necessarily need
 * to be complete. For a complete list refer to the corresponding header
 * files.
 * 
 * The same holds valid for the list of configuration options
 * itself - which is only complete in regard to the base implementation.
 * The implementation for a specified device protocol might add
 * additional configuration options which could be compulsory
 * but can not be covered here.
 * 
 */
#error THIS FILE IS FOR DOCUMUNTATIONAL PURPOSES ONLY.

#ifndef FELDBUS_CONFIG_H_
#define FELDBUS_CONFIG_H_


#include <turag_feldbus_bus_protokoll.h>



/**
 * Defines the device protocol type of the slave.
 * 
 * Possible values:
 * - \ref TURAG_FELDBUS_DEVICE_PROTOCOL_STELLANTRIEBE
 * - \ref TURAG_FELDBUS_DEVICE_PROTOCOL_LOKALISIERUNGSSENSOREN
 * - \ref TURAG_FELDBUS_DEVICE_PROTOCOL_ASEB	
 * - \ref TURAG_FELDBUS_DEVICE_PROTOCOL_BOOTLOADER
 */
#define TURAG_FELDBUS_DEVICE_PROTOCOL


/**
 * Defines the device type of the slave.
 * 
 * Possible values depend on the specified device protocol 
 * and can be found in the corresponding headers.
 */
#define TURAG_FELDBUS_DEVICE_TYPE_ID


/**
 * Defines the length of the address used in 
 * each package.
 * 
 * Possible values:
 * - 1
 * - 2
 */
#define TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH


/**
 * Defines the crc algorithm to use for the device. 
 * 
 * Possible values:
 * - \ref TURAG_FELDBUS_CHECKSUM_XOR
 * - \ref TURAG_FELDBUS_CHECKSUM_CRC8_ICODE
 */
#define TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE 	


/**
 * Defines the size of the I/O-buffers. The base implementation
 * allocates 3 buffers of the specified size.
 */
#define TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE			


/**
 * If set one, debug functions like print_text() become available.
 * If set to zero the function calls are removed and no output is generated.
 */
#define TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED		


/**
 * If set to one the device listens for broadcasts messages.
 * If set to zero these packages are droppped which saves a bit
 * of processing time.
 */
#define TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE		





#endif /* FELDBUS_CONFIG_H_ */
 
