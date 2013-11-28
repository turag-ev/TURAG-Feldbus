/**
 *  @brief		Implements slave side TURAG feldbus support
 *  @file		feldbus.h
 *  @date		06.11.2013
 *  @author		Martin Oemus <martin@oemus.net>
 *
 * @section TURAG Feldbus base support
 * This header implements generic support for TURAG feldbus compatible slave devices.
 *
 * This header requires the presence of a device-specific header with the name "feldbus_config.h"
 * containing the following definitions:
 * - MY_ADDR
 * - TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE
 * - TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE
 * - TURAG_FELDBUS_DEVICE_TYPE_ID
 * - TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
 * - TURAG_FELDBUS_DEVICE_PROTOCOL
 * It should also include a device protocol header.
 *
 * This header should not be included directly but rather by the implementation of the desired device protocol.
 *
 * To be able make use of this module you need to make sure that every required function is present - required hardware functions
 * you need to define yourself while the device protocol should in most cased implement all required functionality.
 */
#ifndef TINA_FELDBUS_SLAVE_FELDBUS_H_
#define TINA_FELDBUS_SLAVE_FELDBUS_H_

#include <tina/tina.h>
#include <feldbus_config.h>

/** @name Required hardware functions
 *  hardware interfacing functions that are used by this module
 *  and need to be defined for the target platform
 */
///@{

extern void turag_feldbus_slave_rts_off(void);
extern void turag_feldbus_slave_rts_on(void);

extern void turag_feldbus_slave_activate_dre_interrupt(void);
extern void turag_feldbus_slave_deactivate_dre_interrupt(void);
extern void turag_feldbus_slave_activate_rx_interrupt(void);
extern void turag_feldbus_slave_deactivate_rx_interrupt(void);
extern void turag_feldbus_slave_activate_tx_interrupt(void);
extern void turag_feldbus_slave_deactivate_tx_interrupt(void);

/**
 * Starts some component that is able to call turag_feldbus_slave_receive_timeout_occured.
 * The timeout is specified in the protocol definition. Whenever the function is called
 * it is required that the timer starts counting from the start regardless of its previous state.
 */
extern void turag_feldbus_slave_start_receive_timeout(void);

/**
 * Transmits on byte over the bus.
 * @param byte Data to send
 */
extern void turag_feldbus_slave_transmit_byte(uint8_t byte);
///@}


/** @name Required device protocol functions
 *  functions that are used by this module
 *  and should be defined by the device protocol implementation
 */
///@{

/**
 * This function gets called when the device received a package.
 * The response should be made as soon as possible.
 *
 * @param message			Buffer holding the received data
 * @param message_length	Size of received data
 * @param response			Buffer that can be filled with the response data.
 * @return					Number of bytes put in the response buffer. You must not return more than TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE - 1 bytes.
 */
extern uint8_t turag_feldbus_slave_process_package(uint8_t* message, uint8_t message_length, uint8_t* response);

/**
 * This function gets called when the device received a broadcast package.
 *
 * @param message			Buffer holding the received data
 * @param message_length	Size of received data
 */
extern void turag_feldbus_slave_process_broadcast(uint8_t* message, uint8_t message_length);
///@}


/** @name Device protocol interface
 *  functions provided by this module
 *  and the device protocol implementation
 *  is supposed to make use of
 */
///@{

/**Initialize TURAG Feldbus support
 *
 * Call this function after initializing the UART peripheral
 * with the correct baudrate and after enabling transmitter and receiver.
 *
 * If your device does not have a bootloader, you should call this function
 * only after making sure that no device is being programmed.
 */
void turag_feldbus_slave_init(void);
///@}


/** @name Hardware interface
 *  functions provided by this module
 *  which should be called by the hardware driver
 */
///@{
// ---------------------------------------------------
// functions provided by this module
// which should be called by the hardware driver
// ---------------------------------------------------
/**
 * Receive-Complete-Interrupt Interface.
 *
 * Call this function from the Receive-Complete-Interrupt on your system.
 * @param byte Received byte
 */
void turag_feldbus_slave_byte_received(uint8_t byte);

/**
 * Data-Register-Empty-Interrupt-Interface.
 *
 * Call this function from the Data-Register-Empty-Interrupt on your system.
 */
void turag_feldbus_slave_ready_to_transmit(void);

/**
 * Transmit-Complete-Interrupt-Interface.
 *
 * Call this function from the Transmit-Complete-Interrupt on your system.
 */
void turag_feldbus_slave_transmission_complete(void);

/** Timeout occured.
 *
 * Call this function when the receive timeout has occured.
 *
 * If a timer is used to provide this functionality don't forget to deactivate the interrupt
 * before calling this function to avoid additional interrupts because of an overflowing timer.
 */
void turag_feldbus_slave_receive_timeout_occured(void);

///@}

// debugging functions
#if defined(TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED) || defined(DOXYGEN)
/** @name Debug-Functions
 *  Only available if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED is defined.
 *
 *  These functions can be used to print conventional
 *  debug messages in between standard communication data for debugging purposes only.
 */
///@{
	void print_text(const char*);	// print some text
	void print_char(uint8_t);			// print one byte in hexadecimal format
	void print_short(uint16_t);		// print two-byte value in hexadecimal format
	void print_sshort(int16_t x);
	void print_long(uint32_t x);
	void print_short_d(int16_t x);
	void print_slong(int32_t);
	void print_sshort_nn(int16_t x);
	void print_short_nn(uint16_t x);
///@}
#else
# 	define print_text(x)
# 	define print_char(x)
# 	define print_short(x)
# 	define print_sshort(x)
# 	define print_long(x)
# 	define print_short_d(x)
# 	define print_slong(x)
# 	define print_sshort_nn(x)
# 	define print_short_nn(x)
#endif


#endif /* TINA_FELDBUS_SLAVE_FELDBUS_H_ */
