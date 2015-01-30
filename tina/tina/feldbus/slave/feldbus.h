/**
 *  @brief		Implementiert slave-side %TURAG feldbus support
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
 * Auf einem Atmega88 belegt die Basis-Implementierung minimal ca 1 KB Flash 
 * zzgl. die Größe der Lookup-Table wenn eine tabellenbasierte 
 * CRC-Checksumme verwendet wird. Der RAM-Verbrauch richtet sich
 * nach der Größe des Übertragungspuffers. Auch die evt. CRC-Tabelle
 * belegt RAM.
 * 
 * \see \ref feldbus-slave
 * 
 * @section feldbus-slave-base-config Konfiguration
 * 
 * Siehe feldbus_config.h.
 * 
 * feldbus_config.h ist eine Beispielkonfigurationsdatei, die nicht direkt includiert werden darf.
 *
 * 
 */

#ifndef TINA_FELDBUS_SLAVE_FELDBUS_H_
#define TINA_FELDBUS_SLAVE_FELDBUS_H_

#include <tina/tina.h>
#include <tina/helper/static_assert.h>
#include <feldbus_config.h>
#include <tina/crc/xor_checksum.h>
#include <tina/crc/crc_checksum.h>
#include <stdlib.h>
#include <string.h>


// hide some uninteresting stuff from documentation
#if (!defined(__DOXYGEN__))
	
#ifndef MY_ADDR
# error MY_ADDR must be defined
#endif
#ifndef TURAG_FELDBUS_DEVICE_NAME
# define TURAG_FELDBUS_DEVICE_NAME "unnamed device"
#endif
#ifndef TURAG_FELDBUS_DEVICE_VERSIONINFO
# define TURAG_FELDBUS_DEVICE_VERSIONINFO __DATE__ " " __TIME__
#endif
#ifndef TURAG_FELDBUS_DEVICE_PROTOCOL
# error TURAG_FELDBUS_DEVICE_PROTOCOL must be defined
#endif
#ifndef TURAG_FELDBUS_DEVICE_TYPE_ID
# error TURAG_FELDBUS_DEVICE_TYPE_ID must be defined
#endif
#ifndef TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH
# error TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH must be defined
#else
# if (TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH != 1 ) && (TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH != 2)
#  error TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH must be 1 or 2
# endif
#endif
#ifndef TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE
# error TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE must be defined
#endif
#ifndef TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE
# error TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE must be defined
#endif
#ifndef TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
# error TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED must be defined
#else
# if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
#  warning TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED = 1
# endif
#endif
#ifndef TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE
# error TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE must be defined
#endif
#ifndef TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY
# error TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY must be defined
#else
# if (TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY<0) || (TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY>65535)
#  error TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY must be within the range of 0-65535
# else
#  if TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY != 0 && !defined(TURAG_FELDBUS_SLAVE_CONFIG_USE_LED_CALLBACK)
#   error TURAG_FELDBUS_SLAVE_CONFIG_USE_LED_CALLBACK must be defined
#  endif
#  if TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY == 0 && !defined(TURAG_FELDBUS_SLAVE_CONFIG_USE_LED_CALLBACK)
#   define TURAG_FELDBUS_SLAVE_CONFIG_USE_LED_CALLBACK 0
#  endif
#  if TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY==0 && defined(TURAG_FELDBUS_SLAVE_CONFIG_USE_LED_CALLBACK) && TURAG_FELDBUS_SLAVE_CONFIG_USE_LED_CALLBACK != 0
#   warning TURAG_FELDBUS_SLAVE_CONFIG_USE_LED_CALLBACK == 1, but TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY == 0. Led flashing feature depends on uptime feature.
#  endif
# endif
#endif
#ifndef TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE
# error TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE must be defined
#endif


#if TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE > 65535
	typedef uint32_t FeldbusSize_t;
# if TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 1
#  define TURAG_FELDBUS_IGNORE_PACKAGE 0xffffffff
# elif TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 2
#  define TURAG_FELDBUS_IGNORE_PACKAGE 0xfffffffe
# else
#  error no option for address size
# endif
#elif TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE > 255
	typedef uint16_t FeldbusSize_t;
# if TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 1
#  define TURAG_FELDBUS_IGNORE_PACKAGE 0xffff
# elif TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 2
#  define TURAG_FELDBUS_IGNORE_PACKAGE 0xfffe
# else
#  error no option for address size
# endif
#else
	typedef uint8_t FeldbusSize_t;
