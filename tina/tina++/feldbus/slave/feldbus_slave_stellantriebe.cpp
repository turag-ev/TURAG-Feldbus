#include "feldbus_slave_stellantriebe.h"

namespace TURAG {
namespace Feldbus {
namespace Slave {

Base::PacketProcessor Stellantriebe::backup_packet_processor_ = nullptr;
const Stellantriebe::Command* Stellantriebe::command_set_ = nullptr;
size_t Stellantriebe::command_set_size_ = 0;

FeldbusSize_t Stellantriebe::process_feldbus_packet(const uint8_t* message, FeldbusSize_t message_length, uint8_t* response)
{
    return backup_packet_processor_(message, message_length, response);
}

} // namespace Slave
} // namespace Feldbus
} // namespace TURAG