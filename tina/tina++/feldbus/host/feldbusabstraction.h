#ifndef FELDBUSABSTRACTION_H
#define FELDBUSABSTRACTION_H


#include <tina++/tina.h>
#include <tina++/time.h>


#if !TURAG_USE_TURAG_FELDBUS_HOST
# warning TURAG_USE_TURAG_FELDBUS_HOST must be defined to 1
#endif


namespace TURAG {
namespace Feldbus {



/**
 * @brief Abstrahiert eine physikalische %TURAG-Feldbus-Schnittstelle.
 */
class FeldbusAbstraction
{
public:
    FeldbusAbstraction() {}

    /**
     * \brief Sendet und empfängt Daten auf dem Bus.
     * \param[in,out] transmit Pointer auf den Sendepuffer.
     * \param[in,out] transmit_length Pointer auf die Größe der zu sendenden Daten.
     * Enthält nach Rückkehr der Funktion die Größe der tatsächlich gesendeten Daten.
     * \param[in,out] receive Pointer auf den Empfangspuffer.
     * \param[in,out] receive_length Pointer auf die Größe der zu empfangenden Daten.
     * Enthält nach Rückkehr der Funktion die Größe der tatsächlich empfangenen Daten.
     * \param[in] delayTransmission Sorgt dafür, dass zwischen zwei Transmissionen ein
     * Delay von 15 Frames eingehalten wird um die korrekte Erkennung von Paketen in
     * den Slaves zu ermöglichen.
     * \return True wenn die korrekte Menge Daten gesendet und empfangen wurden, ansonsten false.
     *
     * Diese Funktion sendet blockierend einen Satz Daten auf den Bus und empfängt
     * danach die angegebene Menge an Daten. Die Funktion kehrt zurück, sobald alle Daten
     * gesendet und empfangen wurden oder wenn der in turag_rs485_init() eingestellte
     * Timeout erreicht wurde.
     *
     * Es ist zulässig, wenn receive_length auf null verweist oder 0 enthält: dann werden
     * nur Daten gesendet, aber keine empfangen.
     *
     * \pre Diese Funktion muss thread-safe implementiert werden.
     *
     * \post Wenn die Übertragung schon beim Senden scheitert, sollte nach
     * Rückkehr der Funktion tatsächlich 0 in receive_length stehen.
     *
     */
    virtual bool transceive(uint8_t *transmit, int *transmit_length, uint8_t *receive, int *receive_length, bool delayTransmission) = 0;

    /**
     * \brief Leert den Eingangspuffer der benutzten Hardwareschnittstelle.
     */
    virtual void clearBuffer(void) = 0;
};


}
}

#endif // FELDBUSABSTRACTION_H
