#ifndef SLAVE_FELDBUS_SLAVE_STELLANTRIEBE_H_
#define SLAVE_FELDBUS_SLAVE_STELLANTRIEBE_H_

/**
 * @defgroup feldbus-slave-stellantriebe Stellantriebe-Implementierung
 * @ingroup feldbus-slave
 * 
 * TURAG-Feldbus-Stellantriebe-Implementierung. Implementiert das Stellantriebe-Protokoll des %TURAG Feldbus.
 * 
 * \see \ref feldbus-slave
 * 
 */

#include "feldbus_slave_base.h"
#include "tina/feldbus/protocol/turag_feldbus_fuer_stellantriebe.h"

namespace TURAG {
namespace Feldbus {
    
/**
 * @brief Feldbus-Slave-Klassen und Debug-Funktionen.
 * @ingroup feldbus-slave-stellantriebe
 */
namespace Slave {

/**
 * @brief Plattform-unabhängiger Teil der Felbus-Stellantriebe-Slave Implementierung.
 * 
 * Diese Klassse ist auf die Existenz einer plattform-abhängigen
 * Slave::Driver-Klasse angewiesen.
 * 
 * \see \ref feldbus-slave-stellantriebe
 */
class Stellantriebe {
public:
    // Better readability than boolean values
    enum class Access {
        RO,
        RW,
    };

    // Created using mk_cmd(...)
    struct Command {
        void* value;
        const char *name;
        float factor;
        uint8_t id;
        uint8_t length;
        bool write_access;
    };

    // 
    template<size_t N>
    static
    void init(
        const Command (&commands)[N],
        Base::PacketProcessor packetProcessor = nullptr,
        Base::BroadcastProcessor broadcastProcessor = nullptr)
    {
        Base::init(&Stellantriebe::process_feldbus_packet, broadcastProcessor);
        backup_packet_processor_ = packetProcessor;
        command_set_ = commands;
        command_set_size_ = N;
    }

    //
    template <typename T>
    static constexpr const
    Command mk_cmd(
        uint8_t id,
        T *value,
        const char *name,
        float factor = 1.0f, 
        Access access = Access::RO)
    {
        static_assert(sizeof(T) <= 4, "Stellantriebe command must not be larger than 4 bytes!");
        return { value, name, factor, id, sizeof(T), access == Access::RW};
    }

    //
    static FeldbusSize_t process_feldbus_packet(const uint8_t* message, FeldbusSize_t message_length, uint8_t* response);
private:
    Stellantriebe();

    static Base::PacketProcessor backup_packet_processor_;
    static const Command* command_set_;
    static size_t command_set_size_;
};


} // namespace Slave
} // namespace Feldbus
} // namespace TURAG

#endif /* SLAVE_FELDBUS_SLAVE_STELLANTRIEBE_H_ */
