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
    /**
     * @brief Gibt an, ob ein Wert über das Stellantriebe-Protokoll geschrieben werden kann.
     */
    enum class Access : bool {
        RO = false,
        RW = true,
    };

    /**
     * @brief Stellantriebe-Befehl
     * 
     * Diese Datenstruktur soll nicht direkt, sondern mit der funktion mk_cmd instanziiert werden.
     */
    struct Command {
        void* value;
        const char *name;
        float factor;
        uint8_t id;
        uint8_t length;
        Access access;
    };

    /**
     * @brief Initializiert die Stellantriebe-Implementierung.
     * @param[in] commands Array mit Stellantriebe-Befehlen
     * @param[in] packetProcessor Funktion, die Paket verarbeitet, falls das nicht konform zum Stellantriebe-Protokoll ist
     * @param[in] broadcastProcessor Funktion, die Broadcast-Pakete verarbeitet
     */
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

    /**
     * @brief Erzeugt einen Stellantriebe-Befehl.
     * @param[in] id Key des Stellantriebe-Befehls
     * @param[in] value Pointer auf den Wert
     * @param[in] name String, der Befehl verbal beschreibt
     * @param[in] factor Umrechnungsfaktor zu Einheit
     * @param[in] access Zugriffsart des Stellantriebe-Befehls
     * @return Stellantriebe-Befehl
     */
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
        return { value, name, factor, id, sizeof(T), access};
    }

    /**
     * @brief Verarbeitet ein Feldbus-Paket nach dem Stellantriebe-Protokoll.
     * @param[in] message Eingabebuffer des Pakets
     * @param[in] message_length Länge des Pakets
     * @param[out] Ausgabepuffer der Antwort
     * @return Länge der Antwortnachricht
     */
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
