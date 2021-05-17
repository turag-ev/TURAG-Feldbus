
#include "dxl_hal.h"
#include <tina++/feldbus/host/feldbusabstraction.h>

using namespace TURAG::Feldbus;

static FeldbusAbstraction* bus;


extern "C" int turag_dxl_hal_open(void* feldbusAbstractionInstance) {
	bus = (FeldbusAbstraction*)feldbusAbstractionInstance;
    return (bus != nullptr);
}


extern "C" void turag_dxl_hal_close(void) {
    return;
}


extern "C" void turag_dxl_hal_clear(void) {
    bus->clearBuffer();
}

extern "C" int turag_dxl_hal_tx( unsigned char *pPacket, int numPacket ) {
    if (bus->transceive(pPacket, &numPacket, nullptr, nullptr, 0xFF, ChecksumType::none) == FeldbusAbstraction::ResultStatus::Success) {
        return numPacket;
    } else {
        return 0;
    }
}

extern "C" int turag_dxl_hal_rx( unsigned char *pPacket, int numPacket ) {
    if (bus->transceive(nullptr, nullptr, pPacket, &numPacket, 0xFF, TURAG::Feldbus::ChecksumType::none) == FeldbusAbstraction::ResultStatus::Success) {
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
