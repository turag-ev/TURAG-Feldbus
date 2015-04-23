/**
 *  @brief		Schnittstelle zur RS485-Hardware.
 *  @file		rs485.h
 *  @date		17.11.2013
 *  @author		Martin Oemus <martin@oemus.net>
 *  @ingroup	feldbus-host
 * 
 * Dieser Header deklariert stellt Zugriffsfunktionen
 * für die RS485-Hardwareschnittstelle bereit, die von den 
 * Host-Klassen des %TURAG-Feldbus benötigt werden. Die
 * Implementierung ist plattform-spezifisch.
 */

#ifndef TINA_FELDBUS_HOST_RS485_H
#define TINA_FELDBUS_HOST_RS485_H

#include <tina/tina.h>
#include <tina/time.h>

#if TURAG_USE_TURAG_FELDBUS_HOST

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialisiert die RS485-Hardware-Schnittstelle.
 * \param[in] baud_rate Baudrate.
 * \param[in] timeout Timeout für Transmissionen.
 * \return True wenn die Hardware erfolgreich initialisiert
 * werden konnte, ansonsten false.
 */
bool turag_rs485_init(uint32_t baud_rate, TuragSystemTime timeout);


/**
 * \brief Sendet und empfängt Daten auf dem Bus.
 * \param[in,out] transmit Pointer auf den Sendepuffer.
 * \param[in,out] transmit_length Pointer auf die Größe der zu sendenden Daten.
 * Enthält nach Rückkehr der Funktion die Größe der tatsächlich gesendeten Daten.
 * \param[in,out] receive Pointer auf den Empfangspuffer.
 * \param[in,out] receive_length Pointer auf die Größe der zu empfangenden Daten.
 * Enthält nach Rückkehr der Funktion die Größe der tatsächlich empfangenen Daten.
 * \param[in] delayTransmission Fügt vor Beginn der Transmission ein Delay von 15 Frames ein,
 * um die korrekte Erkennung von Paketen in den Slaves zu ermöglichen.
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
bool turag_rs485_transceive(uint8_t* transmit, int* transmit_length, uint8_t* receive, int* receive_length, bool delayTransmission);


/**
 * \brief Leert den Eingangspuffer der benutzten Hardwareschnittstelle.
 */
void turag_rs485_buffer_clear(void);


#ifdef __cplusplus
} // extern "C"
#endif

#else
# error TURAG_USE_TURAG_FELDBUS_HOST must be defined to 1
#endif

#endif // TINA_FELDBUS_HOST_RS485_H
