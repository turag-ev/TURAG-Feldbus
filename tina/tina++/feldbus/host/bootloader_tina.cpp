/**
 *  @brief		TURAG feldbus bootloader base class
 *  @file		bootloader_tina.cpp
 *  @date		11.11.2014
 *  @author		Florian Voelker
 *
 */

#define TURAG_DEBUG_LOG_SOURCE "B"


#include "bootloader.h"

#include <tina++/tina.h>
#include <tina/debug.h>



namespace TURAG {
namespace Feldbus {

bool BMaxBootloader::transceiveBoot (uint8_t *transmit, int transmit_length, uint8_t *receive, int receive_length){

    if( this -> transceive(transmit, transmit_length, receive, receive_length) ) return true;
    else return false;

}

} // namespace Feldbus
} // namespace TURAG

