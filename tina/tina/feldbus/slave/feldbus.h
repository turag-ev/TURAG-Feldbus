/**
 *  @brief		Implements slave side %TURAG feldbus support
 *  @file		feldbus.h
 *  @date		06.11.2013
 *  @author		Martin Oemus <martin@oemus.net>
 *  @see 		\ref feldbus-slave
 *  @ingroup feldbus-slave-base
 */


/** 
 * @defgroup feldbus-slave-base Basis-Implementierung
 * @ingroup feldbus-slave
 * 
 * TURAG-Feldbus-Basisimplementierung. 
 * 
 * feldbus_config.h ist eine Beispielkonfigurationsdatei, die nicht direkt includiert werden darf.
 *
 * \see \ref feldbus-slave
 * 
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

/**
 * This function should implement all required hardware initializations (e.g. UART peripheral).
 */
extern void turag_feldbus_hardware_init(void);

/**
 * This function should implement all logic that is required to disable the rs485 line driver thus releasing the bus.
 */
extern void turag_feldbus_slave_rts_off(void);

/**
 * This function should implement all logic that is required to enable the rs485 line driver thus claiming 
 * the bus for active driving.
 */
extern void turag_feldbus_slave_rts_on(void);

/**
 * This function should implement all logic that is required to 
 * activate the data register empty interrupt of the UART peripheral.
 */
extern void turag_feldbus_slave_activate_dre_interrupt(void);

/**
 * This function should implement all logic that is required to 
 * deactivate the data register empty interrupt of the UART peripheral.
 */
extern void turag_feldbus_slave_deactivate_dre_interrupt(void);

/**
 * This function should implement all logic that is required to 
 * activate the receive interrupt of the UART peripheral.
 */
extern void turag_feldbus_slave_activate_rx_interrupt(void);

/**
 * This function should implement all logic that is required to 
 * deactivate the receive interrupt of the UART peripheral.
 */
extern void turag_feldbus_slave_deactivate_rx_interrupt(void);

/**
 * This function should implement all logic that is required to 
 * activate the transmission complete interrupt of the UART peripheral.
 */
extern void turag_feldbus_slave_activate_tx_interrupt(void);

/**
 * This function should implement all logic that is required to 
 * deactivate the transmission complete interrupt of the UART peripheral.
 */
extern void turag_feldbus_slave_deactivate_tx_interrupt(void);

/**
 * This function should start some component that is able to call turag_feldbus_slave_receive_timeout_occured.
 * The timeout is specified in the protocol definition. Whenever the function is called
 * it is required that the timer starts counting from the start regardless of its previous state.
 * 
 * After being enabled it should trigger the call of turag_feldbus_slave_receive_timeout_occured only once
 * and deactivate itself afterwards.
 */
extern void turag_feldbus_slave_start_receive_timeout(void);

/**
 * This function should implement some functionality that ensures code
 * being executed without being interrupted.
 * 
 * Usually this means to deactivate the global interrupt flag.
 * 
 * This fucntion is always followed by a call to turag_feldbus_slave_end_interrupt_protect.
 */
extern void turag_feldbus_slave_begin_interrupt_protect(void);

/**
 * This function follows on a call to turag_feldbus_slave_begin_interrupt_protect.
 * 
 * It should usually implement the disabling of the global interrupt flag.
 */
extern void turag_feldbus_slave_end_interrupt_protect(void);

/**
 * Transmits on byte over the bus.
 * @param byte Data to send
 */
extern void turag_feldbus_slave_transmit_byte(uint8_t byte);
///@}

/// Rückgabewert für turag_feldbus_slave_process_package(), mit dem angezeigt wird,
/// dass kein Antwortpaket zurückgesendet werden soll.
#define TURAG_FELDBUS_IGNORE_PACKAGE		0xff

/** @name Required device protocol functions
 *  functions that are used by this module
 *  and should be defined by the device protocol implementation
 */
///@{

