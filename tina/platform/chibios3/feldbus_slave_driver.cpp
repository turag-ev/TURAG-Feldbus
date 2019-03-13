/*
 * feldbus-driver.cpp
 *
 *  Created on: 20.11.2016
 *      Author: martin
 */

#include <tina++/tina.h>

#if TURAG_USE_TURAG_FELDBUS_SLAVE


#include <tina++/feldbus_slave_driver.h>
#include <tina++/feldbus/slave/feldbus_slave_base.h>

#include <cstring>

#include <ch.h>
#include <hal.h>


namespace TURAG {
namespace Feldbus {
namespace Slave {



#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
BSEMAPHORE_DECL(Driver::tx_sem, false);
BSEMAPHORE_DECL(Driver::tx_finished, true);
#endif

BSEMAPHORE_DECL(Driver::rx_sem, true);

const Driver::HardwareConfig* Driver::config = nullptr;
UARTConfig Driver::uart_config;
GPTConfig Driver::gpt_config;
Driver::Data Driver::data = {
    0,
    false, false,
    {0}, {0},
    0,
    false,
};




void Driver::init(const HardwareConfig* config_) {
    config = config_;

    memset(&uart_config, 0, sizeof(UARTConfig));
    memset(&gpt_config, 0, sizeof(GPTConfig));

    uart_config.speed = config->baudrate;
    uart_config.txend2_cb = txComplete;
    uart_config.rxerr_cb = rxErr;

    if(config->rto_config) {
        //software receive timeout
        uart_config.rxchar_cb = rxChar;
        gpt_config.callback = rxTimeoutSoftware;
        gpt_config.frequency = config->rto_config->gpt_frequency;
        data.timerTicks = (gpt_config.frequency * 15 + config->baudrate / 2) / config->baudrate;
    } else {
        //hardware receive timeout
        uart_config.timeout_cb = rxTimeoutHardware;
        uart_config.cr1 = USART_CR1_RTOIE;
        uart_config.cr2 = USART_CR2_RTOEN;
        uart_config.timeout = 15;
    }
    if(!config->rts) {
        //hardware Driver-Enable
        uart_config.cr3 = USART_CR3_DEM;
        if(config->rts_inverted)
            uart_config.cr3 |= USART_CR3_DEP;
    }

#if TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 1
    data.txbuf[0] = TURAG_FELDBUS_MASTER_ADDR|MY_ADDR;
#elif TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH == 2
    data.txbuf[0] = (TURAG_FELDBUS_MASTER_ADDR_2|MY_ADDR) & 0xff;
    data.txbuf[1] = (TURAG_FELDBUS_MASTER_ADDR_2|MY_ADDR) >> 8;
#endif
    //always do this, since UART Driver is not running yet, controlling the pin
    disableRts();
    if(config->led_inverted) {
        palSetLine(config->led);
    } else {
        palClearLine(config->led);
    }
}


void Driver::start(ThreadImpl* thread, int prio) {
    if(config->rto_config)
        gptStart(config->rto_config->gptd, &gpt_config);
    uartStart(config->uartd, &uart_config);
    thread->start(prio, &thread_func);
}


#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
void Driver::transmitDebugData(const void* data, size_t length) {
    chBSemWait(&tx_sem);
    enableRts();
    uartStartSend(config->uartd, length, data);
    chBSemWait(&tx_finished);
}
#endif


void blinkCallback (void *t) {
    Base::doLedPattern(50);
    chSysLockFromISR();
    chVTSetI(reinterpret_cast<virtual_timer_t*>(t), MS2ST(20), blinkCallback, t);
    chSysUnlockFromISR();
}

void Driver::thread_func() {
    chRegSetThreadName("feldbus slave driver");

    if(!config->rto_config) {
        //we can use DMA for receiving
        //use virtual timers for blinking. we could use a 20ms timeout for receive,
        //but we might miss an incoming byte while were busy blinking :)
        virtual_timer_t vt;
        chVTObjectInit(&vt);
        chVTSet(&vt, MS2ST(20), blinkCallback, &vt);
        while(1) {
            
            size_t n = TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE;
            uartReceiveTimeout(config->uartd, &n, data.rxbuf, TIME_INFINITE);
            if(n <= TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH || !packetAdressedToMe()) //packet must at least contain address and checksum
                continue;
            Base::doLedPattern(50);
            FeldbusSize_t length = Base::processPacket(data.rxbuf, data.rx_size, data.txbuf);
            if (length > 0) {
                size_t l = length;
                enableRts();
                uartSendFullTimeout(config->uartd, &l, data.txbuf, TIME_INFINITE);
                disableRts();
            }
        }
    }
    while(1) {
        chSysLock();
        // Reset packet data after it was processed
        // at the end of this function. We do this here
        // to save a call to chSysLockFromISR().
        if (data.processing) {
            data.processing = false;
            data.packet_complete = false;
            data.rx_size = 0;
        }

        // If there is a new packet waiting, we start working on it.
        // Otherwise we go to sleep. We need to check new packets before
        // going to sleep to cover the rare case where we received a new
        // one while doing the led pattern.
        if (data.packet_complete) {
            data.processing = true;
        } else {
            // Sleep until there is a packet waiting to be processed
            // or we need to toggle our led.
            chBSemWaitTimeoutS(&rx_sem, MS2ST(20));

            // Check whether there is a new packet. We need to do this while
            // we are still in syslock to prevent the packet getting overwritten
            // resulting in the loss of the current one.
            if (data.packet_complete) {
                data.processing = true;
            }
        }
        chSysUnlock();

#if TURAG_FELDBUS_SLAVE_CONFIG_FLASH_LED
        // Heavy traffic will speed up the led pattern.
        // That's a feature, not a bug.
        // We blink with 50 Hz because we wait for 20 ms a few lines above.
        Base::doLedPattern(50);
#endif

        if (data.processing) {
            FeldbusSize_t length = Base::processPacket(data.rxbuf, data.rx_size, data.txbuf);
            if (length > 0) {
#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
                chBSemWait(&tx_sem);
#endif
                enableRts();
                uartStartSend(config->uartd, length, data.txbuf);
            }
        }
    }
}


// new receiption has started
void Driver::rxChar(UARTDriver *, uint16_t c) {
    chSysLockFromISR();
    if (!data.processing) {
        if (data.packet_complete) {
            // packet was ready, but there is a new one before
            // it was processed -> dump it and receive new one
            data.rx_size = 0;
            data.packet_complete = false;
#if (TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE)
            Base::increasePacketLost();
#endif
        }

        if (data.rx_size == TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE) {
            // Buffer overflow.
            // No need to reset rx_size here, because we stop filling
            // rxbuf, once it is full. This is important to correctly
            // identify whether the packet was meant for us.
            data.overflow = true;
        } else {
            data.rxbuf[data.rx_size] = static_cast<uint8_t>(c);
            ++data.rx_size;
        }
        // restart timer to wait for more arriving data
        if (config->rto_config->gptd->state != GPT_READY)
            gptStopTimerI(config->rto_config->gptd);
        gptStartOneShotI(config->rto_config->gptd, data.timerTicks);
    } else {
        // Byte received, but package is processing -> ignore.
        // This shouldn't happen if the data is really meant
        // for us, unless we exceeded the masters timeout and it
        // resent its request.
    }
    chSysUnlockFromISR();
}

//hardware and software timeout call common handler code
void Driver::rxTimeoutHardware(UARTDriver* d) {
    chSysLockFromISR();
    data.rx_size = TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE - dmaStreamGetTransactionSize(d->dmarx);
    uartStopReceiveI(d);
    rxCompleteI();
    chSysUnlockFromISR();
}

void Driver::rxTimeoutSoftware(GPTDriver*) {
    chSysLockFromISR();
    rxCompleteI();
    chSysUnlockFromISR();
}

//call only from lockzone
inline void Driver::rxCompleteI() {
    if (packetAdressedToMe()) {
        if (data.overflow) {
#if (TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE)
            // Only increase the overflow counter if the
            // packet was addressed to us.
            Base::increaseBufferOverflow();
#endif
        } else if (data.rx_size > TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH) {
            // valid packet, addressed to us
            data.packet_complete = true;
            // wake up worker thread
            chBSemSignalI(&rx_sem);
        }
    }
    // If either
    // - we had an overflow or
    // - the packet was not meant for us or
    // - the packet was too short
    // we get a complete reception bot no complete packet. Then we still need
    // to clear the packet buffer and the overflow flag which is potentially set.
    if (!data.packet_complete) {
        data.rx_size = 0;
        data.overflow = false;
    }
}


// usart hardware error detected
void Driver::rxErr(UARTDriver *, uartflags_t) {
    // TODO: Halt and catch fire
}


// dma transmit receive complete
void Driver::txComplete(UARTDriver *) {
    disableRts();

#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED
    chSysLockFromISR();
    chBSemSignalI(&tx_sem);
    chBSemSignalI(&tx_finished);
    chSysUnlockFromISR();
#endif
}


} // namespace Slave
} // namespace Feldbus
} // namespace TURAG


#endif // TURAG_USE_TURAG_FELDBUS_SLAVE
