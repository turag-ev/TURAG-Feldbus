/**
 *  @brief		Implements slave side TURAG feldbus für Stellantriebe support
 *  @file		feldbus_stellantriebe.h
 *  @date		17.11.2013
 *  @author		Martin Oemus <martin@oemus.net>
 *
 * This module implements support for the TURAG Feldbus für Stellantriebe device protocol. 
 * This device protocol was designed to meet the communication requirements of 
 * usual mechanical actors. 
 * 
 * As a main feature it is possible to define a table
 * of values within the device's firmware which is called command set. 
 * This table is used by the protocol implementation to enable 
 * read and or write access to these values. For each entry of the command set
 * you can adjust the length, write access and a factor which needs to be applied to 
 * the returned raw value to get a value of the correct physical dimension.
 * This is useful to avoid expensive floating point operations in potentially 
 * slow devices. The different possible configuration options are defined in the protocol
 * specification header. The actual location in memory of an entry of the command
 * set is given with a void pointer to the corresponding address.
 * Generally the entries of the command set are supposed to have a meaning that
 * justifies them to be handled as floating point values (values with a physical 
 * representation of some kind). However by setting
 * the factor to 0.0f you can indicate that the value should rather be seen
 * as a control value.
 * 
 * Additionally it is possible to supply a human-understandable description
 * for each value which needs to be supplied in a separate array of strings.
 *
 * You should be aware of the fact that communication-caused access to the values
 * of the command set does always happen within interrupt context. This has 2 implications:
 *  - for read-only values you need to protect every write access that happens within the device's 
 *    firmware in main context to this value by disabling all interrupts before the actual write.
 *    if your device runs on an architecture whose register size is smaller than the value in question.
 *  - for writable values you additionally need to declare the variable as volatile.
 * 
 * 
 *
 */
#ifndef TINA_FELDBUS_SLAVE_FELDBUS_STELLANTRIEBE_H_
#define TINA_FELDBUS_SLAVE_FELDBUS_STELLANTRIEBE_H_

#include <tina/tina.h>
#include "feldbus.h"
#include <feldbus_config.h>


#if (TURAG_FELDBUS_DEVICE_PROTOCOL==TURAG_FELDBUS_DEVICE_PROTOCOL_STELLANTRIEBE) || defined(DOXYGEN)

typedef struct {
	/// pointer to the actual value
    void* value;  
	/// is the value allowed to be altered by the host?
    uint8_t write_access;
	/// length of the value
    uint8_t length;
	/// factor that should be applied to the returned values by the host. can be set
	/// to 0.0f to indicate that the value is a control value (not floating point)
    float factor;
} feldbus_stellantriebe_command_t;


/**
 * Sets up this module.
 *
 * This function calls turag_feldbus_slave_init(),
 * so you should not do this again.
 * 
 * @param command_set pointer to array containing the command set definition of the device
 * @param command_names pointer to array of strings describing the command set
 * @param command_set_length length of the command set
 */
void turag_feldbus_stellantriebe_init(
    feldbus_stellantriebe_command_t* command_set, 
    char** command_names, 
    uint8_t command_set_length);

/** 
 * This function is called when the value that belongs to this key
 * was externally changed.
 * 
 * You can use this function to start additional actions that are connected
 * to a change of a certain value. 
 * 
 * At the time this function is called the value is already updated. As of now 
 * there is no way to access the former value unless any form of buffering is done
 * manually.
 * 
 * If you don't nedd this functionality you can simply leave the function body empty.
 * 
 * @param key		
 */
extern void turag_feldbus_stellantriebe_value_changed(uint8_t key);


/**
 * This function is called upon the receiption of packages that are not handled by the 
 * Stellantriebe protocol.
 * 
 * If you don't nedd this functionality you can simply leave the function body empty.
 */
extern uint8_t turag_feldbus_stellantriebe_process_package(uint8_t* message, uint8_t message_length, uint8_t* response);

#endif

#endif /* TINA_FELDBUS_SLAVE_FELDBUS_STELLANTRIEBE_H_ */
