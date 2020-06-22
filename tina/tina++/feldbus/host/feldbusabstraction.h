#ifndef FELDBUSABSTRACTION_H
#define FELDBUSABSTRACTION_H


#include <tina++/tina.h>
#include <tina++/time.h>
#include <tina++/thread.h>
#include <tina++/debug/errorobserver.h>
#include <tina/feldbus/protocol/turag_feldbus_bus_protokoll.h>


#if !TURAG_USE_TURAG_FELDBUS_HOST
# warning TURAG_USE_TURAG_FELDBUS_HOST must be defined to 1
#endif


namespace TURAG {
namespace Feldbus {

/*!
 * \brief Verfügbare Checksummenalgorithmen.
 * \see \ref checksums
 */
enum class ChecksumType : uint8_t {
	xor_based = TURAG_FELDBUS_CHECKSUM_XOR, ///< XOR-Checksumme.
	crc8_icode = TURAG_FELDBUS_CHECKSUM_CRC8_ICODE, ///< CRC8-Checksumme.
	none = 0xFF ///< Keine Checksumme verwenden.
};



/**
 * @brief Abstrahiert eine physikalische %TURAG-Feldbus-Schnittstelle.
 *
 * Diese Klasse implementiert einige gemeinsame Funktionalitäten wie
 * den threadsicheren Zugriff auf das Transportmedium, Mitführen von
 * Übertragungsstatistiken, die Steuerung des Einhaltens bestimmter
 * Freizeiten auf dem Bus, sowie die Checksummenprüfung empfangener
 * Daten.
 *
 * Die eigentliche Implementierung des Transports obliegt abzuleitenden
 * Subklassen, die plattformabhängig sind.
 *
 * Im Konstruktor kann angegeben werden, ob der Buszugriff mit einer
 * Semaphore geschützt werden soll. Ist dies der Fall, so kann ein
 * Busobjekt von Feldbusgeräten in unterschiedlichen Threads benutzt
 * werden.
 */
class FeldbusAbstraction
{
	NOT_COPYABLE(FeldbusAbstraction);

public:
	/**
	 * @brief Ergebnis der Busübertragung.
	 */
	enum class ResultStatus {
		/// Kein Übertragungsfehler.
		Success,
		/// Checksummenfehler.
		ChecksumError,
		/// Innerhalb des Timeouts nicht genügend Daten gesendet oder empfangen
		/// oder ein anderes, transportspezifisches Problem.
		TransmissionError
	};

	/**
	 * @brief Erzeugt ein FeldbusAbstraction-Objekt.
	 * @param name Name des Busses.
	 * @param threadSafe Gibt an, ob der Zugriff auf den Bus mit
	 * einer Semaphore geschützt werden soll.
	 */
	FeldbusAbstraction(const char* name, bool threadSafe = true) :
		name_(name), busTransmissionStatistics_(SystemTime::fromSec(5), 0, 50),
		sem_(1), lastTargetAddress_(TURAG_FELDBUS_BROADCAST_ADDR),
		threadSafe_(threadSafe)
	{}

#if TURAG_USE_LIBSUPCPP_RUNTIME_SUPPORT
	virtual ~FeldbusAbstraction() {}
#endif

