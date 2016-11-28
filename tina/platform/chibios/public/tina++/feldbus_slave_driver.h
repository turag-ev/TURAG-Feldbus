/*
 * feldbus-driver.h
 *
 *  Created on: 20.11.2016
 *      Author: martin
 *
 *      This is a tina-platform header. As such its interface
 *      should be platform-independent, which it isn't.
 *      To get the most flexibility, init() should be called
 *      in platform code and start() in generic code.
 */

#ifndef FELDBUS_SLAVE_DRIVER_H_
#define FELDBUS_SLAVE_DRIVER_H_

#include <ch.h>
#include <hal.h>

#include <tina++/tina.h>
#include <tina++/thread.h>
#include <tina/feldbus/slave/feldbus_config_check.h>



namespace TURAG {
namespace Feldbus {
namespace Slave {

// should there ever be a need to run more than one slave
// device on one board, all static functions
// could be replaced with normal ones.
/**
 * @brief
 * @ingroup feldbus-slave-base
 */
class Driver {
public:
	/**
	 * @brief
	 */
    struct HardwareConfig {
        UARTDriver *uartp;
        UARTConfig uartConfig;
        uint32_t baudrate;		// must be set in uartConfig and in baudrate
        GPTDriver * gptp;
        GPTConfig gptConfig;
        ioportid_t ledPort;
        unsigned ledPin;
        bool ledInverted;
        ioportid_t rtsPort;
        unsigned rtsPin;
        bool rtsInverted;
    };


    // sets up hardware-specific stuff
    // should be called in hardware-dependent source-file
    /**
     *
     * @param config
     */
    static void init(HardwareConfig* config);

    /**
     *
     * @param thread
     * @param prio
     */
    static void start(ThreadImpl* thread, int prio);

    /**
     * @brief Schaltet die konfigurierte LED um.
     */
    static void toggleLed(void) {
    	if (config->ledPort) {
    		palTogglePad(config->ledPort, config->ledPin);
    	}
    }

#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED || defined(__DOXYGEN__)
    /**
    * @brief Sendet blockierend beliebige Daten z.B. zum debuggen.
    * @param[in] data Zu sendende Daten.
    * @param[in] length Länge des Datensatzes.
    */
    static void transmitDebugData(const void* data, size_t length);
#endif
private:
    struct Data {
        FeldbusSize_t rx_size;
        bool processing;    // set and cleared by thread
        bool packet_complete; // set isr, cleared by thread and isr
        uint8_t txbuf[TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE];
        uint8_t rxbuf[TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE];

        uint32_t timerTicks;
        bool overflow;
    };

    // disallow instances
    Driver() {}
    
    static void enableRts(void) {
        if (config->rtsInverted) {
            palClearPad(config->rtsPort, config->rtsPin);
        } else {
            palSetPad(config->rtsPort, config->rtsPin);
        }
    }

    static void disableRts(void) {
        if (config->rtsInverted) {
            palSetPad(config->rtsPort, config->rtsPin);
        } else {
            palClearPad(config->rtsPort, config->rtsPin);
        }
    }    
    
    static bool packetAdressedToMe(void) {
    #if TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 1
    # if TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE
        if ((data.rxbuf[0] == MY_ADDR || data.rxbuf[0] == TURAG_FELDBUS_BROADCAST_ADDR))
    # else
        if (data.rxbuf[0] == MY_ADDR)
    # endif
    #elif TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 2
    # if TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE
        if (((data.rxbuf[0] == (MY_ADDR & 0xff) && data.rxbuf[1] == (MY_ADDR >> 8)) ||
            (data.rxbuf[0] == (TURAG_FELDBUS_BROADCAST_ADDR_2 & 0xff) && data.rxbuf[1] == (TURAG_FELDBUS_BROADCAST_ADDR_2 >> 8))))
    # else
        if ((data.rxbuf[0] == (MY_ADDR & 0xff) && data.rxbuf[1] == (MY_ADDR >> 8)))
    # endif
    #endif
        {
            return true;
        } else {
            return false;
        }
    }

    // callbacks
    static void thread_func();
    static void rxChar(UARTDriver *, uint16_t c);
    static void rxComplete(GPTDriver *);
    static void rxErr(UARTDriver *, uartflags_t);
    static void txComplete(UARTDriver *);
    
    
    static HardwareConfig* config;
    static Data data;
    
#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
    static BinarySemaphore tx_sem;
    static BinarySemaphore tx_finished;
#endif

    static BinarySemaphore rx_sem;
};


}
}
}


#endif /* FELDBUS_SLAVE_DRIVER_H_ */
