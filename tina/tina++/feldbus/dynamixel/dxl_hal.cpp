
#include "dxl_hal.h"
#include <tina++/feldbus/host/feldbusabstraction.h>

static TURAG::Feldbus::FeldbusAbstraction* bus;


extern "C" int turag_dxl_hal_open(void* feldbusAbstractionInstance) {
    bus = (TURAG::Feldbus::FeldbusAbstraction*)feldbusAbstractionInstance;
    return (bus != 0);
}


extern "C" void turag_dxl_hal_close(void) {
    return;
}


extern "C" void turag_dxl_hal_clear(void) {
    bus->clearBuffer();
}

extern "C" int turag_dxl_hal_tx( unsigned char *pPacket, int numPacket ) {
    if (bus->transceive(pPacket, &numPacket, 0, 0, false)) {
        return numPacket;
    } else {
        return 0;
    }
}

extern "C" int turag_dxl_hal_rx( unsigned char *pPacket, int numPacket ) {
    if (bus->transceive(0, 0, pPacket, &numPacket, false)) {
        return numPacket;
    } else {
        return 0;
    }
}


extern "C" void turag_dxl_hal_set_timeout( int NumRcvByte ) {
    (void) NumRcvByte;
    return;
}

extern "C" int turag_dxl_hal_timeout(void) {
    return 1;
}
