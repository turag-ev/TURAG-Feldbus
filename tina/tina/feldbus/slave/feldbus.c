/**
 *  @brief		Implements slave side TURAG feldbus support
 *  @file		feldbus.c
 *  @date		07.11.2013
 *  @author		Martin Oemus <martin@oemus.net>
 * 
 */

#include "feldbus.h"
#include <tina/crc/xor.h>
#include <tina/crc/crc8.h>

#ifdef TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
#warning TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
#endif
#ifndef MY_ADDR
#error MY_ADDR must be defined
#endif
#ifndef TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE
#error TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE must be defined
#endif
#ifndef TURAG_FELDBUS_SLAVE_CONFIG_BOOTLOADER_AVAILABLE
#error TURAG_FELDBUS_SLAVE_CONFIG_BOOTLOADER_AVAILABLE must be defined
#endif
#ifndef TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE
#error TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE must be defined
#endif





typedef struct {
	uint8_t rxbuf[TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE];
	uint8_t txbuf[TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE];
	uint8_t length;
	uint8_t index;
	uint8_t overflow;
	uint8_t chksum;
#ifdef TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
	volatile uint8_t transmission_active;
	uint8_t chksum_required;
#endif
} turag_feldbus_slave_uart_t;

static turag_feldbus_slave_uart_t uart;



void turag_feldbus_slave_init() {
	uart.txbuf[0] = TURAG_FELDBUS_MASTER_ADDR|MY_ADDR;
	uart.index = 0;
	uart.overflow = 0;

#ifdef TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
	uint8_t transmission_active = 0;
#endif
}

void turag_feldbus_slave_byte_received(uint8_t data) {
	uart.rxbuf[uart.index] = data;
	++uart.index;

	if (uart.index >= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE) {
		uart.index = 0;
		uart.overflow = 1;
	}

	// activate timer to recognize end of command
	turag_feldbus_slave_start_receive_timeout();
}

void turag_feldbus_slave_ready_to_transmit() {
	if (uart.index == uart.length) {
		// all data bytes are processed, now transmit checksum
		turag_feldbus_slave_transmit_byte(uart.chksum);
	} else {
		turag_feldbus_slave_transmit_byte(uart.txbuf[uart.index]);
		++uart.index;
	}
}

void turag_feldbus_slave_transmission_complete() {
	// turn off transmitter and release bus, turn on receiver
	turag_feldbus_slave_prepare_receiving();

	uart.index = 0;
	uart.transmission_active = 0;
}

void turag_feldbus_slave_receive_timeout_occured() {
	if (uart.overflow) goto CLEANUP;

	// calculate checksum
#if TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE == TURAG_FELDBUS_CHECKSUM_XOR
	if (!xor_checksum_check(uart.rxbuf, uart.index - 1, uart.rxbuf[uart.index]))
#elif TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE == TURAG_FELDBUS_CHECKSUM_CRC8_ICODE
	if (!turag_crc8_check(uart.rxbuf, uart.index - 1, uart.rxbuf[uart.index]))
#endif
		goto CLEANUP;

	// message addressed to me?
	if (uart.rxbuf[0] == MY_ADDR) {
		uart.length = turag_feldbus_slave_process_package(uart.rxbuf + 1, uart.index - 2, uart.txbuf + 1) + 1;
#if TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE == TURAG_FELDBUS_CHECKSUM_XOR
		uart.txbuf[uart.length] = xor_checksum_calculate(uart.txbuf, uart.length);
#elif TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE == TURAG_FELDBUS_CHECKSUM_CRC8_ICODE
		uart.txbuf[uart.length] = turag_crc8_calculate(uart.txbuf, uart.length);
#endif
		++uart.length;
#ifdef TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
		uart.chksum_required = 1;
#endif
		start_transmission();
	} else if (uart.rxbuf[0] == TURAG_FELDBUS_BROADCAST_ADDR) {
		turag_feldbus_slave_process_broadcast(uart.rxbuf + 1, uart.index - 2);
	}


CLEANUP:
	uart.chksum = 0;
	uart.index = 0;
	uart.overflow = 0;
}
