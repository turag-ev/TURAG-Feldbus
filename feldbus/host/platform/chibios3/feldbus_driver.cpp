#define TURAG_DEBUG_LOG_SOURCE "H"

// Note: Debug log is so slow that we are still printing
// our debug log while we're receiving the answer.
#if defined(TURAG_DEBUG_LEVEL) && TURAG_DEBUG_LEVEL > 4
#undef TURAG_DEBUG_LEVEL
#define TURAG_DEBUG_LEVEL 4
#endif

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
	
	// TODO Hardware Driver-Enable where supported
    uart_config_.speed = baud_rate;
    uart_config_.cr1 = 0;
    uart_config_.cr2 = 0;
    uart_config_.cr3 = 0;
    uart_config_.rxchar_cb = nullptr;
    uart_config_.rxend_cb = nullptr;
    uart_config_.rxerr_cb = nullptr;
    uart_config_.txend1_cb = nullptr;
    uart_config_.txend2_cb = nullptr;
    uartStart(uart_driver_, &uart_config_);

	palSetLineMode(rts_, PAL_MODE_OUTPUT_PUSHPULL);
	palClearLine(rts_);

	
	// wait until slaves left their bootloaders
	chThdSleepMilliseconds(500);

	turag_infof("RS485 init: %u baud, timeout %u ms -> status %d\n", (unsigned int)baud_rate, (unsigned int)timeout.value, uart_driver_->state);

	if (uart_driver_->state != UART_READY) {
		turag_error("RS485 init FAILED!");
	}

	// Calculate bus delay required for at least 1.5 frames (10 bit per frame)
    // distance of TURAG feldbus [us]. We always round up.
	bus_delay_ = (16 * 1000000 - 1) / baud_rate + 1;
	delay_timer_.init();

	return (uart_driver_->state == UART_READY);
}

bool FeldbusDriver::isReady()
{
	return uart_driver_ && (uart_driver_->state == UART_READY);
}

static inline constexpr
const char * msg_to_str(msg_t msg) {
    return (msg == MSG_OK)
            ? "OK"
            : (msg == MSG_TIMEOUT)
              ? "TIMEOUT"
              : (msg == MSG_RESET)
                ? "RESET"
                : "???";
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
        // activate RS485 driver for sending
        palSetLine(rts_);

        msg_t tx_res = uartSendFullTimeout(uart_driver_, (size_t*)transmit_length, transmit, MS2ST(5));
        send_ok = (tx_res == MSG_OK);
        if (!send_ok)
            turag_debugf("%s: UART send failed for reason %s", name(), msg_to_str(tx_res));

        palClearLine(rts_);
        turag_debugf("%s: sent %d bytes, driver state %d, tx state: %d, OK: %d",
                     name(), *transmit_length, uart_driver_->state, uart_driver_->txstate, send_ok);
	}

	if (!send_ok) {
		if (receive_length) {
			*receive_length = 0;
		}
		return false;
	}

    if (receive && receive_length && *receive_length) {
        msg_t rx_res = uartReceiveTimeout(uart_driver_, (size_t*)receive_length, receive, rs485_timeout_.value);
        recv_ok = (rx_res == MSG_OK);
        if (!recv_ok)
            turag_debugf("%s: UART receive failed for reason %s", name(), msg_to_str(rx_res));

        turag_debugf("%s: received %d bytes, driver state %d, rx state: %d, OK : %d",
                     name(), *receive_length, uart_driver_->state, uart_driver_->rxstate, recv_ok);
    } else {
        recv_ok = true;
        turag_debugf("%s: This was a broadcast. There was no receive.", name());
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
