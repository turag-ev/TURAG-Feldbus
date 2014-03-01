/**
 *  @brief		TURAG feldbus aseb base class
 *  @file		aseb_tina.cpp
 *  @date		11.01.2014
 *  @author		Martin Oemus
 *
 */

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


bool Aseb::initialize(uint8_t* sync_buffer, int sync_buffer_size,
                      Analog_t* analogInputs, int analogInputSize,
                      Pwm_t* pwmOutputs, int pwmOutputSize) {

    syncSize_ = 0;

    int sizeBuffer = 0;
    if (!getDigitalInputSize(&sizeBuffer)) return false;
    if (!getDigitalOutputSize(&sizeBuffer)) return false;


    analogInputs_ = analogInputs;
    analogInputSize_ = analogInputSize;

    if (analogInputs_ && analogInputSize_ > 0) {
        if (!getAnalogInputSize(&sizeBuffer)) return false;
        if (analogInputSize_ > sizeBuffer) analogInputSize_ = sizeBuffer;

        Request<AsebGetInfo> request;
        request.data.command = TURAG_FELDBUS_ASEB_ANALOG_INPUT_FACTOR;

        Response<AsebGetAnalogFactor> response;

        for (int i = 0; i < analogInputSize_; ++i) {
            request.data.index = i + TURAG_FELDBUS_ASEB_INDEX_START_ANALOG_INPUT;

            if (!transceive(request, &response)) return false;
            analogInputs_[i].factor = response.data.factor;
        }
    }

    pwmOutputs_ = pwmOutputs;
    pwmOutputSize_ = pwmOutputSize;

    if (pwmOutputs_ && pwmOutputSize_ > 0) {
        if (!getPwmOutputSize(&sizeBuffer)) return false;
        if (pwmOutputSize_ > sizeBuffer) pwmOutputSize_ = sizeBuffer;

        Request<AsebGetInfo> request;
        request.data.command = TURAG_FELDBUS_ASEB_PWM_OUTPUT_MAX_VALUE;

        Response<AsebGetPwmMaxValue> response;

        for (int i = 0; i < pwmOutputSize_; ++i) {
            request.data.index = i + TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT;

            if (!transceive(request, &response)) return false;
            pwmOutputs_[i].max_value = static_cast<float>(response.data.maxValue);
        }
    }

    if (!getSyncSize(&sizeBuffer)) return false;
    if (sync_buffer_size < sizeBuffer) return false;
    syncBuffer_ = sync_buffer;
    syncSize_ = sizeBuffer;

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

    Request<uint8_t> request;
    request.address = myAddress;
    request.data = TURAG_FELDBUS_ASEB_SYNC;

    if (!transceive(reinterpret_cast<uint8_t*>(std::addressof(request)),
                    sizeof(request),
                    syncBuffer_,
                    syncSize_)) {
        return false;
    }

    uint8_t* response = syncBuffer_ + 1;

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

    isSynced_ = true;
    return true;
}

float Aseb::getAnalogInput(unsigned key) {
    if (syncSize_ &&
            analogInputs_ &&
            key >= TURAG_FELDBUS_ASEB_INDEX_START_ANALOG_INPUT &&
            key < TURAG_FELDBUS_ASEB_INDEX_START_ANALOG_INPUT + static_cast<unsigned>(analogInputSize_)) {

        unsigned index = key - TURAG_FELDBUS_ASEB_INDEX_START_ANALOG_INPUT;
        return static_cast<float>(analogInputs_[index].value) * analogInputs_[index].factor;
    } else {
        return 0.0f;
    }
}

bool Aseb::getDigitalInput(unsigned key) {
    if (syncSize_ &&
            key >= TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_INPUT &&
            key < TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_INPUT + static_cast<unsigned>(digitalInputSize_)) {

        unsigned index = key - TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_INPUT;
        return static_cast<bool>(digitalInputs_ & (1<<index));
    } else {
        return false;
    }
}

bool Aseb::getDigitalOutput(unsigned key) {
    if (syncSize_ &&
            key >= TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_OUTPUT &&
            key < TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_OUTPUT + static_cast<unsigned>(digitalOutputSize_)) {

        unsigned index = key - TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_OUTPUT;
        return static_cast<bool>(digitalOutputs_ & (1<<index));
    } else {
        return false;
    }
}

bool Aseb::initDigitalOutputBuffer() {
    if (!syncSize_) return false;

    int size;
    if (!getDigitalOutputSize(&size)) return false;

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
    if (syncSize_ &&
            key >= TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_OUTPUT &&
            key < TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_OUTPUT + static_cast<unsigned>(digitalOutputSize_)) {

        unsigned index = key - TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_OUTPUT;

        uint16_t temp = digitalOutputs_;

        if (value) {
            temp |= (1<<index);
        } else {
            temp &= ~(1<<index);
        }

        Request<AsebSetDigital> request;
        request.data.index = key;
        request.data.value = value;

        Response<> response;

        if (!transceive(request, &response)) return false;

        digitalOutputs_ = temp;

        return true;
    } else {
        return false;
    }
}

float Aseb::getPwmOutput(unsigned key) {
    if (syncSize_ &&
            pwmOutputs_ &&
            key >= TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT &&
            key < TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT + static_cast<unsigned>(pwmOutputSize_)) {

        unsigned index = key - TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT;
        return pwmOutputs_[index].value;
    } else {
        return false;
    }
}

bool Aseb::initPwmOutputBuffer() {
    if (!syncSize_) return false;

    int size;
    if (!getPwmOutputSize(&size)) return false;

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
    if (syncSize_ &&
            pwmOutputs_ &&
            key >= TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT &&
            key < TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT + static_cast<unsigned>(pwmOutputSize_)) {

        unsigned index = key - TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT;

        Request<AsebSet> request;
        request.data.index = key;
        request.data.value = static_cast<uint16_t>(duty_cycle / 100.0f * pwmOutputs_[index].max_value + 0.5f);

        Response<> response;

        if (!transceive(request, &response)) return false;

        pwmOutputs_[index].value = duty_cycle;
        return true;
    } else {
        return false;
    }
}


bool Aseb::getPwmFrequency(unsigned key, uint32_t* frequency) {
    if (syncSize_ &&
            pwmOutputs_ &&
            key >= TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT &&
            key < TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT + static_cast<unsigned>(pwmOutputSize_)) {

        Request<AsebGetInfo> request;
        request.data.command = TURAG_FELDBUS_ASEB_PWM_OUTPUT_FREQUENCY;
        request.data.index = key;

        Response<AsebGetPwmFrequency> response;

        if (!transceive(request, &response)) return false;

        *frequency = response.data.frequency;

        return true;
    } else {
        return false;
    }
}


unsigned int Aseb::getCommandNameLength(unsigned key) {
    if (!syncSize_) {
        errorf("%s: not initialized", name);
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

    Request<AsebGetInfo> request;
    request.address = myAddress;
    request.data.command = TURAG_FELDBUS_ASEB_CHANNEL_NAME;
    request.data.index = key;

    if (!transceive(reinterpret_cast<uint8_t*>(std::addressof(request)),
                    sizeof(request),
                    reinterpret_cast<uint8_t*>(out_name),
                    name_length + 2)) {
        return false;
    }

    for (unsigned i = 0; i < name_length; ++i) {
        out_name[i] = out_name[i+1];
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

