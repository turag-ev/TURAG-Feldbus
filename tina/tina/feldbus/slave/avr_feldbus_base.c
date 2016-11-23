
#include "avr_feldbus_base.h"

#ifdef TURAG_FELDBUS_SLAVE_BASE_IMPLEMENTATION_IS_AVR_VERSION

turag_feldbus_slave_uart_t turag_feldbus_slave_uart = {
	.transmitLength = 0,
	.txOffset = 0,
	.rxOffset = 0,
	.rx_length = 0,
	.overflow = 0,
#if TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE
	.package_lost_flag = 0,
	.buffer_overflow_flag = 0,
#endif
#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
	.transmission_active = 0,
#endif	
#if TURAG_FELDBUS_SLAVE_CONFIG_USE_LED_CALLBACK == 1
	.toggleLedBlocked = 0,
#endif
};

turag_feldbus_slave_info_t turag_feldbus_slave_info = {
	.name = TURAG_FELDBUS_DEVICE_NAME,
	.versioninfo = TURAG_FELDBUS_DEVICE_VERSIONINFO,
#if TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE
	.packagecount_correct = 0,
	.packagecount_buffer_overflow = 0,
	.packagecount_lost = 0,
	.packagecount_chksum_mismatch = 0,
#endif
#if (TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY>0) && (TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY<=65535)
	.uptime_counter = 0
#endif
};


void turag_feldbus_slave_init() {
#if TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 1
	turag_feldbus_slave_uart.txbuf[0] = TURAG_FELDBUS_MASTER_ADDR|MY_ADDR;
#elif TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 2
	turag_feldbus_slave_uart.txbuf[0] = (TURAG_FELDBUS_MASTER_ADDR_2|MY_ADDR) & 0xff;
	turag_feldbus_slave_uart.txbuf[1] = (TURAG_FELDBUS_MASTER_ADDR_2|MY_ADDR) >> 8;
#endif
	
	turag_feldbus_hardware_init();
	
	turag_feldbus_slave_rts_off();
	turag_feldbus_slave_activate_rx_interrupt();
	turag_feldbus_slave_deactivate_dre_interrupt();
	turag_feldbus_slave_deactivate_tx_interrupt();
}




#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
static uint8_t digit_to_hex(uint8_t dig) {
	dig &= 0xF;
	dig += '0';
	if (dig > '9') dig += 'A' - '0' - 10;
	return dig;
}


// public debug functions
void print_text(const char *buf) {
	while (turag_feldbus_slave_uart.transmission_active);

	uint8_t i = 0;

	while(*buf) {
		turag_feldbus_slave_uart.txbuf[i] = *buf;
		++i;
		++buf;

		if (i >= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE) {
			i = 1;
			break;
		}
	}
	turag_feldbus_slave_uart.transmitLength = i;
	turag_feldbus_slave_uart.chksum_required = 0;
	start_transmission();
}

void print_char(uint8_t x) {
	while (turag_feldbus_slave_uart.transmission_active);

	turag_feldbus_slave_uart.txbuf[0] = '0';
	turag_feldbus_slave_uart.txbuf[1] = 'x';
	turag_feldbus_slave_uart.txbuf[2] = digit_to_hex(x >> 4);
	turag_feldbus_slave_uart.txbuf[3] = digit_to_hex(x);

	turag_feldbus_slave_uart.transmitLength = 4;
	turag_feldbus_slave_uart.chksum_required = 0;
	start_transmission();
}

void print_short(uint16_t x) {
	while (turag_feldbus_slave_uart.transmission_active);

	turag_feldbus_slave_uart.txbuf[0] = '0';
	turag_feldbus_slave_uart.txbuf[1] = 'x';
	turag_feldbus_slave_uart.txbuf[2] = digit_to_hex(x >> 12);
	turag_feldbus_slave_uart.txbuf[3] = digit_to_hex(x >> 8);
	turag_feldbus_slave_uart.txbuf[4] = digit_to_hex(x >> 4);
	turag_feldbus_slave_uart.txbuf[5] = digit_to_hex(x);
	turag_feldbus_slave_uart.txbuf[6] = '\r';
	turag_feldbus_slave_uart.txbuf[7] = '\n';

	turag_feldbus_slave_uart.transmitLength = 8;
	turag_feldbus_slave_uart.chksum_required = 0;
	start_transmission();
}
void print_short_nn(uint16_t x) {
	while (turag_feldbus_slave_uart.transmission_active);

	turag_feldbus_slave_uart.txbuf[0] = '0';
	turag_feldbus_slave_uart.txbuf[1] = 'x';
	turag_feldbus_slave_uart.txbuf[2] = digit_to_hex(x >> 12);
	turag_feldbus_slave_uart.txbuf[3] = digit_to_hex(x >> 8);
	turag_feldbus_slave_uart.txbuf[4] = digit_to_hex(x >> 4);
	turag_feldbus_slave_uart.txbuf[5] = digit_to_hex(x);

	turag_feldbus_slave_uart.transmitLength = 6;
	turag_feldbus_slave_uart.chksum_required = 0;
	turag_feldbus_slave_uart.transmission_active = 1;
	start_transmission();
}