/**
 * This function gets called when the device received a package.
 * The response should be made as soon as possible. 
 * 
 * The length of the response package is controlled by the return
 * value. It is possible not to reply to a package at all by returning \ref TURAG_FELDBUS_IGNORE_PACKAGE. 
 * If you return 0, an empty response package consisting only of address and
 * checksum will be generated.
 * 
 * The data held in the message buffer contain the received package
 * stripped of address and checksum. message_length is guaranteed to be >= 1 as a consequence 
 * ping-requests (empty package) being handled by this implementation.
 * 
 * This function is never called in interrupt context.
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
 * As broadcasts can never generate a response package this function does neither have a return type
 * nor pointer-type arguments to generate any kind of feedback.
 *
 * This function is never called in interrupt context.
 * 
 * @param message			Buffer holding the received data
 * @param message_length	Size of received data
 * @param protocol_id		protocol ID to determine for which devices the broadcast was meant
 */
extern void turag_feldbus_slave_process_broadcast(uint8_t* message, uint8_t message_length, uint8_t protocol_id);
///@}


/** @name Device protocol interface
 *  functions provided by this module
 *  and the device protocol implementation
 *  is supposed to make use of
 */
///@{

/**Initialize TURAG Feldbus support
 *
 * Call this function to set up the TURAG Feldbus module.
 *
 * If your device does not have a bootloader, you should call this function
 * only after making sure that no device is being programmed.
 */
void turag_feldbus_slave_init(void);

/**
 * This function needs to be called continuously from the device's
 * main loop. It handles all advanced communication logic.
 * 
 */
TURAG_INLINE void turag_feldbus_do_processing(void);


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
TURAG_INLINE void turag_feldbus_slave_byte_received(uint8_t byte);

/**
 * Data-Register-Empty-Interrupt-Interface.
 *
 * Call this function from the Data-Register-Empty-Interrupt on your system.
 */
TURAG_INLINE void turag_feldbus_slave_ready_to_transmit(void);

/**
 * Transmit-Complete-Interrupt-Interface.
 *
 * Call this function from the Transmit-Complete-Interrupt on your system.
 */
TURAG_INLINE void turag_feldbus_slave_transmission_complete(void);

/** Timeout occured.
 *
 * Call this function when the receive timeout has occured.
 *
 * If a timer is used to provide this functionality don't forget to deactivate the interrupt
 * before calling this function to avoid additional interrupts because of an overflowing timer.
 */
TURAG_INLINE void turag_feldbus_slave_receive_timeout_occured(void);

///@}

// debugging functions
#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED || defined(__DOXYGEN__)
/** @name Debug-Functions
 *  Only available if \ref TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED is defined.
 *
 *  These functions can be used to print conventional
 *  debug messages in between standard communication data for debugging purposes only.
 */
///@{
	/// print some text
	void print_text(const char*);	
	
	/// print signed char in hexadecimal format
	void print_char(uint8_t);	
	
	/// print unsigned short value in hexadecimal format
	void print_short(uint16_t);
	
	/// print signed short value in hexadecimal format
	void print_sshort(int16_t x);
	
	/// print unsigned long value in hexadecimal format
	void print_long(uint32_t x);
	
	/// print signed short value in decimal format
	void print_short_d(int16_t x);
	
	/// print signed long value in hexadecimal format
	void print_slong(int32_t);
	
	/// print signed short value in hexadecimal format without trailuing newline characters
	void print_sshort_nn(int16_t x);
	
	/// print unsigned short value in hexadecimal format without trailuing newline characters
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
	
	
// hide some uninteresting stuff from documentation
#if (!defined(__DOXYGEN__))

