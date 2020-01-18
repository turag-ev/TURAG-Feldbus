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

#include <array>
#include "feldbus_slave_base.h"
#include "tina/feldbus/protocol/turag_feldbus_fuer_stellantriebe.h"

#if (TURAG_FELDBUS_DEVICE_PROTOCOL==TURAG_FELDBUS_DEVICE_PROTOCOL_STELLANTRIEBE) || defined(__DOXYGEN__)

#ifndef TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_BUFFER_SIZE
# warning TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_BUFFER_SIZE must be defined, using default 0
# define TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_BUFFER_SIZE 0
#endif

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
    /**
     * @brief Gibt an, ob ein Wert über das Stellantriebe-Protokoll geschrieben werden kann.
     */
    enum class Access : uint8_t
    {
        READ = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_NO_WRITE_ACCESS,
        WRITE = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
    };

    /**
     * @brief Gibt Länge bzw. Typ eines Befehls an
     */
    enum class CommandType : uint8_t
    {
        NONE = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_NONE,
        CHAR = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_CHAR,
        SHORT = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT,
        LONG = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_LONG,
        NONE_TEXT = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_NONE_TEXT,
        FLOAT = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_FLOAT
    };

    struct CommandInfo
    {
        Access access;
        CommandType type;
        float factor;
    } _packed;

    static constexpr
    size_t get_command_length(CommandType t)
    {
        // constexpr does not allow switch :(
        return t == CommandType::CHAR
                ? sizeof(int8_t)
                : t == CommandType::SHORT
                  ? sizeof(int16_t)
                  : t == CommandType::LONG
                    ? sizeof(int32_t)
                    : t == CommandType::FLOAT
                        ? sizeof(float)
                        : 0;
    }

    struct Command
    {
        constexpr
        Command() :
            info{Access::READ, CommandType::NONE,
                 TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE},
            value(nullptr), name(nullptr)
        { }

        constexpr
        Command(const char* name) :
            info{Access::READ, CommandType::NONE_TEXT,
                 TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE},
            value(nullptr), name(name)
        { }

        constexpr
        Command(const char* name, int8_t* value, float factor,
                Access access = Access::READ) :
            info{access, CommandType::CHAR, factor}, value(value), name(name)
        { }

        constexpr
        Command(const char* name, int16_t* value, float factor,
                Access access = Access::READ) :
            info{access, CommandType::SHORT, factor}, value(value), name(name)
        { }

        constexpr
        Command(const char* name, int32_t* value, float factor,
                Access access = Access::READ) :
            info{access, CommandType::LONG, factor}, value(value), name(name)
        { }

        constexpr
        Command(const char* name, float* value, float factor = 1.0f,
                Access access = Access::READ) :
            info{access, CommandType::FLOAT, factor}, value(value), name(name)
        { }

        // Command as control value
        template<typename T>
        constexpr
        Command(const char* name, T* value, Access access = Access::READ) :
            Command(name, value,
                    TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE,
                    access)
        { }

        // Read-only command with const ptr
        template<typename T>
        constexpr
        Command(const char* name, const T* value, float factor) :
            Command(name, const_cast<T*>(value), factor, Access::READ)
        { }

        // Read-only command with const ptr as control value
        template<typename T>
        constexpr
        Command(const char* name, const T* value) :
            Command(name, const_cast<T*>(value),
                    TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE,
                    Access::READ)
        { }

        CommandInfo info;
        void *value;
        const char *name;
    };


    using CommandUpdateHandler = void (*)(const Command &command);

    /**
     * @brief Initializiert die Stellantriebe-Implementierung.
     * @param[in] commands Array mit Stellantriebe-Befehlen
     * @param[in] packetProcessor Funktion, die Paket verarbeitet,
     * falls das nicht konform zum Stellantriebe-Protokoll ist
     * @param[in] broadcastProcessor Funktion, die Broadcast-Pakete verarbeitet
     */
    template <size_t N>
    static void init(
            const Command (&commands)[N],
            CommandUpdateHandler commandUpdateHandler = nullptr,
            Base::PacketProcessor packetProcessor = nullptr,
            Base::BroadcastProcessor broadcastProcessor = nullptr)
    {
        Base::init(&Stellantriebe::process_feldbus_packet, broadcastProcessor);
        backup_packet_processor_ = packetProcessor;
        command_update_handler_ = commandUpdateHandler;
        command_set_ = commands;
        command_set_size_ = N;
    }

    /**
     * @brief Verarbeitet ein Feldbus-Paket nach dem Stellantriebe-Protokoll.
     * @param[in] message Eingabebuffer des Pakets
     * @param[in] message_length Länge des Pakets
     * @param[out] Ausgabepuffer der Antwort
     * @return Länge der Antwortnachricht
     */
    static FeldbusSize_t process_feldbus_packet(
            const uint8_t* message,
            FeldbusSize_t message_length,
            uint8_t* response);
private:
    Stellantriebe();

    static Base::PacketProcessor backup_packet_processor_;
    static CommandUpdateHandler command_update_handler_;

    static const Command* command_set_;
    static std::array<
        const Command*,
        TURAG_FELDBUS_STELLANTRIEBE_STRUCTURED_OUTPUT_BUFFER_SIZE
    > structured_output_table_;

    static size_t command_set_size_;
    static size_t structured_output_table_size_;
};


} // namespace Slave
} // namespace Feldbus
} // namespace TURAG

#endif /* (TURAG_FELDBUS_DEVICE_PROTOCOL==TURAG_FELDBUS_DEVICE_PROTOCOL_STELLANTRIEBE) */
#endif /* SLAVE_FELDBUS_SLAVE_STELLANTRIEBE_H_ */
