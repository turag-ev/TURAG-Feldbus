/**
 *  @brief		TURAG feldbus DC motor class
 *  @file		dcmotor.h
 *  @date		22.03.2013
 *  @author		Martin Oemus
 * 
 */


#ifndef TURAGFELDBUSDCMOTOR_H_
#define TURAGFELDBUSDCMOTOR_H_

#include "servo.h"

namespace TURAG {
namespace Feldbus {

/*
 *
 */
class DCMotorBase : public TURAG::Feldbus::ServoBase {
protected:
    bool myHomecomingRequested;

public:

    DCMotorBase(const char* name_, int address, ChecksumType type = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE) :
        ServoBase(name_, address, type), myHomecomingRequested(false) {}

    virtual bool driveHome(float velocity);
    virtual bool enableControl();

};


class DCMotor : public DCMotorBase {
protected:
    AktorCommand_t command_set[19];

public:
    DCMotor(const char* name_, int address, ChecksumType type = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE) :
        DCMotorBase(name_, address, type) {}

    bool initialize(void) { return populateCommandSet(command_set, 19); }
};

} // namespace Feldbus
} // namespace TURAG

#endif /* TURAGFELDBUSDCMOTOR_H_ */