# if TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 1
#  define TURAG_FELDBUS_IGNORE_PACKAGE 0xff
# elif TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 2
#  define TURAG_FELDBUS_IGNORE_PACKAGE 0xfe
# else
#  error no option for address size
# endif
#endif

#endif

/** @name Benötigte Hardware-Funktionen
 *  Hardware-Interface-Funktionen die von der Basis-Implementierung benutzt werden
 * und die für die Zielplattform bereitgestellt werden müssen.
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
 * This function is always followed by a call to turag_feldbus_slave_end_interrupt_protect().
 */
extern void turag_feldbus_slave_begin_interrupt_protect(void);

/**
 * This function follows on a call to turag_feldbus_slave_begin_interrupt_protect().
 * 
 * It should usually implement the enabling of the global interrupt flag.
 */
extern void turag_feldbus_slave_end_interrupt_protect(void);

/**
 * Diese Funktion wird in einem
 * einem charakteristischen Frequenzmuster aufgerufen, welches
 * vom Wert von \ref TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY
 * abhängt. Ist dieser Wert 0 oder ist 
 * \ref TURAG_FELDBUS_SLAVE_CONFIG_USE_LED_CALLBACK mit 0 konfiguriert,
 * so ist es nicht nötig, diese Funktion zu implementieren.
 */
extern void turag_feldbus_slave_toggle_led(void);

/**
 * Transmits on byte over the bus.
 * @param byte Data to send
 */
extern void turag_feldbus_slave_transmit_byte(uint8_t byte);
///@}

/** @name Zu benutzende Hardware-Funktionen
 *  Funktionen, die vom Hardwaretreiber des Gerätes
 * aufgerufen werden müssen.
 */
///@{
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

/** 
 * Timeout occured.
 *
 * Call this function when the receive timeout has occured.
 *
 * If a timer is used to provide this functionality don't forget to deactivate it
 * before calling this function to avoid additional interrupts because of an overflowing timer.
 */
TURAG_INLINE void turag_feldbus_slave_receive_timeout_occured(void);

/**
 * Inkrementiert den Uptime-Counter.
 * 
 * Diese Funktion muss periodisch mit einer konstanten Frequenz aufgerufen werden.
 * Zusätzlich muss \ref TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY mit der gleichen
 * Frequenz definiert werden.
 * 
 * Dies gilt nicht, wenn \ref TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY auf
 * 0 definiert wird: dies zeigt an, dass das Feature nicht verfügbar ist.
 */
#if defined(__DOXYGEN__)
TURAG_INLINE void turag_feldbus_slave_increase_uptime_counter(void);
#endif

///@}


/// \brief Rückgabewert für turag_feldbus_slave_process_package(), mit dem angezeigt wird,
/// dass kein Antwortpaket zurückgesendet werden soll.
#if defined(__DOXYGEN__)
# define TURAG_FELDBUS_IGNORE_PACKAGE		<configuration-dependend>
#endif

/// \brief Typ, der für Offsets und Längen benutzt wird.
/// Größe des tatsächlichen Integer-Typs hängt von der Größe
/// des Puffers ab (\ref TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE)
#if defined(__DOXYGEN__)
typedef int FeldbusSize_t;
#endif

/** @name Benötigte Protokoll-Interface-Funktionen
 *  Protokoll-Interface-Funktionen die von der Basis-Implementierung
 * aufgerufen werden und von der Firmware des Gerätes bereitgestellt
 * werden müssen.
 */
///@{

/**
 * @param[in] message			Buffer holding the received data
 * @param[in] message_length	Size of received data
 * @param[out] response			Buffer that can be filled with the response data.
 * @return						Number of bytes put in the response buffer. You must not return 
 * more than \ref TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE - \ref TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH bytes.
 * 
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
 * \note Diese Funktion wird stets im main-Kontext aufgerufen.
 *
 * @warning Keinesfalls dürfen in response mehr Daten geschrieben werden als 
 * \ref TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE - \ref TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH bytes.
 */
extern FeldbusSize_t turag_feldbus_slave_process_package(uint8_t* message, FeldbusSize_t message_length, uint8_t* response);