    /**
     * \brief Sendet und empfängt Daten auf dem Bus.
     * \param[in] transmit Pointer auf den Sendepuffer.
     * \param[in,out] transmit_length Pointer auf die Größe der zu sendenden Daten.
     * Enthält nach Rückkehr der Funktion die Größe der tatsächlich gesendeten Daten.
     * \param[out] receive Pointer auf den Empfangspuffer.
     * \param[in,out] receive_length Pointer auf die Größe der zu empfangenden Daten.
     * Enthält nach Rückkehr der Funktion die Größe der tatsächlich empfangenen Daten.
	 * \param[in] targetAddress Adresse, an die das Paket gesendet wird. Die Adresse
	 * muss zusätzlich bereits im Paket kodiert werden (Die separate Angabe ist zum
	 * effizienten Einhalten des 15-Frame-Delays zwischen Transmissionen nötig,
	 * da diese Klasse keine Information über die im Paket verwendete Adresslänge hat).
	 * \param[in] checksumType Checksummentyp, mit der die Transmission abgesichert wird.
	 * \return True wenn die korrekte Menge Daten gesendet und empfangen wurden, ansonsten false.
	 *
	 * Diese Funktion sendet blockierend einen Satz Daten auf den Bus und empfängt
	 * danach die angegebene Menge an Daten. Die Funktion kehrt zurück, sobald alle Daten
	 * gesendet und empfangen wurden oder wenn ein Timeout erreicht wurde. Dieser ist durch
	 * die Subklasse weiter zu definieren, die außerdem eine Möglichkeit zur Anpassung
	 * desselben bieten sollte.
	 *
	 * Der Sendepuffer muss sowohl Adresse als auch die Checksumme enthalten.
	 *
	 * Es ist zulässig, wenn receive_length auf null verweist oder 0 enthält: dann werden
	 * nur Daten gesendet, aber keine empfangen.
	 *
	 * Wenn die Übertragung schon beim Senden scheitert, steht in receive_length 0.
     *
     */
	ResultStatus transceive(const uint8_t *transmit, int *transmit_length, uint8_t *receive, int *receive_length, unsigned targetAddress, ChecksumType checksumType);

    /**
     * \brief Leert den Eingangspuffer der benutzten Hardwareschnittstelle.
     */
    virtual void clearBuffer(void) = 0;

	/**
	 * @brief Bezeichnung des Busses.
	 * @return Name.
	 */
	const char* name(void) { return name_; }

	/**
	 * @brief Zugriff auf die Übertragungsstatistiken des Busses.
	 * @return Referenz auf eine Debug::ErrorObserver-Instanz.
	 */
	const Debug::ErrorObserver& busTransmissionStatistics(void) const { return busTransmissionStatistics_; }


protected:
	// should be private, but is protected to get it in the docs.
	/**
	 * \brief Sendet und empfängt Daten auf dem Bus.
	 * \param[in] transmit Pointer auf den Sendepuffer.
	 * \param[in,out] transmit_length Pointer auf die Größe der zu sendenden Daten.
	 * Enthält nach Rückkehr der Funktion die Größe der tatsächlich gesendeten Daten.
	 * \param[out] receive Pointer auf den Empfangspuffer.
	 * \param[in,out] receive_length Pointer auf die Größe der zu empfangenden Daten.
	 * Enthält nach Rückkehr der Funktion die Größe der tatsächlich empfangenen Daten.
	 * \param[in] delayTransmission Gibt an, ob das Paket-Delay von 15 Frames
	 * zum zuletzt gesendeten Paket einzuhalten ist oder nicht.
	 *
	 * Mit dieser Funktion muss von Subklassen der tatsächliche Datentransport
	 * implementiert werden.
	 *
	 * Wenn receive oder receive_length auf null verweist oder receive_length
	 * 0 enthält sollen nur Daten gesendet
	 * werden. Wenn transmit oder transmit_length auf 0 verweist oder transmit_length
	 * 0 enthält, so sollen nur Daten empfangen werden.
	 * Trifft beides zu, soll true zurückgegeben werden.
	 *
	 * \post Wenn die Übertragung schon beim Senden scheitert, sollte nach
	 * Rückkehr der Funktion tatsächlich 0 in receive_length stehen.
	 */
	virtual bool doTransceive(const uint8_t *transmit, int *transmit_length, uint8_t *receive, int *receive_length, bool delayTransmission) = 0;

private:
	const char* name_;

	Debug::ErrorObserver busTransmissionStatistics_;

	TURAG::Semaphore sem_;

	unsigned lastTargetAddress_;

	bool threadSafe_;
};


} // namespace Feldbus
} // namespace TURAG

#endif // FELDBUSABSTRACTION_H
