#include <cstring>
#include <algorithm>
#include "feldbus_slave_stellantriebe.h"

#if (TURAG_FELDBUS_DEVICE_PROTOCOL==TURAG_FELDBUS_DEVICE_PROTOCOL_STELLANTRIEBE) || defined(__DOXYGEN__)

namespace TURAG {
namespace Feldbus {
namespace Slave {

Base::PacketProcessor Stellantriebe::backup_packet_processor_ = nullptr;
const Stellantriebe::Command* Stellantriebe::command_set_ = nullptr;
size_t Stellantriebe::command_set_size_ = 0;
std::array<const Stellantriebe::Command*, TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_BUFFER_SIZE> Stellantriebe::structured_output_table_;
size_t Stellantriebe::structured_output_table_size_;
Stellantriebe::CommandUpdateHandler Stellantriebe::command_update_handler_ = nullptr;

FeldbusSize_t Stellantriebe::process_feldbus_packet(const uint8_t* message, FeldbusSize_t message_length, uint8_t* response)
{
    // the feldbus base implementation guarantees message_length >= 1 and message[0] >= 1
    // so we don't need to check that

    uint8_t index = message[0] - 1;

    if (index < command_set_size_)
    {
        const Command &command = command_set_[index];

        if (message_length == 1)
        {
            // read request
            switch (command.info.type)
            {
            case CommandType::CHAR:
            {
                int8_t tmp = *static_cast<int8_t*>(command.value);
                memcpy(response, &tmp, get_command_length(CommandType::CHAR));
                return get_command_length(CommandType::CHAR);
            }
            case CommandType::SHORT:
            {
                int16_t tmp = *static_cast<int16_t*>(command.value);
                memcpy(response, &tmp, get_command_length(CommandType::SHORT));
                return get_command_length(CommandType::SHORT);
            }
            case CommandType::LONG:
            {
                int32_t tmp = *static_cast<int32_t*>(command.value);
                memcpy(response, &tmp, get_command_length(CommandType::LONG));
                return get_command_length(CommandType::LONG);
            }
            case CommandType::FLOAT:
            {
                float tmp = *static_cast<float*>(command.value);
                memcpy(response, &tmp, get_command_length(CommandType::FLOAT));
                return get_command_length(CommandType::FLOAT);
            }
            default:
                return TURAG_FELDBUS_IGNORE_PACKAGE;
            }
        }
        else if (message_length != 4)
        {
            // write request

            if (command.info.access != Stellantriebe::Access::WRITE
                || command.info.type == Stellantriebe::CommandType::NONE
                || command.info.type == Stellantriebe::CommandType::NONE_TEXT)
            {
                return TURAG_FELDBUS_IGNORE_PACKAGE;
            }

            memset(command.value, 0, get_command_length(command.info.type));
            size_t data_len = std::min<size_t>(message_length - 1, get_command_length(command.info.type));
            memcpy(command.value, &message[1], data_len);
            if (command_update_handler_)
                command_update_handler_(command);
            return 0;
        }
        else
        {
            if (message[1] == TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_COMMANDSET_SIZE)
            {
                // return length of command set
                response[0] = command_set_size_;
                return 1;
            }
            else if (message[1] == TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET)
            {
                // command info request
                static_assert(sizeof(CommandInfo) + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH
                    <= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE, "Buffer overflow");
                static_assert(sizeof(CommandInfo) == 6, "CommandInfo must be 6 bytes long!");

                memcpy(response, &command.info, sizeof(CommandInfo));
                return sizeof(CommandInfo);
            }
            else if (message[1] == TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_NAME_LENGTH)
            {
                // return length of command name
                response[0] = command.name ? strlen(command.name) : 0;
                return 1;
            }
            else if (message[1] == TURAG_FELDBUS_STELLANTRIEBE_COMMAND_INFO_GET_NAME)
            {
                // return command name
                if (!command.name)
                    return 0;

                uint8_t length = strlen(command.name);
                if (length + TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH > TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE)
                {
                    length = TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE - TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH;
                }

                memcpy(response, command.name, length);
                return length;
            }
            else
            {
                return TURAG_FELDBUS_IGNORE_PACKAGE;
            }
        }
    }
    else if (message[0] == TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_GET)
    {
        if (message_length == 1)
        {
            // generate structured output
            uint8_t *out = response;

            // we do not check whether command->value is a valid pointer
            // because we check for validity of the requested values
            // when the table is generated
            // There is also a check done whether the output will fit
            // into the bufer, so there is no check required either.
            for (size_t i = 0; i < structured_output_table_size_; ++i)
            {
                const Command &command = *structured_output_table_[i];

                memcpy(out, command.value, get_command_length(command.info.type));
                out += get_command_length(command.info.type);
            }
            return out - response;
        }
        else
        {
            if (message[1] == TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_SET_STRUCTURE)
            {
                // update structure table
                int8_t i, error = 0;
                uint8_t size_sum = 0, value_index;
                //feldbus_stellantriebe_command_t *command;

                // cancel if the request is too long
                if (message_length - 2 > TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_BUFFER_SIZE)
                {
                    error = 1;
                    message_length = 0;
                }

                for (i = 2; i < message_length; ++i)
                {
                    value_index = message[i] - 1;

                    // cancel if the host demands a non-supported key
                    if (value_index >= command_set_size_)
                    {
                        error = 1;
                        break;
                    }
                    const Command &command = command_set_[value_index];

                    // cancel if the host demands a non-supported key
                    if (!get_command_length(command.info.type))
                    {
                        error = 1;
                        break;
                    }

                    structured_output_table_[i - 2] = &command;
                    size_sum += get_command_length(command.info.type);

                    // cancel if whole package would not fit into buffer
                    if (size_sum >= TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE)
                    {
                        error = 1;
                        break;
                    }
                }

                if (error == 1)
                {
                    structured_output_table_size_ = 0;
                    response[0] = TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_TABLE_REJECTED;
                    return 1;
                }
                else
                {
                    structured_output_table_size_ = message_length - 2;
                    response[0] = TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_TABLE_OK;
                    return 1;
                }
            }
            else if (message[1] == TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_GET_BUFFER_SIZE)
            {
                // return table size
                response[0] = TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_BUFFER_SIZE;
                return 1;
            }
            else
            {
                return TURAG_FELDBUS_IGNORE_PACKAGE;
            }
        }
    }
    else
    {
        return backup_packet_processor_
                ? backup_packet_processor_(message, message_length, response)
                : TURAG_FELDBUS_IGNORE_PACKAGE;
    }
}

} // namespace Slave
} // namespace Feldbus
} // namespace TURAG

#endif /* (TURAG_FELDBUS_DEVICE_PROTOCOL==TURAG_FELDBUS_DEVICE_PROTOCOL_STELLANTRIEBE) */