#include <tina/crc/xor_checksum.h>
#include <tina/crc/crc8_icode/crc8_icode.h>
#include <stdlib.h>
#include <string.h>

	
#ifndef MY_ADDR
# error MY_ADDR must be defined
#endif
#ifndef TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE
# error TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE must be defined
#endif
#ifndef TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE
# error TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE must be defined
#else
# if TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE < 20
#  error TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE < 20 and needs to be bigger.
# endif
#endif
#ifndef TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE
# error TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE must be defined
#endif
#ifndef TURAG_FELDBUS_DEVICE_PROTOCOL
# error TURAG_FELDBUS_DEVICE_PROTOCOL must be defined
#endif
#ifndef TURAG_FELDBUS_DEVICE_TYPE_ID
# error TURAG_FELDBUS_DEVICE_TYPE_ID must be defined
#endif
#ifndef TURAG_FELDBUS_DEVICE_NAME
# define TURAG_FELDBUS_DEVICE_NAME "unnamed device"
#endif
#ifndef TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
# error TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED must be defined
#else
# if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
#  warning TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED = 1
# endif
#endif
#ifndef TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH
# error TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH must be defined
#else
# if (TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH != 1 ) && (TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH != 2)
#  error TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH must be 1 or 2
# endif
#endif

typedef struct {
	uint8_t rxbuf[TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE];
	uint8_t txbuf[TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE];
	uint8_t rxbuf_main[TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE];
	uint8_t length;
	uint8_t index;
	volatile uint8_t rx_length;
	uint8_t overflow;
	uint8_t chksum;
#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
	volatile uint8_t transmission_active;
	uint8_t chksum_required;
#endif
} turag_feldbus_slave_uart_t;

extern turag_feldbus_slave_uart_t turag_feldbus_slave_uart;
extern uint8_t turag_feldbus_slave_name_length;
extern char* turag_feldbus_slave_name;


TURAG_INLINE void start_transmission(void) {
#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
		turag_feldbus_slave_uart.transmission_active = 1;
#endif
	turag_feldbus_slave_deactivate_rx_interrupt();
	turag_feldbus_slave_rts_on();
	turag_feldbus_slave_activate_dre_interrupt();
}

#endif // (!defined(DOXYGEN))

	
TURAG_INLINE void turag_feldbus_slave_byte_received(uint8_t data) {
	// By clearing rx_length turag_feldbus_do_processing() will no longer
	// try to copy packages from the in-buffer which will be overwritten now anyway.
	turag_feldbus_slave_uart.rx_length = 0;
	turag_feldbus_slave_uart.rxbuf[turag_feldbus_slave_uart.index] = data;
	++turag_feldbus_slave_uart.index;

	if (turag_feldbus_slave_uart.index >= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE) {
		turag_feldbus_slave_uart.index = 0;
		turag_feldbus_slave_uart.overflow = 1;
	}

	// activate timer to recognize end of command
	turag_feldbus_slave_start_receive_timeout();
}


TURAG_INLINE void turag_feldbus_slave_ready_to_transmit() {
	if (turag_feldbus_slave_uart.index == turag_feldbus_slave_uart.length) {
#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
		if (turag_feldbus_slave_uart.chksum_required)
#endif
			// all data bytes are processed, now transmit checksum
			turag_feldbus_slave_transmit_byte(turag_feldbus_slave_uart.chksum);

		turag_feldbus_slave_deactivate_dre_interrupt();
		turag_feldbus_slave_activate_tx_interrupt();
	} else {
		turag_feldbus_slave_transmit_byte(turag_feldbus_slave_uart.txbuf[turag_feldbus_slave_uart.index]);
		++turag_feldbus_slave_uart.index;
	}
}

TURAG_INLINE void turag_feldbus_slave_transmission_complete() {
	// turn off transmitter and release bus, turn on receiver
	turag_feldbus_slave_rts_off();
	turag_feldbus_slave_deactivate_tx_interrupt();
	turag_feldbus_slave_activate_rx_interrupt();

	turag_feldbus_slave_uart.index = 0;
#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
	turag_feldbus_slave_uart.transmission_active = 0;
#endif
}


