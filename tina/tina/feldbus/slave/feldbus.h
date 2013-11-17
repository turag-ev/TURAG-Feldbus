/**
 *  @brief		Implements slave side TURAG feldbus support
 *  @file		feldbus.h
 *  @date		06.11.2013
 *  @author		Martin Oemus <martin@oemus.net>
 *
 * @section TURAG Feldbus base support
 */
#ifndef TINA_FELDBUS_SLAVE_FELDBUS_H_
#define TINA_FELDBUS_SLAVE_FELDBUS_H_

#include <tina/tina.h>
#include <feldbus_config.h>



#define TURAG_FELDBUS_PROTOCOL_VERSION TURAG_FELDBUS_PROTOCOL_VERSION_1_1

// ---------------------------------------------------
// hardware interfacing functions that are used by this module
// and need to be defined for the target platform
// ---------------------------------------------------

extern void turag_feldbus_slave_rts_off();
extern void turag_feldbus_slave_rts_on();

extern void turag_feldbus_slave_activate_dre_interrupt();
extern void turag_feldbus_slave_deactivate_dre_interrupt();
extern void turag_feldbus_slave_activate_rx_interrupt();
extern void turag_feldbus_slave_deactivate_rx_interrupt();
extern void turag_feldbus_slave_activate_tx_interrupt();
extern void turag_feldbus_slave_deactivate_tx_interrupt();

/**
 * Starts some component that is able to call turag_feldbus_slave_receive_timeout_occured.
 * The timeout is specified in the protocol definition. Whenever the function is called
 * it is required that the timer starts counting from the start regardless of its previous state.
 */
extern void turag_feldbus_slave_start_receive_timeout();

/**
 * Transmits on byte over the bus.
 * @param byte Data to send
 */
extern void turag_feldbus_slave_transmit_byte(uint8_t byte);




// ---------------------------------------------------
// functions that are used by this module
// and should be defined by the device protocol
// implementation
// ---------------------------------------------------
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




// ---------------------------------------------------
// functions provided by this module
// and the device protocol implementation
// is supposed to make use of
// ---------------------------------------------------
/**Initialize TURAG Feldbus support
 *
 * Call this function after initializing the UART peripheral
 * with the correct baudrate and after enabling transmitter and receiver.
 *
 * If your device does not have a bootloader, you should call this function
 * only after making sure that no device is being programmed.
 */
void turag_feldbus_slave_init();

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
void turag_feldbus_slave_ready_to_transmit();

/**
 * Transmit-Complete-Interrupt-Interface.
 *
 * Call this function from the Transmit-Complete-Interrupt on your system.
 */
void turag_feldbus_slave_transmission_complete();

/** Timeout occured.
 *
 * Call this fucntion when the receive timeout has occured.
 */
void turag_feldbus_slave_receive_timeout_occured();



// debugging functions
#ifdef TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED || DOXYGEN
/** @name Debug-Functions
 *  Only available if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED is defined.
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
