/**
 *  @brief		Implements slave side TURAG feldbus support
 *  @file		feldbus.h
 *  @date		06.11.2013
 *  @author		Martin Oemus <martin@oemus.net>
 * 
 */
#ifndef TINA_FELDBUS_SLAVE_FELDBUS_H_
#define TINA_FELDBUS_SLAVE_FELDBUS_H_

#include <tina/tina.h>
#include <feldbus_config.h>


// ---------------------------------------------------
// hardware interfaceing functions that are used by this module
// and need to be defined for the target platform
// ---------------------------------------------------
extern void turag_feldbus_slave_prepare_receiving();
extern void turag_feldbus_slave_prepare_transmitting();
extern void turag_feldbus_slave_start_receive_timeout();
extern void turag_feldbus_slave_transmit_byte(uint8_t byte);


// ---------------------------------------------------
// functions that are used by this module
// and should be defined by the device protocol
// implementation
// ---------------------------------------------------
extern uint8_t turag_feldbus_slave_process_package(uint8_t* message, uint8_t message_length, uint8_t* response);
extern void turag_feldbus_slave_process_broadcast(uint8_t* message, uint8_t message_length);




// ---------------------------------------------------
// functions provided by this module
// and the device protocol implementation
// is supposed to make use of
// ---------------------------------------------------
void turag_feldbus_slave_init();

// ---------------------------------------------------
// functions provided by this module
// which should be called by the hardware driver
// ---------------------------------------------------
void turag_feldbus_slave_byte_received(uint8_t byte);
void turag_feldbus_slave_ready_to_transmit();
void turag_feldbus_slave_transmission_complete();
void turag_feldbus_slave_receive_timeout_occured();


#endif /* TINA_FELDBUS_SLAVE_FELDBUS_H_ */