TURAG_INLINE void turag_feldbus_slave_receive_timeout_occured() {
#if TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 1	
# if TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE
	if ((turag_feldbus_slave_uart.rxbuf[0] == MY_ADDR || turag_feldbus_slave_uart.rxbuf[0] == TURAG_FELDBUS_BROADCAST_ADDR) &&
			!turag_feldbus_slave_uart.overflow && 
			turag_feldbus_slave_uart.index > 1)
# else
	if (turag_feldbus_slave_uart.rxbuf[0] == MY_ADDR  &&
			!turag_feldbus_slave_uart.overflow && 
			turag_feldbus_slave_uart.index > 1)
# endif
#elif TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 2
# if TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE
	if (((turag_feldbus_slave_uart.rxbuf[0] == (MY_ADDR & 0xff) && turag_feldbus_slave_uart.rxbuf[1] == (MY_ADDR >> 8)) || 
		(turag_feldbus_slave_uart.rxbuf[0] == (TURAG_FELDBUS_BROADCAST_ADDR_2 & 0xff) && turag_feldbus_slave_uart.rxbuf[1] == (TURAG_FELDBUS_BROADCAST_ADDR_2 >> 8))) &&
			!turag_feldbus_slave_uart.overflow && 
			turag_feldbus_slave_uart.index > 1)
# else
	if ((turag_feldbus_slave_uart.rxbuf[0] == (MY_ADDR & 0xff) && turag_feldbus_slave_uart.rxbuf[1] == (MY_ADDR >> 8))  &&
			!turag_feldbus_slave_uart.overflow && 
			turag_feldbus_slave_uart.index > 1)
# endif
#endif		
	{
		// package ok -> signal start of processing in main loop
		turag_feldbus_slave_uart.rx_length = turag_feldbus_slave_uart.index;
	}

	turag_feldbus_slave_uart.index = 0;
	turag_feldbus_slave_uart.overflow = 0;
}


