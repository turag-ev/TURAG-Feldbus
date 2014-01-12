/**
 *  @brief		TURAG feldbus aseb base class
 *  @file		aseb.h
 *  @date		11.01.2014
 *  @author		Martin Oemus
 *
 */


#ifndef TURAGFELDBUSASEB_H_
#define TURAGFELDBUSASEB_H_

#include "device.h"
#include <tina++/tina.h>
#include <tina/feldbus/protocol/turag_feldbus_fuer_aseb.h>


namespace TURAG {
namespace Feldbus {

    
/*
 *
 */
class Aseb : public TURAG::Feldbus::Device {
public:
	struct Analog_t {
		float factor;
        int value;

        Analog_t() : factor(0), value(0) {}
	};
	
	struct Pwm_t {
        float max_value;

        /// [%]
        float value;

        Pwm_t() : max_value(0), value(0) {}
	};
	

	
protected:
    Analog_t* analogInputs_;
    unsigned analogInputSize_;

    Pwm_t* pwmOutputs_;
    unsigned pwmOutputSize_;

    uint16_t digitalInputs_;
    unsigned digitalInputSize_;

    uint16_t digitalOutputs_;
    unsigned digitalOutputSize_;

    uint8_t* syncBuffer_;
    unsigned syncSize_;

    bool isSynced_;

	
public:

    Aseb(const char* name_, unsigned int address, ChecksumType type = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE) :
                Device(name_, address, type),
                analogInputs_(nullptr), analogInputSize_(0),
                pwmOutputs_(nullptr), pwmOutputSize_(0),
                digitalInputs_(0), digitalInputSize_(0),
                digitalOutputs_(0), digitalOutputSize_(0),
                syncBuffer_(nullptr), syncSize_(0), isSynced_(false)  { }
                
                
    bool initialize(uint8_t* sync_buffer, unsigned sync_buffer_size,
                    Analog_t* analogInputs, unsigned analogInputSize,
                    Pwm_t* pwmOutputs, unsigned pwmOutputSize );

    bool isSynced(void) { return isSynced_; }

    bool getDigitalOutputSize(unsigned* size);
    bool getDigitalInputSize(unsigned* size);
    bool getAnalogInputSize(unsigned * size);
    bool getPwmOutputSize(unsigned* size);

    bool sync(void);

    float getAnalogInput(unsigned key);
	bool getDigitalInput(unsigned key);
	
	bool getDigitalOutput(unsigned key);
    bool setDigitalOutput(unsigned key, bool value);
    float getPwmOutput(unsigned key);
	bool setPwmOutput(unsigned key, float duty_cycle);

    bool getCommandName(unsigned key, char* name);
    unsigned int getCommandNameLength(unsigned key);
    bool getPwmFrequency(unsigned key, uint32_t* frequency);
                      
};

} // namespace Feldbus
} // namespace TURAG

#endif /* TURAGFELDBUSASEB_H_ */
