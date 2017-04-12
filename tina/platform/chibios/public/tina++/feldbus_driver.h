#ifndef TINA_FELDBUS_DRIVER_H
#define TINA_FELDBUS_DRIVER_H

#include <tina++/tina.h>

#if !TURAG_USE_TURAG_FELDBUS_HOST
# warning TURAG_USE_TURAG_FELDBUS_HOST must be defined to 1
#endif

#include <tina++/time.h>
#include <tina++/feldbus/host/feldbusabstraction.h>
#include <tina++/utils/highresdelaytimer.h>

#include <hal.h>


namespace TURAG {
namespace Feldbus {


/**
* FeldbusDriver-Klasse.
*/
class FeldbusDriver : public FeldbusAbstraction {
public:
	FeldbusDriver(const char* name, bool threadSafe = true);

#if TURAG_USE_LIBSUPCPP_RUNTIME_SUPPORT
	virtual ~FeldbusDriver(void) {}
#endif

	bool init(SerialDriver* serialDriver_, uint32_t baud_rate, TuragSystemTime timeout);
	bool isReady(void);

	virtual void clearBuffer(void);

private:
	virtual bool doTransceive(const uint8_t *transmit, int *transmit_length, uint8_t *receive, int *receive_length, bool delayTransmission);

	SerialConfig serialConfig;
	SerialDriver* serialDriver;

	HighResDelayTimer delay;
	uint16_t bus_delay;
	TuragSystemTime rs485_timeout;
};




} // namespace Feldbus
} // namespace TURAG



#endif // _TINA_FELDBUS_DRIVER_H_
