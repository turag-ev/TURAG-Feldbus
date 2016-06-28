#define TURAG_DEBUG_LOG_SOURCE "H"

#include <tina++/tina.h>

#if TURAG_USE_TURAG_FELDBUS_HOST

#include <ch.h>
#include <hal.h>
#include <tina++/debug.h>
#include <tina++/time.h>
#include <tina++/thread.h>
#include <tina++/feldbus_driver.h>
#include <platform/chibios/backplane.h>


namespace TURAG {
namespace Feldbus {


FeldbusDriver::FeldbusDriver(void) :
	FeldbusAbstraction(), serialDriver(nullptr)
{
}

bool FeldbusDriver::init(SerialDriver *serialDriver_, uint32_t baud_rate, TuragSystemTime timeout)
{
	if (!serialDriver_) return false;
	serialDriver = serialDriver_;

	serialConfig.speed = baud_rate;
	serialConfig.cr1 = 0;
	serialConfig.cr2 = 0;
	serialConfig.cr3 = 0;
	sdStart(serialDriver, &serialConfig);

	rs485_timeout = timeout;


	// flush transmit queue
	uint8_t buf[1] = { 0 };
	sdReadTimeout(serialDriver, buf, 1, TIME_IMMEDIATE);
	sdWriteTimeout(serialDriver, buf, 1, TIME_IMMEDIATE);

	// wait until slaves left their bootloaders
	chBSemInit(&sem, TRUE);
	chThdSleepMilliseconds(500);
	chBSemSignal(&sem);

	turag_infof("RS485 init: %u baud, timeout %u ms -> status %d\n", (unsigned int)baud_rate, (unsigned int)timeout.value, serialDriver->state);

	if (serialDriver->state != SD_READY) {
		turag_error("RS485 init FAILED!");
	}

	// calculate bus delay required for 15 frames distance of
	// TURAG feldbus [us]
	// we always round up.
	bus_delay = (15 * 1000000 - 1) / baud_rate + 1;
	delay.init();

	return (serialDriver->state == SD_READY);
}

bool FeldbusDriver::isReady()
{
	return serialDriver && (serialDriver->state == SD_READY);
}

bool FeldbusDriver::transceive(const uint8_t *transmit, int *transmit_length, uint8_t *receive, int *receive_length, bool delayTransmission)
{
	if (!serialDriver) return false;

	// prevents crashes
	while (!isReady()) {
		chThdSleepMilliseconds(10);
	}

	bool send_ok = true, recv_ok = true;
	uint16_t transmit_delay = 0;

	// communication on the bus should always be atomic, thus the semaphore wait.
	chBSemWait(&sem);

	// Insert bus delay, if required. The timer is started after each transmission,
	// making sure we never wait longer than necessary.
	if (delayTransmission) {
		delay.wait();
	}

	if (transmit && transmit_length && *transmit_length) {
		int transmit_length_copy = *transmit_length;

		// activate RS485 driver for sending
		palSetPad(GPIOD, BPD_SC_RTS);
		int ok = sdWriteTimeout(serialDriver, transmit, transmit_length_copy, MS2ST(5));
		// TC interrupt handler sets the RTS pin after transmission is completed

		// calculate transmit time in [us]
		transmit_delay = 1000000 * 10 / serialConfig.speed * ok;

		send_ok = (transmit_length_copy == ok);
		*transmit_length = ok;
		turag_debugf("turag_rs485_transceive: sending %d bytes, %d bytes written ok, driver state %d, OK: %d", transmit_length_copy, ok, serialDriver->state, send_ok);
	}

	if (!send_ok) {
		if (receive_length) {
			*receive_length = 0;
		}
		chBSemSignal(&sem);
		return false;
	}

	if (receive && receive_length && *receive_length) {
		int receive_length_copy = *receive_length;

		// read answer, timeout in systemticks!
		int ok = sdReadTimeout(serialDriver, receive, receive_length_copy, rs485_timeout.value);

		recv_ok = (ok == receive_length_copy);
		*receive_length = ok;
		turag_debugf("turag_rs485_transceive: receiving %d bytes, retval %d, driver state %d, OK: %d", receive_length_copy, ok, serialDriver->state, recv_ok);

		// Start the delay timer after communication is finished.
		// On the next call of this function we can wait for the timeout.
		// This way we can use the required delay time for computing if there
		// is anything to be done.
		delay.startTimeout_us(bus_delay);
	} else {
		// also wait for transmission to finish if we did not receive any data
		delay.startTimeout_us(bus_delay + transmit_delay);
	}

	chBSemSignal(&sem);

	return (send_ok && recv_ok);
}

void FeldbusDriver::clearBuffer()
{
	if (serialDriver) {
		while (!isReady()) {
			chThdSleepMilliseconds(10);
		}

		while (sdGetTimeout(serialDriver, TIME_IMMEDIATE) != Q_TIMEOUT);
	}
}




} // namespace Feldbus
} // namespace TURAG


#endif // TURAG_USE_TURAG_FELDBUS_HOST
