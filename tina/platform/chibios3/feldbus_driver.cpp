#define TURAG_DEBUG_LOG_SOURCE "H"

#include <tina++/tina.h>

#if TURAG_USE_TURAG_FELDBUS_HOST

#include <ch.h>
#include <hal.h>
#include <tina++/debug.h>
#include <tina++/time.h>
#include <tina++/thread.h>
#include <tina++/feldbus_driver.h>


namespace TURAG {
namespace Feldbus {


FeldbusDriver::FeldbusDriver(const char* name, bool threadSafe) :
	FeldbusAbstraction(name, threadSafe), uart_driver_(nullptr)
{ }

void FeldbusDriver::rxEnd(UARTDriver* d) {
	chSysLockFromISR();
	FeldbusDriver* instance = reinterpret_cast<UARTConfigExt*>( const_cast<UARTConfig*>(d->config) )->instance;
	chBSemSignalI(&instance->receive_sem_);
	chSysUnlockFromISR();
}

void FeldbusDriver::txEnd2(UARTDriver* d) {
	chSysLockFromISR();
	FeldbusDriver* instance = reinterpret_cast<UARTConfigExt*>( const_cast<UARTConfig*>(d->config) )->instance;
	palClearLine(instance->rts_);
	chBSemSignalI(&instance->send_sem_);
	chSysUnlockFromISR();
}

bool FeldbusDriver::init(UARTDriver* uart_driver, uint32_t baud_rate, TuragSystemTime timeout, ioline_t rts)
{
	uart_driver_ = uart_driver;
	rts_ = rts;
	rs485_timeout_ = timeout;

	if (!uart_driver_)
	{
		turag_error("serialDriver_ must not be null.");
		return false;
	}

	chBSemObjectInit(&send_sem_, true);
	chBSemObjectInit(&receive_sem_, true);
	
	// TODO Hardware Driver-Enable where supported
	uart_config_.cfg.speed = baud_rate;
	uart_config_.cfg.cr1 = 0;
	uart_config_.cfg.cr2 = 0;
	uart_config_.cfg.cr3 = 0;
	uart_config_.cfg.rxchar_cb = nullptr;
	uart_config_.cfg.rxend_cb = rxEnd;
	uart_config_.cfg.rxerr_cb = nullptr;
	uart_config_.cfg.txend1_cb = nullptr;
	uart_config_.cfg.txend2_cb = txEnd2;
	uart_config_.instance = this;
	uartStart(uart_driver_, reinterpret_cast<UARTConfig*>(&uart_config_));

	palSetLineMode(rts_, PAL_MODE_OUTPUT_PUSHPULL);
	palClearLine(rts_);

	
	// wait until slaves left their bootloaders
	chThdSleepMilliseconds(500);

	turag_infof("RS485 init: %u baud, timeout %u ms -> status %d\n", (unsigned int)baud_rate, (unsigned int)timeout.value, uart_driver_->state);

	if (uart_driver_->state != UART_READY) {
		turag_error("RS485 init FAILED!");
	}

	// calculate bus delay required for 15 frames distance of
	// TURAG feldbus [us]
	// we always round up.
	bus_delay_ = (15 * 1000000 - 1) / baud_rate + 1;
	delay_timer_.init();

	return (uart_driver_->state == UART_READY);
}

bool FeldbusDriver::isReady()
{
	return uart_driver_ && (uart_driver_->state == UART_READY);
}

bool FeldbusDriver::doTransceive(const uint8_t *transmit, int *transmit_length, uint8_t *receive, int *receive_length, bool delay_transmission)
{
	// prevents crashes
	while (!isReady()) {
		chThdSleepMilliseconds(10);
	}
	bool send_ok = false, recv_ok = false;
	// Insert bus delay, if required. The timer is started after each transmission,
	// making sure we never wait longer than necessary.
	if (delay_transmission) {
		delay_timer_.wait();
	}

	if (transmit && transmit_length && *transmit_length) {
		chBSemReset(&send_sem_, true);
		// activate RS485 driver for sending, its reset by txEnd2 callback (or on timeout)
		palSetLine(rts_);
		//start sending data
		uartStartSend(uart_driver_, *transmit_length, transmit);
		//wait for completion
		if(chBSemWaitTimeout(&send_sem_, MS2ST(5)) != MSG_OK) {
			uartStopSend(uart_driver_);
			palClearLine(rts_);
			send_ok = false;
			turag_criticalf("%s: UART send timed out.", name());
		} else {
			send_ok = true;
		}
		turag_debugf("%s: sending %d bytes, driver state %d, OK: %d", name(), *transmit_length, uart_driver_->state, send_ok);
	}

	if (!send_ok) {
		if (receive_length) {
			*receive_length = 0;
		}
		return false;
	}

	if (receive && receive_length && *receive_length) {
		chBSemReset(&receive_sem_, true);
		//start receiving data
		uartStartReceive(uart_driver_, *receive_length, receive);
		//wait for completion
		if(chBSemWaitTimeout(&receive_sem_, rs485_timeout_.value) != MSG_OK) {
			uartStopReceive(uart_driver_);
			recv_ok = false;
			turag_criticalf("%s: UART receive timed out.", name());
		} else {
			recv_ok = true;
		}
		turag_debugf("%s: receiving %d bytes, driver state %d, OK : %d", name(), *receive_length, uart_driver_->state, recv_ok);
	}
	// Start the delay timer after communication is finished.
	// On the next call of this function we can wait for the timeout.
	// This way we can use the required delay time for computing if there
	// is anything to be done.
	delay_timer_.startTimeout_us(bus_delay_);

	return (send_ok && recv_ok);
}

void FeldbusDriver::clearBuffer()
{
	while (!isReady()) {
		chThdSleepMilliseconds(10);
	}
	return;
}




} // namespace Feldbus
} // namespace TURAG


#endif // TURAG_USE_TURAG_FELDBUS_HOST
