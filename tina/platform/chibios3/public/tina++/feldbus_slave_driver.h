/*
 * feldbus-driver.h
 *
 *  Created on: 20.11.2016
 *      Author: martin
 *
 *      This is a tina-platform header. As such its interface
 *      should be platform-independent, which it isn't 
 *      (because init() requires a pointer to a platform-specific
 *      structure).
 *      To get the most flexibility, init() should be called
 *      in platform code and start() in generic code.
 */

#ifndef FELDBUS_SLAVE_DRIVER_H_
#define FELDBUS_SLAVE_DRIVER_H_


#include <tina++/tina.h>
#include <tina++/thread.h>
#include <tina/feldbus/slave/feldbus_config_check.h>

#include <ch.h>
#include <hal.h>


namespace TURAG {
namespace Feldbus {
namespace Slave {

/**
 * @brief Feldbus-Slave Treiber für ChibiOS 4.
 * 
 * Dieser Treiber implementiert den plattform-abhängigen Teil des Feldbus-Slave
 * Treibers für die ChibiOS 4-Plattform.
 */
class Driver {
public:
    /**
     * @brief Konfiguration für Software-Receive-Timeout.
     */
    struct RTOConfig {
	GPTDriver* gptd; /// Zu verwendender General-Purpose-Timer.
	uint32_t gpt_frequency; /// Frequenz des Timers.
    };

    struct HardwareConfig {
	/// Zu verwendender UART-Treiber.
	UARTDriver *uartd;
	/// Baudrate. 
	uint32_t baudrate;
	/// Konfiguration für Software-Receive-Timeout oder NULL für Hardware-Receive-Timeout.
	RTOConfig* rto_config;
	/// Port/Pin an dem die LED angeschlossen ist.
	ioline_t led;
	/// Gibt an, ob das LED-Signal invertiert werden soll.
	bool led_inverted;
	/// Port/Pin an dem das RTS-Signal (Driver-Enable) angeschlossen ist.
	ioline_t rts;
	/// Gibt an, ob das RTS-Signal invertiert werden soll.
	bool rts_inverted;
	/// Gibt an, ob das RTS-Signal in Software gesteuert werden soll.
	bool rts_software;
    };


    /**
     * @brief Initialisiert die Hardware.
     * 
     */
    static void init(const HardwareConfig*);

    /**
     * @brief Startet den Betrieb des Feldbus-Slave-Treibers.
     * @param[in] thread Zeiger auf einen Arbeitsthread, der noch nicht gestartet wurde.
     * @param[in] prio Priorität, mit der der Arbeitsthread gestartet werden soll.
     * 
     * Diese Funktion sollte nach Slave::Driver::init() und Slave::Base::init() aufgerufen werden.
     * Sie startet den Datenempfang und den %Thread, der die empfangenen Daten verarbeitet.
     */
    static void start(ThreadImpl* thread, int prio);

    /**
     * @brief Schaltet die konfigurierte LED um.
     * 
     * Diese Funktion schaltet den Zustand der in init() konfigurierten LED um.
     * Sie wird von Slave::Base benötigt.
     */
    static void toggleLed(void) {
	palToggleLine(config->led);
    }
#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED || defined(__DOXYGEN__)
    /**
    * @brief Sendet blockierend beliebige Daten z.B. zum debuggen.
    * @param[in] data Zu sendende Daten.
    * @param[in] length Länge des Datensatzes.
    * 
    * Diese Funktion wird von Slave::printf() und Slave::puts() zur Ausgabe
    * von Debug-Informationen benutzt.
    */
    static void transmitDebugData(const void* data, size_t length);
#endif
private:
    //prevent instantiation
    Driver() { }
    
    struct Data {
        FeldbusSize_t rx_size;
        bool processing;    // set and cleared by thread
        bool packet_complete; // set isr, cleared by thread and isr
        uint8_t txbuf[TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE];
        uint8_t rxbuf[TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE];

        uint32_t timerTicks;
        bool overflow;
    };

    static void enableRts(void) {
        if (config->rts_inverted) {
            palClearLine(config->rts);
        } else {
            palSetLine(config->rts);
        }
    }

    static void disableRts(void) {
        if (config->rts_inverted) {
            palSetLine(config->rts);
        } else {
            palClearLine(config->rts);
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
    static void rxTimeoutSoftware(GPTDriver*);
    static void rxTimeoutHardware(UARTDriver*);
    static inline void rxComplete();
    static void rxErr(UARTDriver *, uartflags_t);
    static void txComplete(UARTDriver *);

    static const HardwareConfig* config;
    static UARTConfig uart_config;
    static GPTConfig gpt_config;
    static Data data;
    
#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
    static binary_semaphore_t tx_sem;
    static binary_semaphore_t tx_finished;
#endif

    static binary_semaphore_t rx_sem;
};


}
}
}


#endif /* FELDBUS_SLAVE_DRIVER_H_ */