TURAG_INLINE void turag_feldbus_do_processing(void) {
	// One might think it is unnecessary to disable interrupts just for
	// reading rx_length. But because rx_length is volatile the following
	// scenario is quite possible: we copy the the value of rx_length from memory
	// to our register. Then we receive a byte and turag_feldbus_slave_byte_received() is called.
	// This function is executed in interrupt context and clears rx_length. Then we continue here
	// and evaluate the now invalid value of rx_length which was buffered
	// in our register. For this reason we need to disable all interrupts before
	// reading rx_length.
	turag_feldbus_slave_begin_interrupt_protect();
	
	if (turag_feldbus_slave_uart.rx_length == 0) {
		turag_feldbus_slave_end_interrupt_protect();
		return;
	}

	uint8_t length = turag_feldbus_slave_uart.rx_length;
	memcpy(turag_feldbus_slave_uart.rxbuf_main, turag_feldbus_slave_uart.rxbuf, length);
	turag_feldbus_slave_uart.rx_length = 0;
	turag_feldbus_slave_end_interrupt_protect();

	// if we are here, we have a package (with length>1 that is adressed to us) safe in our buffer
	// and we can start working on it
	

	// calculate checksum
#if TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE == TURAG_FELDBUS_CHECKSUM_XOR
	if (!xor_checksum_check(turag_feldbus_slave_uart.rxbuf_main, length - 1, turag_feldbus_slave_uart.rxbuf_main[length - 1]))
#elif TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE == TURAG_FELDBUS_CHECKSUM_CRC8_ICODE
	if (!turag_crc8_check(turag_feldbus_slave_uart.rxbuf_main, length - 1, turag_feldbus_slave_uart.rxbuf_main[length - 1]))
#endif
	{
		return;
	}

	// The address is already checked in turag_feldbus_slave_receive_timeout_occured(). 
	// Thus the second check is only necessary
	// to distinguish broadcasts from regular packages.
#if TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE
	// message addressed to me?
# if TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 1	
	if (turag_feldbus_slave_uart.rxbuf_main[0] == MY_ADDR) {
# elif TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 2
	if (turag_feldbus_slave_uart.rxbuf_main[0] == (MY_ADDR & 0xff) && turag_feldbus_slave_uart.rxbuf_main[1] == (MY_ADDR >> 8)) {	
# endif	
#endif
		if (length == 1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH) {
			// we received a ping request -> respond with empty packet (address + checksum)
			turag_feldbus_slave_uart.length = TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH;
		} else if (turag_feldbus_slave_uart.rxbuf_main[TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] == 0) {
			// first data byte is zero -> reserved packet
			if (length == 2 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH) {
				// received a debug packet
				turag_feldbus_slave_uart.txbuf[0 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = TURAG_FELDBUS_DEVICE_PROTOCOL;
				turag_feldbus_slave_uart.txbuf[1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = TURAG_FELDBUS_DEVICE_TYPE_ID;
				turag_feldbus_slave_uart.txbuf[2 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE;
				turag_feldbus_slave_uart.txbuf[3 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE;
				turag_feldbus_slave_uart.txbuf[4 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = turag_feldbus_slave_name_length;
				turag_feldbus_slave_uart.length = 5 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH;
			} else if (turag_feldbus_slave_uart.rxbuf_main[1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] == 0 && length == 3 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH) {
				// return device name
				memcpy(turag_feldbus_slave_uart.txbuf + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH, turag_feldbus_slave_name, turag_feldbus_slave_name_length);
				turag_feldbus_slave_uart.length = turag_feldbus_slave_name_length + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH;    
			} else {
				// unhandled reserved packet
				return;
			}
		} else {
			// received some other packet --> let somebody else process it
			turag_feldbus_slave_uart.length = turag_feldbus_slave_process_package(
				turag_feldbus_slave_uart.rxbuf_main + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH, 
				length - (1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH), 
				turag_feldbus_slave_uart.txbuf + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH;
		}
		
		// this happens if the device protocol or the user code returned TURAG_FELDBUS_IGNORE_PACKAGE.
		if (turag_feldbus_slave_uart.length == (uint8_t)(TURAG_FELDBUS_IGNORE_PACKAGE + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH)) {
			return;
		}

#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
		turag_feldbus_slave_uart.chksum_required = 1;
# if TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 1
	turag_feldbus_slave_uart.txbuf[0] = TURAG_FELDBUS_MASTER_ADDR|MY_ADDR;
# elif TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 2
	turag_feldbus_slave_uart.txbuf[0] = (TURAG_FELDBUS_MASTER_ADDR_2|MY_ADDR) & 0xff;
	turag_feldbus_slave_uart.txbuf[1] = (TURAG_FELDBUS_MASTER_ADDR_2|MY_ADDR) >> 8;
# endif
#endif
		
		// calculate correct checksum and initiate transmission
#if TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE == TURAG_FELDBUS_CHECKSUM_XOR
		turag_feldbus_slave_uart.chksum = xor_checksum_calculate(turag_feldbus_slave_uart.txbuf, turag_feldbus_slave_uart.length);
#elif TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE == TURAG_FELDBUS_CHECKSUM_CRC8_ICODE
		turag_feldbus_slave_uart.chksum = turag_crc8_calculate(turag_feldbus_slave_uart.txbuf, turag_feldbus_slave_uart.length);
#endif
		
		start_transmission();

	// not every device protocol requires broadcasts, so we can save a few bytes here
#if TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE
	} else {
		if (length == 1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH) {
			// compatibility mode to support deprecated Broadcasts without protocol-ID
			turag_feldbus_slave_process_broadcast(0, 0, TURAG_FELDBUS_DEVICE_PROTOCOL_LOKALISIERUNGSSENSOREN);
		} else if (turag_feldbus_slave_uart.rxbuf_main[TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] == TURAG_FELDBUS_BROADCAST_TO_ALL_DEVICES || 
				turag_feldbus_slave_uart.rxbuf_main[TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] == TURAG_FELDBUS_DEVICE_PROTOCOL) {
			// otherwise process only the correct broadcasts
			turag_feldbus_slave_process_broadcast(
				turag_feldbus_slave_uart.rxbuf_main + 1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH, 
				length - (2 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH), 
				turag_feldbus_slave_uart.rxbuf_main[TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH]);
		}
	}
#endif
}

	



#endif /* TINA_FELDBUS_SLAVE_FELDBUS_H_ */