void print_sshort(int16_t x) {
	while (turag_feldbus_slave_uart.transmission_active);

	if (x<0) {
		turag_feldbus_slave_uart.txbuf[0] = '-';
		x=-x;
	} else turag_feldbus_slave_uart.txbuf[0] = '+';

	turag_feldbus_slave_uart.txbuf[1] = '0';
	turag_feldbus_slave_uart.txbuf[2] = 'x';
	turag_feldbus_slave_uart.txbuf[3] = digit_to_hex(x >> 12);
	turag_feldbus_slave_uart.txbuf[4] = digit_to_hex(x >> 8);
	turag_feldbus_slave_uart.txbuf[5] = digit_to_hex(x >> 4);
	turag_feldbus_slave_uart.txbuf[6] = digit_to_hex(x);
	turag_feldbus_slave_uart.txbuf[7] = '\r';
	turag_feldbus_slave_uart.txbuf[8] = '\n';

	turag_feldbus_slave_uart.transmitLength = 9;
	turag_feldbus_slave_uart.chksum_required = 0;
	start_transmission();
}
void print_sshort_nn(int16_t x) {
	while (turag_feldbus_slave_uart.transmission_active);

	if (x<0) {
		turag_feldbus_slave_uart.txbuf[0] = '-';
		x=-x;
	} else turag_feldbus_slave_uart.txbuf[0] = '+';

	turag_feldbus_slave_uart.txbuf[1] = '0';
	turag_feldbus_slave_uart.txbuf[2] = 'x';
	turag_feldbus_slave_uart.txbuf[3] = digit_to_hex(x >> 12);
	turag_feldbus_slave_uart.txbuf[4] = digit_to_hex(x >> 8);
	turag_feldbus_slave_uart.txbuf[5] = digit_to_hex(x >> 4);
	turag_feldbus_slave_uart.txbuf[6] = digit_to_hex(x);

	turag_feldbus_slave_uart.transmitLength = 7;
	turag_feldbus_slave_uart.chksum_required = 0;
	start_transmission();
}

void print_long(uint32_t x) {
	while (turag_feldbus_slave_uart.transmission_active);

	turag_feldbus_slave_uart.txbuf[0] = '0';
	turag_feldbus_slave_uart.txbuf[1] = 'x';
	turag_feldbus_slave_uart.txbuf[2] = digit_to_hex(x >> 28);
	turag_feldbus_slave_uart.txbuf[3] = digit_to_hex(x >> 24);
	turag_feldbus_slave_uart.txbuf[4] = digit_to_hex(x >> 20);
	turag_feldbus_slave_uart.txbuf[5] = digit_to_hex(x >> 16);
	turag_feldbus_slave_uart.txbuf[6] = digit_to_hex(x >> 12);
	turag_feldbus_slave_uart.txbuf[7] = digit_to_hex(x >> 8);
	turag_feldbus_slave_uart.txbuf[8] = digit_to_hex(x >> 4);
	turag_feldbus_slave_uart.txbuf[9] = digit_to_hex(x);
	turag_feldbus_slave_uart.txbuf[10] = '\r';
	turag_feldbus_slave_uart.txbuf[11] = '\n';


	turag_feldbus_slave_uart.transmitLength = 12;
	turag_feldbus_slave_uart.chksum_required = 0;
	start_transmission();
}

void print_short_d(int16_t x) {
	while (turag_feldbus_slave_uart.transmission_active);

	turag_feldbus_slave_uart.txbuf[0] = ' ';
	turag_feldbus_slave_uart.txbuf[1] = ' ';
	turag_feldbus_slave_uart.txbuf[2] = ' ';
	turag_feldbus_slave_uart.txbuf[3] = ' ';
	turag_feldbus_slave_uart.txbuf[4] = ' ';
	turag_feldbus_slave_uart.txbuf[5] = ' ';
	turag_feldbus_slave_uart.txbuf[6] = '\r';
	turag_feldbus_slave_uart.txbuf[7] = '\n';
	itoa(x, (char*)turag_feldbus_slave_uart.txbuf, 10);

	turag_feldbus_slave_uart.transmitLength = 8;
	turag_feldbus_slave_uart.chksum_required = 0;
	start_transmission();
}

void print_slong(int32_t x) {
	while (turag_feldbus_slave_uart.transmission_active);

	if (x<0) {
		turag_feldbus_slave_uart.txbuf[0] = '-';
		x=-x;
	} else turag_feldbus_slave_uart.txbuf[0] = '+';

	turag_feldbus_slave_uart.txbuf[1] = '0';
	turag_feldbus_slave_uart.txbuf[2] = 'x';
	turag_feldbus_slave_uart.txbuf[3] = digit_to_hex(x >> 28);
	turag_feldbus_slave_uart.txbuf[4] = digit_to_hex(x >> 24);
	turag_feldbus_slave_uart.txbuf[5] = digit_to_hex(x >> 20);
	turag_feldbus_slave_uart.txbuf[6] = digit_to_hex(x >> 16);
	turag_feldbus_slave_uart.txbuf[7] = digit_to_hex(x >> 12);
	turag_feldbus_slave_uart.txbuf[8] = digit_to_hex(x >> 8);
	turag_feldbus_slave_uart.txbuf[9] = digit_to_hex(x >> 4);
	turag_feldbus_slave_uart.txbuf[10] = digit_to_hex(x);
	turag_feldbus_slave_uart.txbuf[11] = '\r';
	turag_feldbus_slave_uart.txbuf[12] = '\n';

	turag_feldbus_slave_uart.transmitLength = 13;
	turag_feldbus_slave_uart.chksum_required = 0;
	start_transmission();
}
#endif

#endif // TURAG_FELDBUS_SLAVE_BASE_IMPLEMENTATION_IS_AVR_VERSION
