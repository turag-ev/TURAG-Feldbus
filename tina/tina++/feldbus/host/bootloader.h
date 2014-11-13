/**
 *  @brief		TURAG feldbus bootloader base class
 *  @file		bootloader.h
 *  @date		11.11.2014
 *  @author		Florian Voelker
 *
 */


#ifndef TURAGFELDBUSBOOTLOADER_H_
#define TURAGFELDBUSBOOTLOADER_H_

#include "device.h"
#include <tina++/tina.h>
#include <tina/feldbus/protocol/turag_feldbus_fuer_bootloader.h>


namespace TURAG {
namespace Feldbus {

class Bootloader : public TURAG::Feldbus::Device {

public:
    Bootloader(TURAG::Feldbus::Device *dev_);
    bool transceiveBoot (uint8_t *transmit, int transmit_length, uint8_t *receive, int receive_length);

protected:
    

private:

};

} // namespace Feldbus
} // namespace TURAG

#endif /* TURAGFELDBUSASEB_H_ */