/**
 * @param[in] message			Puffer mit den empfangenen Daten-
 * @param[in] message_length	Länge des Puffers.
 * @param[in] protocol_id		Protokoll-ID, an die der Broadcast gerichtet ist.
 *
 * Diese Funktion wird aufgerufen, wenn das Gerät ein Broadcast-Paket empfängt. 
 * Dies ist nur dann der Fall, wenn die Protokoll-ID des Broadcasts der des Gerätes
 * entspricht oder 0 ist.
 * 
 * Da Antworten auf Braodcasts nicht zulässig sind, besitzt diese Funktion weder
 * einen Rückgabetyp, noch output-Argumente.
 *
 * \note Diese Funktion wird stets im main-Kontext aufgerufen.
 */
extern void turag_feldbus_slave_process_broadcast(uint8_t* message, FeldbusSize_t message_length, uint8_t protocol_id);
///@}


/** @name Zu benutzende Protokoll-Interface-Funktionen
 *  Funktionen, die von der Firmware des Gerätes aufgerufen
 * werden müssen.
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



// debugging functions
#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED || defined(__DOXYGEN__)
/** @name Debug-Functions
 * Diese Funktionen können verwendet werden, um Debugausgaben in Form
 * konventioneller Zeichenketten auf dem Bus auszugeben.
 * 
 * Soll eine der Debug-Funktionen von turag_feldbus_slave_process_package()
 * aus aufgerufen werden, kann das Makro TURAG_FELDBUS_DEBUG_SAFE() benutzt werden,
 * welches als Argument den kompletten Funktionsaufruf entgegennimmt.
 * 
 * \pre Nur verfügbar, wenn \ref TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED auf 1 definiert ist.
 * 
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
	
	/// print unsigned short value in hexadecimal format without trailing newline characters
	void print_short_nn(uint16_t x);
	
	/// Makro, von welchem eine Debug-Funktion umschlossen werden muss, wenn es von 
	/// turag_feldbus_slave_process_package() aus aufgerufen wird.
	#define TURAG_FELDBUS_DEBUG_SAFE(function) function; while (turag_feldbus_slave_uart.transmission_active)
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
		
typedef struct {
	uint8_t txbuf[TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE];
	uint8_t rxbuf[TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE];
	// holds the number of bytes in txbuf
	FeldbusSize_t transmitLength;
	// offset in txbuf
	FeldbusSize_t txOffset;
	// offset in rxBuf
	FeldbusSize_t rxOffset;
	// if not 0, then there is a package waiting for processsing
	volatile FeldbusSize_t rx_length;
	// overflow detected
	uint8_t overflow;
	// holds the calculated checksum
	uint8_t chksum;
#if TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE
	// package loss detected and counter must be increased
	uint8_t package_lost_flag;
	// overflow detected and counter must be increased
	uint8_t package_overflow_flag;
#endif
#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
	volatile uint8_t transmission_active;
	uint8_t chksum_required;
#endif
#if TURAG_FELDBUS_SLAVE_CONFIG_USE_LED_CALLBACK == 1
	volatile uint8_t toggleLedBlocked;
#endif
} turag_feldbus_slave_uart_t;

typedef struct {
	char* name;
	char* versioninfo;
#if TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE
	uint32_t packagecount_correct;
	uint32_t packagecount_buffer_overflow;
	uint32_t packagecount_lost;
	uint32_t packagecount_chksum_mismatch;
#endif
#if (TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY>0) && (TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY<=65535)
	uint32_t uptime_counter;
#endif
} turag_feldbus_slave_info_t;

extern turag_feldbus_slave_uart_t turag_feldbus_slave_uart;
extern turag_feldbus_slave_info_t turag_feldbus_slave_info;


TURAG_INLINE void start_transmission(void) {
#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
		turag_feldbus_slave_uart.transmission_active = 1;
#endif
		
	turag_feldbus_slave_uart.txOffset = 0;
		
	turag_feldbus_slave_deactivate_rx_interrupt();
	turag_feldbus_slave_rts_on();
	turag_feldbus_slave_activate_dre_interrupt();
}

#endif // (!defined(__DOXYGEN__))

	
TURAG_INLINE void turag_feldbus_slave_byte_received(uint8_t data) {
	// if at this point rx_length is not 0, obviously the last 
	// received package was not processed yet. This package
	// will be overwritten now and is lost.
	// Once rx_length is cleared, turag_feldbus_do_processing() will no longer
	// try to copy packages from the in-buffer.
	if (turag_feldbus_slave_uart.rx_length) {
		turag_feldbus_slave_uart.rx_length = 0;
#if TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE
		turag_feldbus_slave_uart.package_lost_flag = 1;
#endif
	}

	// accessing the buffer as an array is more effective than using a pointer
	// and increasing it.
	turag_feldbus_slave_uart.rxbuf[turag_feldbus_slave_uart.rxOffset] = data;
	++turag_feldbus_slave_uart.rxOffset;

	if (turag_feldbus_slave_uart.rxOffset >= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE) {
		turag_feldbus_slave_uart.rxOffset = 0;
		
#if TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE
		// We have a buffer overflow. If this happens for the 
		// first time for this package, we check the address.
		// If the package was for us, we increase the counter for 
		// package overflow.
		if (turag_feldbus_slave_uart.overflow == 0) {
# if TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 1	
#  if TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE
			if ((turag_feldbus_slave_uart.rxbuf[0] == MY_ADDR || turag_feldbus_slave_uart.rxbuf[0] == TURAG_FELDBUS_BROADCAST_ADDR))
#  else
			if (turag_feldbus_slave_uart.rxbuf[0] == MY_ADDR)
#  endif
# elif TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 2
#  if TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE
			if (((turag_feldbus_slave_uart.rxbuf[0] == (MY_ADDR & 0xff) && turag_feldbus_slave_uart.rxbuf[1] == (MY_ADDR >> 8)) || 
				(turag_feldbus_slave_uart.rxbuf[0] == (TURAG_FELDBUS_BROADCAST_ADDR_2 & 0xff) && turag_feldbus_slave_uart.rxbuf[1] == (TURAG_FELDBUS_BROADCAST_ADDR_2 >> 8))))
#  else
			if ((turag_feldbus_slave_uart.rxbuf[0] == (MY_ADDR & 0xff) && turag_feldbus_slave_uart.rxbuf[1] == (MY_ADDR >> 8)))
#  endif
# endif		
			{
				turag_feldbus_slave_uart.package_overflow_flag = 1;
			}
			turag_feldbus_slave_uart.overflow = 1;
		}
#else
		turag_feldbus_slave_uart.overflow = 1;
#endif
	}

	// activate timer to recognize end of command
	turag_feldbus_slave_start_receive_timeout();
}


TURAG_INLINE void turag_feldbus_slave_ready_to_transmit() {
	if (turag_feldbus_slave_uart.txOffset == turag_feldbus_slave_uart.transmitLength) {
#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
		if (turag_feldbus_slave_uart.chksum_required)
#endif
			// all data bytes are processed, now transmit checksum
			turag_feldbus_slave_transmit_byte(turag_feldbus_slave_uart.chksum);

		turag_feldbus_slave_deactivate_dre_interrupt();
		turag_feldbus_slave_activate_tx_interrupt();
	} else {
		// accessing the buffer as an array is more effective than using a pointer
		// and increasing it.
		turag_feldbus_slave_transmit_byte(turag_feldbus_slave_uart.txbuf[turag_feldbus_slave_uart.txOffset]);
		++turag_feldbus_slave_uart.txOffset;
	}
}

TURAG_INLINE void turag_feldbus_slave_transmission_complete() {
	// turn off transmitter and release bus, turn on receiver
	turag_feldbus_slave_rts_off();
	turag_feldbus_slave_deactivate_tx_interrupt();
	turag_feldbus_slave_activate_rx_interrupt();

#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
	turag_feldbus_slave_uart.transmission_active = 0;
#endif
}


TURAG_INLINE void turag_feldbus_slave_receive_timeout_occured() {
#if TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE
	if (turag_feldbus_slave_uart.package_lost_flag) {
		++turag_feldbus_slave_info.packagecount_lost;
		turag_feldbus_slave_uart.package_lost_flag = 0;
	}
	
	if (turag_feldbus_slave_uart.package_overflow_flag) {
		++turag_feldbus_slave_info.packagecount_buffer_overflow;
		turag_feldbus_slave_uart.package_overflow_flag = 0;
	}
#endif

#if TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 1	
# if TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE
	if ((turag_feldbus_slave_uart.rxbuf[0] == MY_ADDR || turag_feldbus_slave_uart.rxbuf[0] == TURAG_FELDBUS_BROADCAST_ADDR) &&
			!turag_feldbus_slave_uart.overflow && 
			turag_feldbus_slave_uart.rxOffset > 1)
# else
	if (turag_feldbus_slave_uart.rxbuf[0] == MY_ADDR  &&
			!turag_feldbus_slave_uart.overflow && 
			turag_feldbus_slave_uart.rxOffset > 1)
# endif
#elif TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 2
# if TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE
	if (((turag_feldbus_slave_uart.rxbuf[0] == (MY_ADDR & 0xff) && turag_feldbus_slave_uart.rxbuf[1] == (MY_ADDR >> 8)) || 
		(turag_feldbus_slave_uart.rxbuf[0] == (TURAG_FELDBUS_BROADCAST_ADDR_2 & 0xff) && turag_feldbus_slave_uart.rxbuf[1] == (TURAG_FELDBUS_BROADCAST_ADDR_2 >> 8))) &&
			!turag_feldbus_slave_uart.overflow && 
			turag_feldbus_slave_uart.rxOffset > 1)
# else
	if ((turag_feldbus_slave_uart.rxbuf[0] == (MY_ADDR & 0xff) && turag_feldbus_slave_uart.rxbuf[1] == (MY_ADDR >> 8))  &&
			!turag_feldbus_slave_uart.overflow && 
			turag_feldbus_slave_uart.rxOffset > 1)
# endif
#endif		
	{
		// package ok -> signal main loop that we have package ready
		turag_feldbus_slave_uart.rx_length = turag_feldbus_slave_uart.rxOffset;
		
		// we stop the led blinking until the user program starts the package
		// processing
#if TURAG_FELDBUS_SLAVE_CONFIG_USE_LED_CALLBACK == 1
		turag_feldbus_slave_uart.toggleLedBlocked = 1;
#endif
	}

	// reset rxOffset and offset-flag to ensure correct
	// receiving of future packages
	turag_feldbus_slave_uart.rxOffset = 0;
	turag_feldbus_slave_uart.overflow = 0;
}

#if TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY != 0 || defined(__DOXYGEN__)
TURAG_INLINE void turag_feldbus_slave_increase_uptime_counter(void) {
	++turag_feldbus_slave_info.uptime_counter;

# if TURAG_FELDBUS_SLAVE_CONFIG_USE_LED_CALLBACK == 1
	// we only toggle the led if there is no package
	// waiting to be processsed.
	// We use this as an indicator for the user whether
	// there is something wrong with the communication.
	if (!turag_feldbus_slave_uart.toggleLedBlocked) {
#  if TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY >= 12
#   define COUNT_MAX (TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY / 12 - 1)
#    if COUNT_MAX > 255
		static uint16_t count = 0;
#    else
		static uint8_t count = 0;
#    endif
		static uint8_t subcount = 0;
	
		++count;
		if (count > COUNT_MAX) {
			if (subcount == 0) {
				turag_feldbus_slave_toggle_led();
			} else if (subcount == 1) {
				turag_feldbus_slave_toggle_led();
			}
			subcount = (subcount+1) & 7;
			count = 0;
		}
#  elif TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY >= 2
		static uint8_t count = 0;
		
		++count;
		if (count > (TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY / 2 - 1)) {
			turag_feldbus_slave_toggle_led();
			count = 0;
		}
#  else
		turag_feldbus_slave_toggle_led();
#  endif
	}
# endif
}
#else
# define turag_feldbus_slave_increase_uptime_counter()
#endif

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

	// we have to disable the receive interrupt to ensure that 
	// the rx-buffer does not get corrupted by incoming data
	// (even though the protocol already enforces this and it should
	// not happen at all)
	turag_feldbus_slave_deactivate_rx_interrupt();
	
	// we copy the value of rx_length because it is volatile and accessing
	// it is expensive.
	FeldbusSize_t length = turag_feldbus_slave_uart.rx_length;
	turag_feldbus_slave_uart.rx_length = 0;
	
	// we release the blinking to indicate that the user program is
	// still calling turag_feldbus_do_processing() as required.
#if TURAG_FELDBUS_SLAVE_CONFIG_USE_LED_CALLBACK == 1
	turag_feldbus_slave_uart.toggleLedBlocked = 0;
#endif
	turag_feldbus_slave_end_interrupt_protect();

	// if we are here, we have a package (with length>1 that is adressed to us) safe in our buffer
	// and we can start working on it
	

	// calculate checksum
#if TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE == TURAG_FELDBUS_CHECKSUM_XOR
	if (!xor_checksum_check(turag_feldbus_slave_uart.rxbuf, length - 1, turag_feldbus_slave_uart.rxbuf[length - 1]))
#elif TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE == TURAG_FELDBUS_CHECKSUM_CRC8_ICODE
	if (!turag_crc8_check(turag_feldbus_slave_uart.rxbuf, length - 1, turag_feldbus_slave_uart.rxbuf[length - 1]))
#endif
	{
#if TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE
		++turag_feldbus_slave_info.packagecount_chksum_mismatch;
#endif
		turag_feldbus_slave_activate_rx_interrupt();
		return;
	}
	
#if TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE
	++turag_feldbus_slave_info.packagecount_correct;
#endif

	// The address is already checked in turag_feldbus_slave_receive_timeout_occured(). 
	// Thus the second check is only necessary
	// to distinguish broadcasts from regular packages.
#if TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE
	// message addressed to me?
# if TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 1	
	if (turag_feldbus_slave_uart.rxbuf[0] == MY_ADDR) {
# elif TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 2
	if (turag_feldbus_slave_uart.rxbuf[0] == (MY_ADDR & 0xff) && turag_feldbus_slave_uart.rxbuf[1] == (MY_ADDR >> 8)) {	
# endif	
#endif
		if (length == 1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH) {
			// we received a ping request -> respond with empty packet (address + checksum)
			turag_feldbus_slave_uart.transmitLength = TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH;
		} else if (turag_feldbus_slave_uart.rxbuf[TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] == 0) {
			// first data byte is zero -> reserved packet
			if (length == 2 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH) {
				// received a debug packet
				_Static_assert(8 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
				turag_feldbus_slave_uart.txbuf[0 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = TURAG_FELDBUS_DEVICE_PROTOCOL;
				turag_feldbus_slave_uart.txbuf[1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = TURAG_FELDBUS_DEVICE_TYPE_ID;
#if TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE
				turag_feldbus_slave_uart.txbuf[2 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE | 0x80;
#else
				turag_feldbus_slave_uart.txbuf[2 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE;
#endif
				turag_feldbus_slave_uart.txbuf[3 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE & 0xff;
#if TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE > 255				
				turag_feldbus_slave_uart.txbuf[4 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = (TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE >> 8) & 0xff;
#else
				turag_feldbus_slave_uart.txbuf[4 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = 0;
#endif
#if TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE > 65535				
				turag_feldbus_slave_uart.txbuf[5 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = (TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE >> 16) & 0xff;
				turag_feldbus_slave_uart.txbuf[6 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = (TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE >> 24) & 0xff;
#else
				turag_feldbus_slave_uart.txbuf[5 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = 0;
				turag_feldbus_slave_uart.txbuf[6 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = 0;
#endif
				turag_feldbus_slave_uart.txbuf[7 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = sizeof(TURAG_FELDBUS_DEVICE_NAME) - 1;
				turag_feldbus_slave_uart.txbuf[8 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = sizeof(TURAG_FELDBUS_DEVICE_VERSIONINFO) - 1;
				turag_feldbus_slave_uart.txbuf[9 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY & 0xff;
				turag_feldbus_slave_uart.txbuf[10 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] = TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY >> 8;
				turag_feldbus_slave_uart.transmitLength = 11 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH;
			} else if (length == 3 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH) {
				switch (turag_feldbus_slave_uart.rxbuf[1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH]) {
				case TURAG_FELDBUS_SLAVE_COMMAND_DEVICE_NAME: {
					_Static_assert(sizeof(TURAG_FELDBUS_DEVICE_NAME) - 1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
					memcpy(turag_feldbus_slave_uart.txbuf + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH, turag_feldbus_slave_info.name, sizeof(TURAG_FELDBUS_DEVICE_NAME) - 1);
					turag_feldbus_slave_uart.transmitLength = sizeof(TURAG_FELDBUS_DEVICE_NAME) - 1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH;
					break;
				}
				case TURAG_FELDBUS_SLAVE_COMMAND_UPTIME_COUNTER: {
#if (TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY>0) && (TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY<=65535)
					_Static_assert(sizeof(turag_feldbus_slave_info.uptime_counter) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
					memcpy(turag_feldbus_slave_uart.txbuf + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH, &turag_feldbus_slave_info.uptime_counter, sizeof(turag_feldbus_slave_info.uptime_counter));
					turag_feldbus_slave_uart.transmitLength = sizeof(turag_feldbus_slave_info.uptime_counter) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH;
#else
					_Static_assert(sizeof(uint32_t) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
					turag_feldbus_slave_uart.transmitLength = sizeof(uint32_t) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH;
#endif
					break;
				}	
				case TURAG_FELDBUS_SLAVE_COMMAND_VERSIONINFO: {
					_Static_assert(sizeof(TURAG_FELDBUS_DEVICE_VERSIONINFO) - 1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
					memcpy(turag_feldbus_slave_uart.txbuf + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH, turag_feldbus_slave_info.versioninfo, sizeof(TURAG_FELDBUS_DEVICE_VERSIONINFO) - 1);
					turag_feldbus_slave_uart.transmitLength = sizeof(TURAG_FELDBUS_DEVICE_VERSIONINFO) - 1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH;
					break;
				}	
#if TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE
				case TURAG_FELDBUS_SLAVE_COMMAND_PACKAGE_COUNT_CORRECT: {
					_Static_assert(sizeof(turag_feldbus_slave_info.packagecount_correct) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
					memcpy(turag_feldbus_slave_uart.txbuf + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH, &turag_feldbus_slave_info.packagecount_correct, sizeof(turag_feldbus_slave_info.packagecount_correct));
					turag_feldbus_slave_uart.transmitLength = sizeof(turag_feldbus_slave_info.packagecount_correct) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH;
					break;
				}	
				case TURAG_FELDBUS_SLAVE_COMMAND_PACKAGE_COUNT_BUFFEROVERFLOW: {
					_Static_assert(sizeof(turag_feldbus_slave_info.packagecount_buffer_overflow) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
					memcpy(turag_feldbus_slave_uart.txbuf + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH, &turag_feldbus_slave_info.packagecount_buffer_overflow, sizeof(turag_feldbus_slave_info.packagecount_buffer_overflow));
					turag_feldbus_slave_uart.transmitLength = sizeof(turag_feldbus_slave_info.packagecount_buffer_overflow) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH;
					break;
				}	
				case TURAG_FELDBUS_SLAVE_COMMAND_PACKAGE_COUNT_LOST: {
					_Static_assert(sizeof(turag_feldbus_slave_info.packagecount_lost) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
					memcpy(turag_feldbus_slave_uart.txbuf + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH, &turag_feldbus_slave_info.packagecount_lost, sizeof(turag_feldbus_slave_info.packagecount_lost));
					turag_feldbus_slave_uart.transmitLength = sizeof(turag_feldbus_slave_info.packagecount_lost) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH;
					break;
				}
				case TURAG_FELDBUS_SLAVE_COMMAND_PACKAGE_COUNT_CHKSUM_MISMATCH: {
					_Static_assert(sizeof(turag_feldbus_slave_info.packagecount_chksum_mismatch) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
					memcpy(turag_feldbus_slave_uart.txbuf + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH, &turag_feldbus_slave_info.packagecount_chksum_mismatch, sizeof(turag_feldbus_slave_info.packagecount_chksum_mismatch));
					turag_feldbus_slave_uart.transmitLength = sizeof(turag_feldbus_slave_info.packagecount_chksum_mismatch) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH;
					break;
				}	
#else
				case TURAG_FELDBUS_SLAVE_COMMAND_PACKAGE_COUNT_CORRECT:
				case TURAG_FELDBUS_SLAVE_COMMAND_PACKAGE_COUNT_BUFFEROVERFLOW:
				case TURAG_FELDBUS_SLAVE_COMMAND_PACKAGE_COUNT_LOST:
				case TURAG_FELDBUS_SLAVE_COMMAND_PACKAGE_COUNT_CHKSUM_MISMATCH: {
					_Static_assert(sizeof(uint32_t) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
					turag_feldbus_slave_uart.transmitLength = sizeof(uint32_t) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH;
				}
#endif
				case TURAG_FELDBUS_SLAVE_COMMAND_PACKAGE_COUNT_ALL: {
#if TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE
					_Static_assert(sizeof(turag_feldbus_slave_info.packagecount_correct) + sizeof(turag_feldbus_slave_info.packagecount_buffer_overflow) + sizeof(turag_feldbus_slave_info.packagecount_lost) + sizeof(turag_feldbus_slave_info.packagecount_chksum_mismatch) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
					memcpy(turag_feldbus_slave_uart.txbuf + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH, &turag_feldbus_slave_info.packagecount_correct, sizeof(turag_feldbus_slave_info.packagecount_correct) + sizeof(turag_feldbus_slave_info.packagecount_buffer_overflow) + sizeof(turag_feldbus_slave_info.packagecount_lost) + sizeof(turag_feldbus_slave_info.packagecount_chksum_mismatch));
					turag_feldbus_slave_uart.transmitLength = sizeof(turag_feldbus_slave_info.packagecount_correct) + sizeof(turag_feldbus_slave_info.packagecount_buffer_overflow) + sizeof(turag_feldbus_slave_info.packagecount_lost) + sizeof(turag_feldbus_slave_info.packagecount_chksum_mismatch) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH;
#else
					_Static_assert(sizeof(uint32_t) * 4 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
					turag_feldbus_slave_uart.transmitLength = sizeof(uint32_t) * 4 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH;
#endif
					break;
				}	
				case TURAG_FELDBUS_SLAVE_COMMAND_RESET_PACKAGE_COUNT: {
					_Static_assert(TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
#if TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE
					turag_feldbus_slave_info.packagecount_correct = 0;
					turag_feldbus_slave_info.packagecount_buffer_overflow = 0;
					turag_feldbus_slave_info.packagecount_lost = 0;
					turag_feldbus_slave_info.packagecount_chksum_mismatch = 0;
#endif
					turag_feldbus_slave_uart.transmitLength = TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH;
					break;
				}
				default:
					// unhandled reserved packet with length == 3 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH
					turag_feldbus_slave_activate_rx_interrupt();
					return;
				}
			} else {
				// unhandled reserved packet with length > 3 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH
				turag_feldbus_slave_activate_rx_interrupt();
				return;
			}
		} else {
			// received some other packet --> let somebody else process it
			turag_feldbus_slave_uart.transmitLength = turag_feldbus_slave_process_package(
				turag_feldbus_slave_uart.rxbuf + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH, 
				length - (1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH), 
				turag_feldbus_slave_uart.txbuf + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH;
				
			// this happens if the device protocol or the user code returned TURAG_FELDBUS_IGNORE_PACKAGE.
			if (turag_feldbus_slave_uart.transmitLength == (uint8_t)(TURAG_FELDBUS_IGNORE_PACKAGE + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH)) {
				turag_feldbus_slave_activate_rx_interrupt();
				return;
			}
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
		turag_feldbus_slave_uart.chksum = xor_checksum_calculate(turag_feldbus_slave_uart.txbuf, turag_feldbus_slave_uart.transmitLength);
#elif TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE == TURAG_FELDBUS_CHECKSUM_CRC8_ICODE
		turag_feldbus_slave_uart.chksum = turag_crc8_calculate(turag_feldbus_slave_uart.txbuf, turag_feldbus_slave_uart.transmitLength);
#endif
		
		start_transmission();

	// not every device protocol requires broadcasts, so we can save a few bytes here
#if TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE
	} else {
		if (length == 1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH) {
			// compatibility mode to support deprecated Broadcasts without protocol-ID
			turag_feldbus_slave_process_broadcast(0, 0, TURAG_FELDBUS_DEVICE_PROTOCOL_LOKALISIERUNGSSENSOREN);
		} else if (turag_feldbus_slave_uart.rxbuf[TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] == TURAG_FELDBUS_BROADCAST_TO_ALL_DEVICES || 
				turag_feldbus_slave_uart.rxbuf[TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH] == TURAG_FELDBUS_DEVICE_PROTOCOL) {
			// otherwise process only the correct broadcasts
			turag_feldbus_slave_process_broadcast(
				turag_feldbus_slave_uart.rxbuf + 1 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH, 
				length - (2 + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH), 
				turag_feldbus_slave_uart.rxbuf[TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH]);
		}
	}
#endif
	turag_feldbus_slave_activate_rx_interrupt();
}


#endif /* TINA_FELDBUS_SLAVE_FELDBUS_H_ */
