
#include "dxl_hal.h"
#include <tina/rs485.h>


int turag_dxl_hal_open(int deviceIndex, float baudrate) {
    (void) deviceIndex;
    (void) baudrate;
    return 1;
}


void turag_dxl_hal_close(void) {
    return;
}


void turag_dxl_hal_clear(void) {
    return;
}

int turag_dxl_hal_tx( unsigned char *pPacket, int numPacket )
{
    if (turag_rs485_transceive (pPacket, numPacket, 0, 0)) {
	return numPacket;
    } else {    
	return 0;
    }
}

int turag_dxl_hal_rx( unsigned char *pPacket, int numPacket )
{
    if (turag_rs485_transceive (0, 0, pPacket, numPacket)) {
	return numPacket;
    } else {    
	return 0;
    }
}


void turag_dxl_hal_set_timeout( int NumRcvByte ) {
    (void) NumRcvByte;
    return;
}

int turag_dxl_hal_timeout(void) {
    return 1;
}
