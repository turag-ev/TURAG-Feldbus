/**
 *  @brief		TURAG feldbus aseb base class
 *  @file		aseb_tina.cpp
 *  @date		11.01.2014
 *  @author		Martin Oemus
 *
 */

#define TURAG_DEBUG_LOG_SOURCE "B"

#include <tina++/tina.h>
#if TURAG_USE_TURAG_FELDBUS_HOST

#include "aseb.h"

#include <tina/debug.h>
#include <cmath>


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

	// syncSize_ being 0 signals an error, because it will always be
	// at least one once the initialization was successfully completed.
    syncSize_ = 0;

    int sizeBuffer = 0;
    if (!getDigitalInputSize(&sizeBuffer)) return false;
    if (!getDigitalOutputSize(&sizeBuffer)) return false;


    if (analogInputs) {
        if (!getAnalogInputSize(&sizeBuffer)) return false;
        if (sizeBuffer > static_cast<int>(analogInputSize)) {
			turag_errorf("%s: analogInputSize must be %d (is %d)", name(), sizeBuffer, analogInputSize);
			return false;
		}
			
        if (static_cast<int>(analogInputSize) > sizeBuffer) {
            turag_warningf("%s: analog input buffer larger than required (%d > %d)", name(), analogInputSize, sizeBuffer);
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
			turag_warningf("%s: analogInputs is null, but analogInputSize != 0", name());
		}
    }

    if (pwmOutputs) {
        if (!getPwmOutputSize(&sizeBuffer)) return false;
        if (sizeBuffer > static_cast<int>(pwmOutputSize)) {
			turag_errorf("%s: pwmOutputSize must be %d (is %d)", name(), sizeBuffer, pwmOutputSize);
			return false;
		}

        if (static_cast<int>(pwmOutputSize) > sizeBuffer) {
            turag_warningf("%s: pwm output buffer larger than required (%d > %d)", name(), pwmOutputSize, sizeBuffer);
        }

        pwmOutputs_ = pwmOutputs;

        Request<AsebGetInfo> request;
        Response<AsebGetPwmMaxValue> max_value_response;
        Response<AsebGetPwmFrequency> frequency_response;

        for (int i = 0; i < pwmOutputSize_; ++i) {
            request.data.index = i + TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT;

            request.data.command = TURAG_FELDBUS_ASEB_PWM_OUTPUT_MAX_VALUE;
            if (!transceive(request, &max_value_response)) return false;
            request.data.command = TURAG_FELDBUS_ASEB_PWM_OUTPUT_FREQUENCY;
            if (!transceive(request, &frequency_response)) return false;

            pwmOutputs_[i].max_value = max_value_response.data.maxValue;
            pwmOutputs_[i].frequency = frequency_response.data.frequency;
        }
    } else {
        pwmOutputSize_ = 0;
		if (pwmOutputSize != 0) {
			turag_warningf("%s: pwmOutputs is null, but pwmOutputSize != 0", name());
		}
    }

    if (!sync_buffer) {
		turag_errorf("%s: sync_buffer is null", name());
		return false;
    }
    if (!getSyncSize(&sizeBuffer)) return false;
    if (static_cast<int>(sync_buffer_size) < sizeBuffer) {
        turag_errorf("%s: sync buffer size must be %d (is %d)", name(), sizeBuffer, sync_buffer_size);
        return false;
    }
    if (static_cast<int>(sync_buffer_size) > sizeBuffer) {
		turag_warningf("%s: sync buffer larger than required (%d > %d)", name(), sync_buffer_size, sizeBuffer);
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
	if (!isInitialized()) {
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

	if (!isInitialized()) {
		turag_errorf("%s: tried to call Aseb::sync prior to initialization", name());
		return false;
	}

    // sync only if there are inputs available
    if (syncSize_ > 2) {
        // Do not use whole buffer for one byte address
        const int request_len = addressLength() + 1 + 1;
		uint8_t request[2 + 1 + 1];
		request[addressLength()] = TURAG_FELDBUS_ASEB_SYNC;

        if (!transceive(request,
                        request_len,
						syncBuffer_,
                        syncSize_)) {
            return false;
        }

		uint8_t* response = syncBuffer_ + addressLength();

        if (digitalInputSize_ > 0) {
			digitalInputs_ = (response[1] << 8) + response[0];
            response += 2;
        }

        if (analogInputs_) {
            for (int i = 0; i < analogInputSize_; ++i) {
				analogInputs_[i].value = static_cast<int16_t>((response[1] << 8) + response[0]);
                response += 2;
            }
        }
    }

    isSynced_ = true;
    return true;
}

float Aseb::getAnalogInput(unsigned key) {
	if (!isInitialized()) {
        turag_errorf("%s: tried to call Aseb::getAnalogInput prior to initialization", name());
        return 0.0f;
	} else if (key >= static_cast<unsigned>(analogInputSize_)) {
        turag_errorf("%s: Wrong arguments to getAnalogInput. Key must be in the range of 0 to %u (given %u).", name(), static_cast<unsigned>(analogInputSize_) - 1, key);
        return 0.0f;
    } else {
        return static_cast<float>(analogInputs_[key].value) * analogInputs_[key].factor;
    }
}


float Aseb::getTargetPwmOutput(unsigned key) {
    if (!isInitialized()) {
        turag_errorf("%s: tried to call Aseb::getTargetPwmOutput prior to initialization", name());
        return 0.0f;
    } else if (key >= static_cast<unsigned>(pwmOutputSize_)) {
        turag_errorf("%s: Wrong arguments to getTargetPwmOutput. Key must be in the range of 0 to %u (given %u).", name(), static_cast<unsigned>(pwmOutputSize_) - 1, key);
        return 0.0f;
    } else {
        return pwmOutputs_[key].target;
    }
}

bool Aseb::getDigitalInput(unsigned key) {
	if (!isInitialized()) {
        turag_errorf("%s: tried to call Aseb::getDigitalInput prior to initialization", name());
        return false;
    } else if (key >= static_cast<unsigned>(digitalInputSize_)) {
        turag_errorf("%s: Wrong arguments to getDigitalInput. Key must be in the range of 0 to %d (given %d).", name(), static_cast<unsigned>(digitalInputSize_) - 1, key);
        return false;
    } else {
        return static_cast<bool>(digitalInputs_ & (1<<key));
    }
}

bool Aseb::getDigitalOutput(unsigned key) {
	if (!isInitialized()) {
        turag_errorf("%s: tried to call Aseb::getDigitalOutput prior to initialization", name());
        return false;
    } else if (key >= static_cast<unsigned>(digitalOutputSize_)) {
        turag_errorf("%s: Wrong arguments to getDigitalOutput. Key must be in the range of 0 to %d (given %d).", name(), static_cast<unsigned>(digitalOutputSize_) - 1, key);
        return false;
    } else {
        return static_cast<bool>(digitalOutputs_ & (1<<key));
    }
}

bool Aseb::initDigitalOutputBuffer() {
    int size;
    if (!getDigitalOutputSize(&size)) {
        turag_errorf("%s: call to Aseb::initDigitalOutputBuffer() failed", name());
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
	if (!isInitialized()) {
        turag_errorf("%s: tried to call Aseb::setDigitalOutput prior to initialization", name());
        return false;
    } else if (key >= static_cast<unsigned>(digitalOutputSize_)) {
        turag_errorf("%s: Wrong arguments to setDigitalOutput. Key must be in the range of 0 to %d (given %d).", name(), static_cast<unsigned>(digitalOutputSize_) - 1, key);
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

bool Aseb::getCurrentPwmOutput(unsigned key, float* duty_cycle) {
	if (!isInitialized()) {
        turag_errorf("%s: tried to call Aseb::getCurrentPwmOutput prior to initialization", name());
        return false;
    } else if (key >= static_cast<unsigned>(pwmOutputSize_) || !duty_cycle) {
        turag_errorf("%s: Wrong arguments to getPwmOutput. Key must be in the range of 0 to %d (given %d).", name(), static_cast<unsigned>(pwmOutputSize_) - 1, key);
        return false;
    } else {
        Request<uint8_t> request;
        Response<AsebGetPwmValue> response;
        request.data = TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT + key;
        if(!transceive(request, &response)) return false;
        *duty_cycle = static_cast<float>(response.data.value) * 100.0f / pwmOutputs_[key].max_value;
        return true;
    }
}

bool Aseb::initPwmOutputBuffer() {
    int size;
    if (!getPwmOutputSize(&size)) {
        turag_errorf("%s: call to Aseb::initPwmOutputBuffer() failed", name());
        return false;
    }

    if (size > 0 && !pwmOutputs_) {
        turag_errorf("%s: call to Aseb::initPwmOutputBuffer() failed", name());
        return false;
    }

    Request<AsebGetInfo> request;
    Response<AsebGetPwmValue> response;

    for (int i = 0; i < size; ++i) {
        pwmOutputs_[i].target = 0.0f;

        request.data.command = TURAG_FELDBUS_ASEB_PWM_SPEED;
        request.data.index = i + TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT;
        if(!transceive(request, &response)) return false;

        if(response.data.value >= pwmOutputs_[i].max_value)
            pwmOutputs_[i].speed = INFINITY;
        else
            pwmOutputs_[i].speed = static_cast<float>(pwmOutputs_[i].frequency) * static_cast<float>(response.data.value) * 100.0f /
                    static_cast<float>(pwmOutputs_[i].max_value);
    }
    return true;
}

float Aseb::getPwmSpeed(unsigned key) {
	if (!isInitialized()) {
        turag_errorf("%s: tried to call Aseb::getPwmSpeed prior to initialization", name());
        return false;
    } else if (key >= static_cast<unsigned>(pwmOutputSize_)) {
        turag_errorf("%s: Wrong arguments to getPwmSpeed. Key must be in the range of 0 to %d (given %d).", name(), static_cast<unsigned>(pwmOutputSize_) - 1, key);
        return false;
    } else {
        return pwmOutputs_[key].speed;
    }
}

bool Aseb::setPwmSpeed(unsigned key, float speed) {
	if (!isInitialized()) {
        turag_errorf("%s: tried to call Aseb::setPwmSpeed prior to initialization", name());
        return false;
    } else if (key >= static_cast<unsigned>(pwmOutputSize_)) {
        turag_errorf("%s: Wrong arguments to setPwmSpeed. Key must be in the range of 0 to %d (given %d).", name(), static_cast<unsigned>(pwmOutputSize_) - 1, key);
        return false;
    }
    struct AsebSetSpeed {
        uint8_t command;
        uint8_t index;
        uint16_t value;
    };

    Request<AsebSetSpeed> request;
    request.data.command = TURAG_FELDBUS_ASEB_PWM_SPEED;
    request.data.index = key + TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT;
    if(std::isfinite(speed))
        request.data.value = static_cast<float>(pwmOutputs_[key].max_value) * (speed / 100.0f) / static_cast<float>(pwmOutputs_[key].frequency);
    else
        request.data.value = 0xFFFF;

    Response<> response;
    if (!transceive(request, &response)) {
        turag_errorf("%s: Aseb setPwmSpeed transceive failed", name());
        return false;
    }
    pwmOutputs_[key].speed = speed;
    return true;
}

bool Aseb::setTargetPwmOutput(unsigned key, float duty_cycle) {
	if (!isInitialized()) {
        turag_errorf("%s: tried to call Aseb::setTargetPwmOutput prior to initialization", name());
        return false;
    } else if (key >= static_cast<unsigned>(pwmOutputSize_)) {
        turag_errorf("%s: Wrong arguments to setTargetPwmOutput. Key must be in the range of 0 to %d (given %d).", name(), static_cast<unsigned>(pwmOutputSize_) - 1, key);
        return false;
	} else if (duty_cycle < 0.0f || duty_cycle > 100.0f) {
        turag_errorf("%s: Wrong arguments to setTargetPwmOutput: %.2f not within valid range (0 ... 100 %%)", name(), (double)duty_cycle);
		return false;
	} else {
        Request<AsebSet> request;
        request.data.index = key + TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT;
        request.data.value = static_cast<uint16_t>(duty_cycle / 100.0f * pwmOutputs_[key].max_value + 0.5f);

        Response<> response;
        if (!transceive(request, &response)) {
            turag_errorf("%s: Aseb setTargetPWMOutput transceive failed", name());
            return false;
        }

        pwmOutputs_[key].target = duty_cycle;
        return true;
    }
}


unsigned int Aseb::getCommandNameLength(unsigned key) {
	if (!isInitialized()) {
        turag_errorf("%s: tried to call Aseb::getCommandNameLength prior to initialization", name());
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

    // Do not use whole buffer for one byte address
    const int request_len = addressLength() + 2 + 1;
	uint8_t request[2 + 2 + 1];
	request[addressLength()] = TURAG_FELDBUS_ASEB_CHANNEL_NAME;
	request[addressLength() + 1] = key;

    if (!transceive(request,
                    request_len,
                    reinterpret_cast<uint8_t*>(out_name),
					name_length + addressLength() + 1)) {
        return false;
    }

    for (unsigned i = 0; i < name_length; ++i) {
		out_name[i] = out_name[i + addressLength()];
    }
    out_name[name_length] = 0;

    return true;

}

uint32_t Aseb::getPwmFrequency(unsigned key) {
    if (!isInitialized()) {
        turag_errorf("%s: tried to call Aseb::getPwmFrequency prior to initialization", name());
        return 0;
    } else if (key >= static_cast<unsigned>(pwmOutputSize_)) {
        turag_errorf("%s: Wrong arguments to getPwmFrequency. Key must be in the range of 0 to %d (given %d).", name(), static_cast<unsigned>(pwmOutputSize_) - 1, key);
        return 0;
    } else {
        return pwmOutputs_[key].frequency;
    }
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

#endif // TURAG_USE_TURAG_FELDBUS_HOST
