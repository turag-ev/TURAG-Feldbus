/**
 *  @brief		TURAG feldbus actor base class
 *  @file		TURAGFeldbusAktor.cpp
 *  @date		22.03.2013
 *  @author		Martin Oemus
 *
 */

#include "aktor.h"

#include <tina++/tina.h>
#include <tina/debug.h>

#include <cstring>


namespace TURAG {
namespace Feldbus {

struct AktorGetShort {
    int16_t value;
} _packed;

struct AktorGetLong {
    int32_t value;
} _packed;

struct AktorSetChar {
    uint8_t key;
    int8_t value;
} _packed;

struct AktorSetShort {
    uint8_t key;
    int16_t value;
} _packed;

struct AktorSetLong {
    uint8_t key;
    int32_t value;
} _packed;

struct AktorGetCommandInfo {
    uint8_t key;
    uint8_t cmd0;
    uint8_t cmd1;
    uint8_t cmd2;
} _packed;

struct AktorGetCommandInfoResponse {
    AktorCommandWriteAccess writeAccess;
    AktorCommandLength length;
    float factor;
} _packed;

struct AktorGetStructuredOutputControl {
    uint8_t key;
    uint8_t cmd;
};


unsigned int Aktor::getCommandsetLength(void) {
    if (commandSetPopulated) {
        return commandSetLength;
    } else {
        Request<AktorGetCommandInfo> request;
        request.data.key = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_COMMANDSET_SIZE;
        request.data.cmd0 = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_COMMANDSET_SIZE;
        request.data.cmd1 = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_COMMANDSET_SIZE;
        request.data.cmd2 = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_COMMANDSET_SIZE;

        Response<uint8_t> response;
        if (!transceive(request, &response)) {
            return 0;
        }
        return response.data;
    }
}

bool Aktor::populateCommandSet(AktorCommand_t* commandSet_, unsigned int commandSetLength_) {
    if (!commandSet_) return false;
    
    unsigned int deviceCommandSetLength = getCommandsetLength();
    if (deviceCommandSetLength == 0) return false;
    
    if (deviceCommandSetLength > commandSetLength_) {
        commandSetLength = commandSetLength_;
    } else {
        commandSetLength = deviceCommandSetLength;
    }
    commandSet = commandSet_;
    
    Request<AktorGetCommandInfo> request;
    request.data.key = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET;
    request.data.cmd0 = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET;
    request.data.cmd1 = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET;
    request.data.cmd2 = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET;
    Response<AktorGetCommandInfoResponse> response;
    
    for (unsigned int i = 0; i < commandSetLength; ++i) {
        request.data.key = i + 1;

        if (!transceive(request, &response)) {
            return false;
        }
        commandSet[i].writeAccess = static_cast<AktorCommandWriteAccess>(response.data.writeAccess);
        commandSet[i].length = static_cast<AktorCommandLength>(response.data.length);
        commandSet[i].factor = response.data.factor;

    }
    commandSetPopulated = true;
    return true;
}

bool Aktor::getValue(uint8_t key, float* value) {
    if (!commandSetPopulated) {
        errorf("%s: commandSet not populated", name);
        return false;
    }
    if (key > commandSetLength || key == 0) {
        errorf("%s: key not within commandSetLength", name);
        return false;
    }

    AktorCommand_t* command = &commandSet[key-1];
    
    if (command->length == AktorCommandLength::none) {
        errorf("%s: key not supported", name);
        return false;
    }
    if (command->factor == TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE) {
        errorf("%s: value not floating point", name);
        return false;
    }
    
    if (!command->bufferValid) {
        Request<uint8_t> request;
        request.data = key;

        if (command->length == AktorCommandLength::length_char) {
            Response<int8_t> response;
            if (!transceive(request, &response)) {
                return false;
            }
            command->buffer.floatValue = static_cast<float>(response.data) * command->factor;
        } else if (command->length == AktorCommandLength::length_short) {
            Response<AktorGetShort> response;
            if (!transceive(request, &response)) {
                return false;
            }
            command->buffer.floatValue = static_cast<float>(response.data.value) * command->factor;
        } else {
            Response<AktorGetLong> response;
            if (!transceive(request, &response)) {
                return false;
            }
            command->buffer.floatValue = static_cast<float>(response.data.value) * command->factor;
        }

        // protocol definition: writable values are bufferable
        if (command->writeAccess == AktorCommandWriteAccess::write) {
            command->bufferValid = true;
        }

    }
    *value = command->buffer.floatValue;
    return true;
}

bool Aktor::getValue(uint8_t key, int32_t* value) {
    if (!commandSetPopulated) {
        errorf("%s: commandSet not populated", name);
        return false;
    }
    if (key > commandSetLength || key == 0) {
        errorf("%s: key not within commandSetLength", name);
        return false;
    }

    AktorCommand_t* command = &commandSet[key-1];

    if (command->length == AktorCommandLength::none) {
        errorf("%s: key not supported", name);
        return false;
    }
    if (command->factor != TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE) {
        errorf("%s: value is floating point", name);
        return false;
    }

    if (!command->bufferValid) {
        Request<uint8_t> request;
        request.data = key;

        if (command->length == AktorCommandLength::length_char) {
            Response<int8_t> response;
            if (!transceive(request, &response)) {
                return false;
            }
            command->buffer.intValue = response.data;
        } else if (command->length == AktorCommandLength::length_short) {
            Response<AktorGetShort> response;
            if (!transceive(request, &response)) {
                return false;
            }
            command->buffer.intValue = response.data.value;
        } else {
            Response<AktorGetLong> response;
            if (!transceive(request, &response)) {
                return false;
            }
            command->buffer.intValue = response.data.value;
        }

        // protocol definition: writable values are bufferable
        if (command->writeAccess == AktorCommandWriteAccess::write) {
            command->bufferValid = true;
        }

    }
    *value = command->buffer.intValue;
    return true;
}


bool Aktor::setValue(uint8_t key, float value) {
    if (!commandSetPopulated) {
        errorf("%s: commandSet not populated", name);
        return false;
    }
    if (key > commandSetLength || key == 0) {
        errorf("%s: key not within commandSetLength", name);
        return false;
    }

    AktorCommand_t* command = &commandSet[key-1];

    if (command->length == AktorCommandLength::none) {
        errorf("%s: key not supported", name);
        return false;
    }
    if (command->writeAccess != AktorCommandWriteAccess::write) {
        errorf("%s: key not writable", name);
        return false;
    }
    if (command->factor == TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE) {
        errorf("%s: value not floating point", name);
        return false;
    }

    command->bufferValid = false;
    Response<> response;

    if (command->length == AktorCommandLength::length_char) {
        Request<AktorSetChar> request;
        request.data.key = key;
        request.data.value = static_cast<int8_t>(value / command->factor);

        if (!transceive(request, &response)) {
            return false;
        }
    } else if (command->length == AktorCommandLength::length_short) {
        Request<AktorSetShort> request;
        request.data.key = key;
        request.data.value = static_cast<int16_t>(value / command->factor);

        if (!transceive(request, &response)) {
            return false;
        }
    } else {
        Request<AktorSetLong> request;
        request.data.key = key;
        request.data.value = static_cast<int32_t>(value / command->factor);

        if (!transceive(request, &response)) {
            return false;
        }
    }
    return true;
}

bool Aktor::setValue(uint8_t key, int32_t value) {
    if (!commandSetPopulated) {
        errorf("%s: commandSet not populated", name);
        return false;
    }
    if (key > commandSetLength || key == 0) {
        errorf("%s: key not within commandSetLength", name);
        return false;
    }

    AktorCommand_t* command = &commandSet[key-1];

    if (command->length == AktorCommandLength::none) {
        errorf("%s: key not supported", name);
        return false;
    }
    if (command->writeAccess != AktorCommandWriteAccess::write) {
        errorf("%s: key not writable", name);
        return false;
    }
    if (command->factor != TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE) {
        errorf("%s: value is floating point", name);
        return false;
    }

    command->bufferValid = false;
    Response<> response;

    if (command->length == AktorCommandLength::length_char) {
        Request<AktorSetChar> request;
        request.data.key = key;
        request.data.value = static_cast<int8_t>(value);

        if (!transceive(request, &response)) {
            return false;
        }
    } else if (command->length == AktorCommandLength::length_short) {
        Request<AktorSetShort> request;
        request.data.key = key;
        request.data.value = static_cast<int16_t>(value);

        if (!transceive(request, &response)) {
            return false;
        }
    } else {
        Request<AktorSetLong> request;
        request.data.key = key;
        request.data.value = static_cast<int32_t>(value);

        if (!transceive(request, &response)) {
            return false;
        }
    }
    return true;
}

unsigned int Aktor::getCommandNameLength(uint8_t key) {
    if (!commandSetPopulated) {
        errorf("%s: commandSet not populated", name);
        return false;
    }
    if (key > commandSetLength || key == 0) {
        errorf("%s: key not within commandSetLength", name);
        return false;
    }

    AktorCommand_t* command = &commandSet[key-1];

    if (command->length == AktorCommandLength::none) {
        errorf("%s: key not supported", name);
        return false;
    }

    Request<AktorGetCommandInfo> request;
    request.data.key = key;
    request.data.cmd0 = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_NAME_LENGTH;
    request.data.cmd1 = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_NAME_LENGTH;
    request.data.cmd2 = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_NAME_LENGTH;

    Response<uint8_t> response;

    if (!transceive(request, &response)) {
        return 0;
    } else {
        return response.data;
    }
}

bool Aktor::getCommandName(uint8_t key, char* out_name) {
    if (!out_name) {
        return false;
    }

    int name_length = getCommandNameLength(key);
    if (name_length == 0) {
        *out_name = 0;
        return false;
    }
    
    Request<AktorGetCommandInfo> request;
    request.address = myAddress;
    request.data.key = key;
    request.data.cmd0 = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_NAME;
    request.data.cmd1 = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_NAME;
    request.data.cmd2 = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_NAME;
    
    if (!transceive(reinterpret_cast<uint8_t*>(std::addressof(request)),
                    sizeof(request),
                    reinterpret_cast<uint8_t*>(out_name),
                    name_length + 2)) {
        return false;
    }

    for (int i = 0; i < name_length; ++i) {
        out_name[i] = out_name[i+1];
    }
    out_name[name_length] = 0;

    return true;

}

#ifdef TURAG_FELDBUS_AKTOR_STRUCTURED_OUTPUT_AVAILABLE

unsigned int Aktor::getStructuredOutputTableLength(void) {
    if (structuredOutputTableLength != 0) {
        return structuredOutputTableLength;
    } else {
        Request<AktorGetStructuredOutputControl> request;
        request.data.key = TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_CONTROL;
        request.data.cmd = TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_GET_BUFFER_SIZE;

        Response<uint8_t> response;

        if (!transceive(request, &response)) {
            return 0;
        }
        structuredOutputTableLength = response.data;
        return structuredOutputTableLength;
    }
}


bool Aktor::setStructuredOutputTable(const std::vector<uint8_t>& keys) {
    if (keys.size() > getStructuredOutputTableLength()) {
        errorf("%s: output table in device too small for number of provided keys", name);
        return false;
    }
    if (!commandSetPopulated) {
        errorf("%s: commandSet not populated", name);
        return false;
    }
    
    for (unsigned int i = 0; i < keys.size(); ++i) {
        if (keys[i] > getCommandsetLength() || commandSet[keys[i] - 1].length == AktorCommandLength::none) {
            errorf("%s: requested keys invalid", name);
            return false;
        }
    }
    
    uint8_t* request = new uint8_t[keys.size() + 4];
    request[0] = myAddress;
    request[1] = TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_CONTROL;
    request[2] = TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_SET_STRUCTURE;
    memcpy(request + 3, keys.data(), keys.size());
    
    Response<uint8_t> response;
    
    if (transceive(request, keys.size() + 4,
                   reinterpret_cast<uint8_t*>(std::addressof(response)), sizeof(response))) {
        if (response.data == TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_TABLE_OK) {
            structuredOutputTable = keys;
            delete[] request;
            return true;
        }
    }
    
    delete[] request;
    return false;
}


bool Aktor::getStructuredOutput(std::vector<StructuredDataPair_t>* values) {
    if (structuredOutputTable.size() == 0) {
        errorf("%s: structured output mapping empty", name);
        return false;
    }
    if (!commandSetPopulated) {
        errorf("%s: commandSet not populated", name);
        return false;
    }
    
    int data_size = 0;
    for (unsigned int i = 0; i < structuredOutputTable.size(); ++i) {
        switch (commandSet[structuredOutputTable[i] - 1].length) {
        case AktorCommandLength::length_char:
            data_size += 1;
            break;
        case AktorCommandLength::length_short:
            data_size += 2;
            break;
        case AktorCommandLength::length_long:
            data_size += 4;
            break;
        default:
            return false;
        }
    }

    Request<uint8_t> request;
    request.address = myAddress;
    request.data = TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_GET;

    uint8_t* response = new uint8_t[data_size + 2];

    if (transceive(reinterpret_cast<uint8_t*>(std::addressof(request)),
                   sizeof(request),
                   response,
                   data_size + 2)) {
        uint8_t* pValue = response + 1;

        for (unsigned int i = 0; i < structuredOutputTable.size(); ++i) {
            int32_t device_value;

            switch (commandSet[structuredOutputTable[i] - 1].length) {
            case AktorCommandLength::length_char:
                device_value = *(reinterpret_cast<int8_t*>(pValue));
                pValue += 1;
                break;
            case AktorCommandLength::length_short:
                device_value = *(reinterpret_cast<int16_t*>(pValue));
                pValue += 2;
                break;
            case AktorCommandLength::length_long:
                device_value = *(reinterpret_cast<int32_t*>(pValue));
                pValue += 4;
                break;
            default:
                return false;
            }

            StructuredDataPair_t pair;
            pair.key = structuredOutputTable[i];

            if (commandSet[structuredOutputTable[i] - 1].factor == TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE) {
                pair.value = static_cast<float>(device_value);
            } else {
                pair.value = static_cast<float>(device_value) * commandSet[structuredOutputTable[i] - 1].factor;
            }
            values->push_back(pair);
        }
        delete[] response;
        return true;
    }

    delete[] response;
    return false;
}

#endif // TURAG_FELDBUS_AKTOR_STRUCTURED_OUTPUT_AVAILABLE

} // namespace Feldbus
} // namespace TURAG

