#ifndef TURAG_FELDBUS_FUER_ESCON_H_
#define TURAG_FELDBUS_FUER_ESCON_H_

#include "turag_feldbus_bus_protokoll.h"

//command bytes
#define TURAG_FELDBUS_ESCON_SET_RPM 0x01

//command structures
struct TuragEsconSetRPM_t {
    int16_t value[2];
};

//response structures
struct TuragEsconStatus_t {
    uint8_t status;
    int16_t currentRPM[2];
    int16_t measuredCurrent[2];
};

//status bytes
#define TURAG_FELDBUS_ESCON_READY 0
#define TURAG_FELDBUS_ESCON_FAILURE 1

#endif
