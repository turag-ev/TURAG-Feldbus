/**
 *  @brief		TURAG feldbus aseb base class
 *  @file		aseb_tina.cpp
 *  @date		11.01.2014
 *  @author		Martin Oemus
 *
 */

#define TURAG_DEBUG_LOG_SOURCE "B"


#include "aseb.h"

#include <tina++/tina.h>
#include <tina/debug.h>



namespace TURAG {
namespace Feldbus {

struct AsebGetInfo {
    uint8_t command;
    uint8_t index;
} _packed;

struct AsebSet {
    uint8_t index;
    uint16_t value;
} _packed;

struct AsebSetDigital {
    uint8_t index;
    uint8_t value;
} _packed;

struct AsebGetAnalogFactor {
    float factor;
} _packed;

struct AsebGetPwmMaxValue {
    uint16_t maxValue;
} _packed;

struct AsebGetPwmValue {
    uint16_t value;
} _packed;

struct AsebGetPwmFrequency {
    uint32_t frequency;
} _packed;


bool Aseb::initialize(uint8_t* sync_buffer, unsigned sync_buffer_size,
                      Analog_t* analogInputs, unsigned analogInputSize,
                      Pwm_t* pwmOutputs, unsigned pwmOutputSize) {

    syncSize_ = 0;

    int sizeBuffer = 0;
    if (!getDigitalInputSize(&sizeBuffer)) return false;
    if (!getDigitalOutputSize(&sizeBuffer)) return false;


    if (analogInputs) {
        if (!getAnalogInputSize(&sizeBuffer)) return false;
        if (sizeBuffer > static_cast<int>(analogInputSize)) {
			turag_errorf("%s: analogInputSize must be %d (is %d)", name, sizeBuffer, analogInputSize);
			return false;
		}
			
        if (static_cast<int>(analogInputSize) > sizeBuffer) {
            turag_warningf("%s: analog input buffer larger than required (%d > %d)", name, analogInputSize, sizeBuffer);
        }

        analogInputs_ = analogInputs;

        Request<AsebGetInfo> request;
        request.data.command = TURAG_FELDBUS_ASEB_ANALOG_INPUT_FACTOR;

        Response<AsebGetAnalogFactor> response;

        for (int i = 0; i < analogInputSize_; ++i) {
            request.data.index = i + TURAG_FELDBUS_ASEB_INDEX_START_ANALOG_INPUT;

            if (!transceive(request, &response)) return false;
            analogInputs_[i].factor = response.data.factor;
        }
    } else {
        analogInputSize_ = 0;
		if (analogInputSize != 0) {
			turag_warningf("%s: analogInputs is null, but analogInputSize != 0", name);
		}
    }

    if (pwmOutputs) {
        if (!getPwmOutputSize(&sizeBuffer)) return false;
        if (sizeBuffer > static_cast<int>(pwmOutputSize)) {
			turag_errorf("%s: pwmOutputSize must be %d (is %d)", name, sizeBuffer, pwmOutputSize);
			return false;
		}

        if (static_cast<int>(pwmOutputSize) > sizeBuffer) {
            turag_warningf("%s: pwm output buffer larger than required (%d > %d)", name, pwmOutputSize, sizeBuffer);
        }

        pwmOutputs_ = pwmOutputs;

        Request<AsebGetInfo> request;
        request.data.command = TURAG_FELDBUS_ASEB_PWM_OUTPUT_MAX_VALUE;

        Response<AsebGetPwmMaxValue> response;

        for (int i = 0; i < pwmOutputSize_; ++i) {
            request.data.index = i + TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT;

            if (!transceive(request, &response)) return false;
            pwmOutputs_[i].max_value = static_cast<float>(response.data.maxValue);
        }
    } else {
        pwmOutputSize_ = 0;
		if (pwmOutputSize != 0) {
			turag_warningf("%s: pwmOutputs is null, but pwmOutputSize != 0", name);
		}
    }

    if (!sync_buffer) {
		turag_errorf("%s: sync_buffer is null", name);
		return false;
    }
    if (!getSyncSize(&sizeBuffer)) return false;
    if (static_cast<int>(sync_buffer_size) < sizeBuffer) {
        turag_errorf("%s: sync buffer size must be %d (is %d)", name, sizeBuffer, sync_buffer_size);
        return false;
    }
    if (static_cast<int>(sync_buffer_size) > sizeBuffer) {
		turag_warningf("%s: sync buffer larger than required (%d > %d)", name, sync_buffer_size, sizeBuffer);
    }
    syncBuffer_ = sync_buffer;

    if (!initDigitalOutputBuffer()) return false;
    if (!initPwmOutputBuffer()) return false;

    return true;
}
	

bool Aseb::getDigitalOutputSize(int* size) {
    if (digitalOutputSize_ == -1) {
        Request<uint8_t> request;
        request.data = TURAG_FELDBUS_ASEB_NUMBER_OF_DIGITAL_OUTPUTS;

        Response<uint8_t> response;

        if (!transceive(request, &response)) return false;

        digitalOutputSize_ = response.data;
    }
    *size = digitalOutputSize_;
    return true;
}

bool Aseb::getDigitalInputSize(int* size) {
    if (digitalInputSize_ == -1) {
        Request<uint8_t> request;
        request.data = TURAG_FELDBUS_ASEB_NUMBER_OF_DIGITAL_INPUTS;

        Response<uint8_t> response;

        if (!transceive(request, &response)) return false;

        digitalInputSize_ = response.data;
    }
    *size = digitalInputSize_;
    return true;
}

bool Aseb::getAnalogInputSize(int * size) {
    if (analogInputSize_ == -1) {
        Request<uint8_t> request;
        request.data = TURAG_FELDBUS_ASEB_NUMBER_OF_ANALOG_INPUTS;

        Response<uint8_t> response;

        if (!transceive(request, &response)) return false;

        analogInputSize_ = response.data;
    }
    *size = analogInputSize_;
    return true;
}

bool Aseb::getPwmOutputSize(int* size) {
    if (pwmOutputSize_ == -1) {
        Request<uint8_t> request;
        request.data = TURAG_FELDBUS_ASEB_NUMBER_OF_PWM_OUTPUTS;

        Response<uint8_t> response;

        if (!transceive(request, &response)) return false;

        pwmOutputSize_ = response.data;
    }
    *size = pwmOutputSize_;
    return true;
}


bool Aseb::getSyncSize(int* size) {
    if (!syncSize_) {
        Request<uint8_t> request;
        request.data = TURAG_FELDBUS_ASEB_SYNC_SIZE;

        Response<uint8_t> response;

        if (!transceive(request, &response)) return false;

        syncSize_ = response.data;
    }
    *size = syncSize_;
    return true;
}



bool Aseb::sync(void) {
    isSynced_ = false;

    if (!syncSize_) return false;

    // sync only if there are inputs available
    if (syncSize_ > 2) {
		uint8_t request[myAddressLength + 1 + 1];
		request[myAddressLength] = TURAG_FELDBUS_ASEB_SYNC;

        if (!transceive(request,
                        sizeof(request),
                        syncBuffer_,
                        syncSize_)) {
            return false;
        }

        uint8_t* response = syncBuffer_ + myAddressLength;

        if (digitalInputSize_ > 0) {
            digitalInputs_ = *(reinterpret_cast<uint16_t*>(response));
            response += 2;
        }

        if (analogInputs_) {
            for (int i = 0; i < analogInputSize_; ++i) {
                analogInputs_[i].value = *(reinterpret_cast<int16_t*>(response));
                response += 2;
            }
        }
    }

    isSynced_ = true;
    return true;
}

float Aseb::getAnalogInput(unsigned key) {
    if (!syncSize_ || !analogInputs_ || analogInputSize_ < 0) {
        turag_errorf("%s: tried to call Aseb::getAnalogInput prior to initialization", name);
        return 0.0f;
    } else if (key >= static_cast<unsigned>(analogInputSize_)) {
        turag_errorf("%s: Wrong arguments to getAnalogInput. Key must be in the range of 0 to %u (given %u).", name, static_cast<unsigned>(analogInputSize_) - 1, key);
        return 0.0f;
    } else {
        return static_cast<float>(analogInputs_[key].value) * analogInputs_[key].factor;
    }
}

bool Aseb::getDigitalInput(unsigned key) {
    if (!syncSize_ || digitalInputSize_ < 0) {
        turag_errorf("%s: tried to call Aseb::getDigitalInput prior to initialization", name);
        return false;
    } else if (key >= static_cast<unsigned>(digitalInputSize_)) {
        turag_errorf("%s: Wrong arguments to getDigitalInput. Key must be in the range of 0 to %d (given %d).", name, static_cast<unsigned>(digitalInputSize_) - 1, key);
        return false;
    } else {
        return static_cast<bool>(digitalInputs_ & (1<<key));
    }
}

bool Aseb::getDigitalOutput(unsigned key) {
    if (!syncSize_ || digitalOutputSize_ < 0) {
        turag_errorf("%s: tried to call Aseb::getDigitalOutput prior to initialization", name);
        return false;
    } else if (key >= static_cast<unsigned>(digitalOutputSize_)) {
        turag_errorf("%s: Wrong arguments to getDigitalOutput. Key must be in the range of 0 to %d (given %d).", name, static_cast<unsigned>(digitalOutputSize_) - 1, key);
        return false;
    } else {
        return static_cast<bool>(digitalOutputs_ & (1<<key));
    }
}

bool Aseb::initDigitalOutputBuffer() {
    int size;
    if (!getDigitalOutputSize(&size)) {
        turag_errorf("%s: call to Aseb::initDigitalOutputBuffer() failed", name);
        return false;
    }

    Request<uint8_t> request;
    Response<uint8_t> response;

    for (int i = 0; i < size; ++i) {
        request.data = i + TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_OUTPUT;
        if (!transceive(request, &response)) return false;
        if (response.data) digitalOutputs_ |= (1<<i);
    }
    return true;
}

bool Aseb::setDigitalOutput(unsigned key, bool value) {
    if (!syncSize_ || digitalOutputSize_ < 0) {
        turag_errorf("%s: tried to call Aseb::setDigitalOutput prior to initialization", name);
        return false;
    } else if (key >= static_cast<unsigned>(digitalOutputSize_)) {
        turag_errorf("%s: Wrong arguments to setDigitalOutput. Key must be in the range of 0 to %d (given %d).", name, static_cast<unsigned>(digitalOutputSize_) - 1, key);
        return false;
    } else {
        uint16_t temp = digitalOutputs_;

        if (value) {
            temp |= (1<<key);
        } else {
            temp &= ~(1<<key);
        }

        Request<AsebSetDigital> request;
        request.data.index = key + TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_OUTPUT;
        request.data.value = value;

        Response<> response;

        if (!transceive(request, &response)) return false;

        digitalOutputs_ = temp;

        return true;
    }
}

float Aseb::getPwmOutput(unsigned key) {
    if (!syncSize_ || !pwmOutputs_ || pwmOutputSize_ < 0) {
        turag_errorf("%s: tried to call Aseb::getPwmOutput prior to initialization", name);
        return 0.0f;
    } else if (key >= static_cast<unsigned>(pwmOutputSize_)) {
        turag_errorf("%s: Wrong arguments to getPwmOutput. Key must be in the range of 0 to %d (given %d).", name, static_cast<unsigned>(pwmOutputSize_) - 1, key);
        return 0.0f;
    } else {
        return pwmOutputs_[key].value;
    }
}

bool Aseb::initPwmOutputBuffer() {
    int size;
    if (!getPwmOutputSize(&size)) {
        turag_errorf("%s: call to Aseb::initPwmOutputBuffer() failed", name);
        return false;
    }

    if (size > 0 && !pwmOutputs_) {
        turag_errorf("%s: call to Aseb::initPwmOutputBuffer() failed", name);
        return false;
    }

    Request<uint8_t> request;
    Response<AsebGetPwmValue> response;

    for (int i = 0; i < size; ++i) {
        request.data = i + TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT;
        if (!transceive(request, &response)) return false;
        pwmOutputs_[i].value = static_cast<float>(response.data.value) * 100.0f / pwmOutputs_[i].max_value;
    }
    return true;
}

bool Aseb::setPwmOutput(unsigned key, float duty_cycle) {
    if (!syncSize_ || !pwmOutputs_ || pwmOutputSize_ < 0) {
        turag_errorf("%s: tried to call Aseb::setPwmOutput prior to initialization", name);
        return false;
    } else if (key >= static_cast<unsigned>(pwmOutputSize_)) {
        turag_errorf("%s: Wrong arguments to setPwmOutput. Key must be in the range of 0 to %d (given %d).", name, static_cast<unsigned>(pwmOutputSize_) - 1, key);
        return false;
    } else {
        Request<AsebSet> request;
        request.data.index = key + TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT;
        request.data.value = static_cast<uint16_t>(duty_cycle / 100.0f * pwmOutputs_[key].max_value + 0.5f);

        Response<> response;
        if (!transceive(request, &response)) {
            turag_errorf("%s: Aseb setPWMOutput transceive failed", name);
            return false;
        }

        pwmOutputs_[key].value = duty_cycle;
        return true;
    }
}


bool Aseb::getPwmFrequency(unsigned key, uint32_t* frequency) {
    if (!syncSize_ || !pwmOutputs_ || pwmOutputSize_ < 0) {
        turag_errorf("%s: tried to call Aseb::getPwmFrequency prior to initialization", name);
        return false;
    } else if (key >= static_cast<unsigned>(pwmOutputSize_)) {
        turag_errorf("%s: Wrong arguments to getPwmFrequency. Key must be in the range of 0 to %d (given %d).", name, static_cast<unsigned>(pwmOutputSize_) - 1, key);
        return false;
    } else {
        Request<AsebGetInfo> request;
        request.data.command = TURAG_FELDBUS_ASEB_PWM_OUTPUT_FREQUENCY;
        request.data.index = key + TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT;

        Response<AsebGetPwmFrequency> response;

        if (!transceive(request, &response)) return false;

        *frequency = response.data.frequency;

        return true;
    }
}


unsigned int Aseb::getCommandNameLength(unsigned key) {
    if (!syncSize_) {
        turag_errorf("%s: tried to call Aseb::getCommandNameLength prior to initialization", name);
        return false;
    }

    Request<AsebGetInfo> request;
    request.data.command = TURAG_FELDBUS_ASEB_CHANNEL_NAME_LENGTH;
    request.data.index = key;

    Response<uint8_t> response;

    if (!transceive(request, &response)) {
        return 0;
    } else {
        return response.data;
    }
}


bool Aseb::getCommandName(unsigned key, char* out_name) {
    if (!out_name) {
        return false;
    }

    unsigned name_length = getCommandNameLength(key);
    if (name_length == 0) {
        *out_name = 0;
        return false;
    }

	uint8_t request[myAddressLength + 2 + 1];
	request[myAddressLength] = TURAG_FELDBUS_ASEB_CHANNEL_NAME;
	request[myAddressLength + 1] = key;

    if (!transceive(request,
                    sizeof(request),
                    reinterpret_cast<uint8_t*>(out_name),
                    name_length + myAddressLength + 1)) {
        return false;
    }

    for (unsigned i = 0; i < name_length; ++i) {
        out_name[i] = out_name[i + myAddressLength];
    }
    out_name[name_length] = 0;

    return true;

}

bool Aseb::getAnalogResolution(unsigned* resolution) {
    Request<uint8_t> request;
    request.data = TURAG_FELDBUS_ASEB_ANALOG_INPUT_RESOLUTION;

    Response<uint8_t> response;

    if (!transceive(request, &response)) return false;

    *resolution = response.data;

    return true;
}

	
} // namespace Feldbus
} // namespace TURAG

