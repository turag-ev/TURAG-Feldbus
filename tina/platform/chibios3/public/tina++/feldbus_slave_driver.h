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

// should there ever be a need to run more than one slave
// device on one board, all static functions
// could be replaced with normal ones.
/**
 * @brief Feldbus-Slave Treiber für ChibiOS 3.
 * 
 * Dieser Treiber implementiert den plattform-abhängigen Teil des Feldbus-Slave
 * Treibers für die ChibiOS 3-Plattform.
 */
class Driver {
public:
	/**
	 * @brief Feldbus-Konfigurationsstruktur.
     * 
     * Enthält sämtliche Informationen die für einen Betrieb als Feldbus-Slave
     * in ChibiOS nötig sind.
	 */
    struct HardwareConfig {
        /// Zu verwendender UART-Treiber.
        UARTDriver *uartp;
        /// UART-Config.
        UARTConfig uartConfig;
        /// Baudrate. Muss hier und in uartConfig angegeben werden.
        uint32_t baudrate;		// must be set in uartConfig and in baudrate
        /// Zu verwendender GPTimer.
        GPTDriver * gptp;
        /// GPTimer Config.
        GPTConfig gptConfig;
        /// Port an dem die LED angeschlossen ist.
        ioportid_t ledPort;
        /// Pin der LED.
        unsigned ledPin;
        /// Gibt an, ob das LED-Signal invertiert werden soll.
        bool ledInverted;
        /// Port an dem das RTS-Signal angeschlossen ist.
        ioportid_t rtsPort;
        /// Pin des RTS-Signals.
        unsigned rtsPin;
        /// Gibt an, ob das RTS-Signal invertiert werden soll.
        bool rtsInverted;
    };


    // sets up hardware-specific stuff
    // should be called in hardware-dependent source-file
    /**
     * @brief Initialisiert die Hardware.
     * @param[in] config Konfigurationsstruktur.
     * 
     * Da diese Funktion einen Pointer auf eine plattform-abhängige
     * Struktur benötigt, sollte sie im plattform-abhängigen Teil der 
     * Applikation aufgerufen werden.
     */
    static void init(HardwareConfig* config);

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
    	if (config->ledPort) {
    		palTogglePad(config->ledPort, config->ledPin);
    	}
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
    static binary_semaphore_t tx_sem;
    static binary_semaphore_t tx_finished;
#endif

    static binary_semaphore_t rx_sem;
};


}
}
}


#endif /* FELDBUS_SLAVE_DRIVER_H_ */
