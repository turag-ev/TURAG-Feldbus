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
	

	Aseb(const char* name_, unsigned int address, ChecksumType type = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE) :
                Device(name_, address, type),
                analogInputs_(nullptr), analogInputSize_(-1),
                pwmOutputs_(nullptr), pwmOutputSize_(-1),
                digitalInputs_(0), digitalInputSize_(-1),
                digitalOutputs_(0), digitalOutputSize_(-1),
                syncBuffer_(nullptr), syncSize_(0), isSynced_(false)  { }
                
                
    bool initialize(uint8_t* sync_buffer, unsigned sync_buffer_size,
                    Analog_t* analogInputs, unsigned analogInputSize,
                    Pwm_t* pwmOutputs, unsigned pwmOutputSize );

    bool isSynced(void) { return isSynced_; }

    bool getDigitalOutputSize(int* size);
    bool getDigitalInputSize(int* size);
    bool getAnalogInputSize(int * size);
    bool getPwmOutputSize(int* size);
    bool getSyncSize(int* size);

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
    bool getAnalogResolution(unsigned* resolution);
	
protected:
    Analog_t* analogInputs_;
    int analogInputSize_;

    Pwm_t* pwmOutputs_;
    int pwmOutputSize_;

    uint16_t digitalInputs_;
    int digitalInputSize_;

    uint16_t digitalOutputs_;
    int digitalOutputSize_;

    uint8_t* syncBuffer_;
    int syncSize_;

    bool isSynced_;

    bool initDigitalOutputBuffer(void);
    bool initPwmOutputBuffer(void);
	
};

} // namespace Feldbus
} // namespace TURAG

#endif /* TURAGFELDBUSASEB_H_ */
