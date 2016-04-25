/**
 *  @brief		Base class for all TURAG feldbus devices
 *  @file		device.h
 *  @date		22.03.2013
 *  @author		Martin Oemus
 *
 */


#ifndef TURAGFELDBUSDEVICE_H_
#define TURAGFELDBUSDEVICE_H_

#include <tina++/tina.h>
#include <tina++/thread.h>
#include <tina++/time.h>
#include <tina/feldbus/protocol/turag_feldbus_bus_protokoll.h>
#include "feldbusabstraction.h"

#if !TURAG_USE_TURAG_FELDBUS_HOST
# warning TURAG_USE_TURAG_FELDBUS_HOST must be defined to 1
#endif


/**
 * \addtogroup feldbus-host
 * @{
 */

/// Definiert, wie oft eine Übertragung wiederholt wird, bis mit einem Fehler
/// abgebrochen wird, falls im Konstruktor kein anderer Wert übergeben wird.
#if !defined(TURAG_FELDBUS_DEVICE_CONFIG_MAX_TRANSMISSION_ATTEMPTS) || defined(__DOXYGEN__)
# define TURAG_FELDBUS_DEVICE_CONFIG_MAX_TRANSMISSION_ATTEMPTS		5
#endif

/// Definiert, wieviele Übetragungen hintereinander fehlschlagen müssen,
/// damit das Gerät als dysfunktional deklariert wird, falls im Konstruktor
/// kein anderer Wert angegeben wird.
#if !defined(TURAG_FELDBUS_DEVICE_CONFIG_MAX_TRANSMISSION_ERRORS) || defined(__DOXYGEN__)
# define TURAG_FELDBUS_DEVICE_CONFIG_MAX_TRANSMISSION_ERRORS			35
#endif

/// Checksummen-Algorithmus der beim Instanziieren von Feldbusklassen
/// standardmäßig benutzt wird.
#if !defined(TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE) || defined(__DOXYGEN__)
# define TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE			TURAG::Feldbus::Device::ChecksumType::crc8_icode
#endif

/// Standardmäßige Adresslänge.
#if !defined(TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_ADDRESS_LENGTH) || defined(__DOXYGEN__)
# define TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_ADDRESS_LENGTH		TURAG::Feldbus::Device::AddressLength::byte_
#endif

/*!
 * @}
 */



namespace TURAG {

/**
 * @brief TURAG-Feldbus host classes
 * @ingroup feldbus-host
 */	
namespace Feldbus {

/**
 * \brief Basis-Klasse aller Feldbus-Geräte.
 * 
 * Die Device-Klasse implementiert auf Hostseite das Basis-Protkoll
 * des %TURAG-Feldbus.
 * 
 * Zu Beginn der Initialisierung jedes Gerätes sollte isAvailable() aufgerufen werden,
 * damit nicht funktionierende Geräte nicht erst zum Zeitpunkt ihres ersten Einsatzes
 * identifiziert werden, da das u.U. etwas dauern kann.
 * 
 * Während des Betriebs kann isAvailable() jederzeit benutzt werden um den Status
 * des Gerätes zu prüfen, da dieser Aufruf nur einmalig Buslast verursacht.
 * 
 * Instanzen dieser Klasse können je nach Bedarf mit verschiedenen Adresslängen 
 * gebildet werden.
 * 
 */
class Device {
public:
	/*!
	 * \brief Verfügbare Checksummenalgorithmen.
	 * \see \ref checksums
	 */
	enum class ChecksumType {
		xor_based = TURAG_FELDBUS_CHECKSUM_XOR, ///< XOR-Checksumme.
        crc8_icode = TURAG_FELDBUS_CHECKSUM_CRC8_ICODE ///< CRC8-Checksumme.
	};
	
	/**
	 * \brief Verfügbare Adresslängen.
	 */
	enum class AddressLength {
		// do not change the values! They are casted to
		// int and used for pointer arithmetics.
		// Also the size of the address fields in the following templates
		// equal the largest element.
		byte_ = 1, 	///< Adresse umfasst 1 Byte.
		short_ = 2	///< Adresse umfasst 2 Byte.
	};

    /*!
     * \brief Struktur-Template zur Verwendung mit transceive().
	 * 
	 * address und checksum werden dann automatisch befüllt.
     */
    template<typename T = void>
    struct Broadcast {
		uint8_t address[2];
        uint8_t id;
        T data;
        uint8_t checksum;
    } _packed;

	/**
	 * \brief Struktur-Template zur Verwendung mit transceive().
	 * 
	 * address und checksum werden dann automatisch befüllt.
	 */
    template<typename T = void>
    struct Request {
		uint8_t address[2];
        T data;
        uint8_t checksum;
    } _packed;

	/**
	 * \brief Struktur-Template zur Verwendung mit transceive().
	 * 
	 * address und checksum werden dann automatisch befüllt.
	 */
    template<typename T = void>
    struct Response {
		uint8_t address[2];
        T data;
        uint8_t checksum;
    } _packed;

    /*!
     * \brief Speichert das Device-Info-Paket eines Slave-Gerätes.
     *
     * Diese Struktur entspricht vom Speicherlayout nicht dem, was der
     * Slave über den Bus sendet.
     */
    struct DeviceInfo {
        uint8_t deviceProtocolId;
        uint8_t deviceTypeId;
        uint8_t crcType;
        uint32_t bufferSize;
        uint8_t nameLength;
        uint8_t versioninfoLength;
        uint16_t uptimeFrequency;
        bool packageStatisticsAvailable;
		
		DeviceInfo() :
			deviceProtocolId(0),
			deviceTypeId(0),
			crcType(0),
			bufferSize(0),
			nameLength(0),
			versioninfoLength(0),
			uptimeFrequency(0),
			packageStatisticsAvailable(false)
		{}
    };


	/**
	 * \brief Konstruktor.
	 * \param[in] name_
	 * \param[in] address
     * \param[in] feldbus Pointer auf eine FeldbusAbstraction-Instanz, an die die
     * eigentliche Datenübertragung delegiert wird.
	 * \param[in] type
	 * \param[in] max_transmission_attempts
	 * \param[in] max_transmission_errors
	 * \param[in] addressLength
	 */
    Device(const char* name_, unsigned address, FeldbusAbstraction* feldbus,
           ChecksumType type = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE,
           const AddressLength addressLength = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_ADDRESS_LENGTH,
		   unsigned int max_transmission_attempts = TURAG_FELDBUS_DEVICE_CONFIG_MAX_TRANSMISSION_ATTEMPTS,
		   unsigned int max_transmission_errors = TURAG_FELDBUS_DEVICE_CONFIG_MAX_TRANSMISSION_ERRORS);

#if TURAG_USE_LIBSUPCPP_RUNTIME_SUPPORT
    virtual ~Device() { }
#endif

	/**
	 * \brief Gibt zurück, ob das Gerät verfügbar ist.
	 * \param[in] forceUpdate Gibt an, ob in jedem Fall ein 
	 * Ping-Paket versendet werden soll, um eine möglichst aktuelle
	 * Information zu erhalten.
	 * \return True wenn das Gerät verfügbar ist, ansonsten false.
	 * 
	 * Diese Funktion informiert über den Verfügbarkeitsstatus des
	 * Gerätes. Dazu versucht sie bei ihrem ersten Aufruf solange
	 * ein Ping-Paket zu senden, bis dies entweder erfolgreich ist
	 * oder die maximale Anzahl an Fehler erreicht wurde. 
	 * 
	 * Im Normalfall gelingt die Übertragung des Ping-Paketes. Von da
	 * an verursacht die Funktion keine Buslast mehr, sondern gibt lediglich
	 * den aktuellen Fehlerstatus des Gerätes zurück. Dadurch ist diese
	 * Funktion bestens geeignet, wenn eine Verfügbarkeitsinformation 
	 * mit einer großen Häufiigkeit, doch mittleren Verlässlichkeit benötigt wird. Ist eine
	 * besonders aktuelle Information nötig, so kann das Verhalten des ersten
	 * Aufrufs mit dem forceUpdate-Parameter erzwungen werden - in diesem Fall
	 * wird stets mindestens ein Paket gesendet.
	 * 
	 * Gibt die Funktion einmal false zurück, so wird sich daran von selbst
	 * nichts mehr ändern, weil zu diesem Zeitpunkt sämtliche Pakete verworfen
	 * werden, außer wenn forceUpdate benutzt wird. Gibt diese Funktion false zurück
	 * obwohl das Gerät funktionieren sollte, so kann entweder mit clearTransmissionCounters()
	 * der interne Fehler-Counter zurückgesetzt oder diese Funktion mit ForceUpdate
	 * benutzt werden um das erneute Versenden von Paketen zu erzwingen.
	 */
    bool isAvailable(bool forceUpdate = false);
	
	/*!
	 * \brief Gibt die Adresse des Gerätes zurück.
	 * \return Adresse des Gerätes.
	 */
    unsigned getAddress(void) const { return myAddress; }
	
	/*!
	 * \brief Gibt die Geräte-Info zurück.
	 * \param[out] device_info Pointer auf DeviceInfo, was nach dem Aufruf die Daten
	 * enthält.
	 * \return True bei erfolgreicher Übertragung, ansonsten false.
	 * 
	 * Da sich die Geräteinformation nicht während des Betriebs ändern
	 * können, werden diese beim ersten erfolgreichen Aufruf von getDeviceInfo()
	 * gepuffert, wodurch folgende Aufrufe keine Buslast mehr verursachen.
	 * 
	 * Wird die Device-Info von innerhalb dieser oder einer Kindklasse benötigt,
	 * so kann diese Funktion mit null als Argument aufgerufen werden.
	 * Die Daten werden dann in myDeviceInfo gepuffert (falls dies nicht bereits
	 * zu einem früheren Zeitpunkt geschehen ist) und können von dort
	 * ausgelesen werden.
	 */
	bool getDeviceInfo(DeviceInfo* device_info);
   
	/*!
	 * \brief Empfängt die im Gerät hinterlegte Bezeichnung des Slaves.
	 * \param[out] out_real_name Char-Array, das den String aufnimmt.
	 * \return True bei erfolgreichem Empfang der Gerätebezeichnung.
	 * 
	 * Diese Funktion puffert den empfangenen String nicht und verursacht 
	 * daher bei jedem Aufruf erneute Buslast.
	 * 
	 * \warning Die DeviceInfo-Struktur gibt die Länge des Gerätenamens 
	 * ohne Nullzeichen an, während diese Funktion einen null-terminierten
	 * String zurückgibt. Daher muss out_real_name ein Byte größer reserviert
	 * werden!
	 */
    bool receiveDeviceRealName(char* out_real_name);
	
	/*!
	 * \brief Empfängt die Versionsinfo des Gerätes.
	 * \param[out] out_version_info Char-Array, das den String aufnimmt.
	 * \return True bei erfolgreichem Empfang der Versionsinfo.
	 * 
	 * Diese Funktion puffert den empfangenen String nicht und verursacht 
	 * daher bei jedem Aufruf erneute Buslast.
	 * 
	 * \warning Die DeviceInfo-Struktur gibt die Länge der Versionsinfo 
	 * ohne Nullzeichen an, während diese Funktion einen null-terminierten
	 * String zurückgibt. Daher muss out_version_info ein Byte größer reserviert
	 * werden!
	 */
	bool receiveVersionInfo(char* out_version_info);
	
	/**
	 * \brief Fragt die Uptime des Gerätes ab.
	 * \param[out] uptime Puffer in dem die Uptime gespeichert wird.
	 * \return True bei Erfolg, ansonsten false.
	 * 
	 * Angabe in Sekunden.
	 * 
	 * Wenn das Gerät die eigene Uptime nicht zählt, so wird 
	 * NaN in uptime geschrieben.
	 */
	bool receiveUptime(float* uptime);
	
	/**
	 * \brief Fragt die Anzahl korrekt empfangener Pakete vom Slave ab.
	 * \param[out] packageCount Puffer in dem der Wert gespeichert wird.
	 * \return True bei Erfolg, ansonsten false.
	 * \see https://intern.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus/#fehlerzustaende
	 */
	bool receiveNumberOfAcceptedPackages(uint32_t* packageCount);
	
	/**
	 * \brief Fragt die Anzahl aufgetretener Pufferüberlaufe vom Slave ab.
	 * \param[out] overflowCount Puffer in dem der Wert gespeichert wird.
	 * \return True bei Erfolg, ansonsten false.
	 * \see https://intern.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus/#fehlerzustaende
	 */
	bool receiveNumberOfOverflows(uint32_t* overflowCount);
	
	/**
	 * \brief Fragt die Anzahl verlorener Pakete vom Slave ab.
	 * \param[out] lostPackagesCount Puffer in dem der Wert gespeichert wird.
	 * \return True bei Erfolg, ansonsten false.
	 * \see https://intern.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus/#fehlerzustaende
	 */
	bool receiveNumberOfLostPackages(uint32_t* lostPackagesCount);
	
	/**
	 * \brief Fragt die Anzahl aufgetretener Checksummenfehler vom Slave ab.
	 * \param[out] checksumErrorCount Puffer in dem der Wert gespeichert wird.
	 * \return True bei Erfolg, ansonsten false.
	 * \see https://intern.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus/#fehlerzustaende
	 */
	bool receiveNumberOfChecksumErrors(uint32_t* checksumErrorCount);
	
	/**
	 * \brief Fragt die Anzahl korrekt empfangener Pakete, aufgetretener Pufferüberlaufe, verlorener Pakete und 
	 * aufgetretener Checksummenfehler vom Slave ab.
	 * \param[out] counts Puffer in dem der Wert gespeichert wird.
	 * \return True bei Erfolg, ansonsten false.
	 * \see https://intern.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus/#fehlerzustaende
	 * 
	 * Es werden mit einem Mal alle 4 Werte abgefragt, der Puffer muss also eine
	 * entsprechende Größe aufweisen.
	 */
	bool receiveAllSlaveErrorCount(uint32_t* counts);
	
	/**
	 * \brief Weist das Gerät an, seine internen Paketstatistiken zurückzusetzen.
	 * \return True bei Erfolg, anonsten false.
	 */
	bool resetSlaveErrors(void);
	
	/**
	 * \brief Sendet ein Ping-Paket.
	 * \return True, wenn des Gerät erwartungsgemäß geantwortet hat, false
	 * im Fehlerfall.
	 *
	 * Diese Funktion sendet immer ein Paket, auch wenn das Gerät dysfunktional ist.
	 * Ist die Übertragung erfolgreich, so wird das Gerät wieder als funktionieren
	 * betrachtet.
	 */
	bool sendPing(void);

	/**
	 * \brief Gibt die Anzahl der im Master aufgetretenen Checksummenfehler zurück.
	 * \return Anzahl der Checksummenfehler.
	 * \see https://intern.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus/#fehlerzustaende
	 */
	unsigned int getChecksumErrors(void) const { return myTotalChecksumErrors; }
	
	/**
	 * \brief Gibt die Anzahl der im Master aufgetretenen Antwortfehler zurück.
	 * \return Anzahl der Antwortfehler.
	 * \see https://intern.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus/#fehlerzustaende
	 */
	unsigned int getNoAnswerErrors(void) const { return myTotalNoAnswerErrors; }
	
	/**
	 * \brief Gibt die Anzahl der im Master aufgetretenen Datenfehler zurück.
	 * \return Anzahl der Datenfehler.
	 * \see https://intern.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus/#fehlerzustaende
	 */
	unsigned int getMissingDataErrors(void) const { return myTotalMissingDataErrors; }

	/**
	 * \brief Gibt die Anzahl der im Master aufgetretenen Sendefehler zurück.
	 * \return Anzahl der Sendefehler.
	 * \see https://intern.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus/#fehlerzustaende
	 */
	unsigned int getTransmitErrors(void) const { return myTotalTransmitErrors; }

	/**
	 * \brief Gibt die Anzahl aller aufgetretenen Fehler zurück.
	 * \return Gesamtzahl der Fehler.
	 */
	unsigned int getAllErrors(void) const { return myTotalChecksumErrors + myTotalNoAnswerErrors + myTotalMissingDataErrors + myTotalTransmitErrors; }
	
	/**
	 * \brief Gibt die Anzahl der durchgeführten Datenübertragungen zurück.
	 * \return Anzahl der durchgeführten Datenübertragungen.
	 * 
	 * Dieser Wert repräsentiert die Gesamtanzahl an Übertragungsversuchen, schließt also 
	 * die Fehlerfälle mit ein.
	 */
	unsigned int getTotalTransmissions(void) const { return myTotalTransmissions; }

    /**
	 * \brief Setzt die Error-Counter und den Übertragungs-Counter auf null zurück.
	 * 
	 * Nach Aufruf dieser Funktion wird ein Gerät nicht mehr als dysfunktional betrachtet
	 * und Übertragungen werden erneut versucht.
	 */
    void clearTransmissionCounters(void) { 
		myTotalChecksumErrors = 0;
		myTotalNoAnswerErrors = 0;
		myTotalMissingDataErrors = 0;
		myTotalTransmitErrors = 0;
		myTotalTransmissions = 0;
		myCurrentErrorCounter = 0; 
		hasCheckedAvailabilityYet = false;
	}

	/**
	 * @brief Anzahl aller am Bus aufgetretenen Übertragungsfehler zurückgeben
	 * @return Anzahl der Fehler.
	 */
	int getGlobalTransmissionErrors(void) const;

    /*!
	 * \brief Blockierende Standard-Datenüberträgung.
	 * \param[in] transmit Zu sendende Daten, verpackt in einer Request-Struktur.
	 * \param[out] receive Pointer auf eine Response-Struktur, die nach dem Aufruf die
	 * Antwortdaten enthält.
	 * \param[in] ignoreDysfunctional Sendet das Paket auch, wenn das Gerät dysfunktional
	 * ist.
	 * \return True bei erfolgreicher Übertragung.
	 * 
	 * Sendet ein Datenpaket an das Slavegerät und blockiert solange, bis entweder 
	 * die erwartete Anzahl an Daten eingetroffen ist oder ein timeout erreicht wurde.
	 * 
	 * Diese Funktion benutzt intern die plattformabhängige Funktion turag_rs485_transceive(),
	 * deren Implementierung bestimmt, wie die Übertragung im Detail stattfindet.
	 */
	template<typename T, typename U> _always_inline
	bool transceive(Request<T>& transmit, Response<U>* receive, bool ignoreDysfunctional = false) {
	return transceive(reinterpret_cast<uint8_t*>(&(transmit)) + sizeof(Request<T>::address) - myAddressLength,
					  sizeof(Request<T>) + myAddressLength - sizeof(Request<T>::address),
					  reinterpret_cast<uint8_t*>(receive) + sizeof(Response<T>::address) - myAddressLength,
					  sizeof(Response<U>) + myAddressLength - sizeof(Response<T>::address),
					  ignoreDysfunctional);
	}

	/*!
	 * \brief Versendet einen Broadcast.
	 * \param[out] transmit Zu sendende Daten, verpackt in eine Broadcast-Struktur.
	 * \param[in] ignoreDysfunctional Sendet das Paket auch, wenn das Gerät dysfunktional
	 * ist.
	 * \return True bei erfolgreicher Übertragung.
	 * 
	 * Sendet einen %Broadcast an die in transmit eingestellte Gerätegruppe und
	 * kehrt zurück, sobald alle Daten geschrieben wurden.
	 */
    template<typename T> _always_inline
	bool transceive(Broadcast<T>& transmit, bool ignoreDysfunctional = false) {
		return transceive(reinterpret_cast<uint8_t*>(&(transmit)) + sizeof(Broadcast<T>::address) - myAddressLength,
						  sizeof(Broadcast<T>) + myAddressLength - sizeof(Broadcast<T>::address),
						  nullptr,
						  0,
						  ignoreDysfunctional);
	}

	/**
	 * @brief Gibt das erste Feldbus-Device zurück.
	 * @return Zeiger auf das erste Feldbus-Device im System.
	 *
	 * Über alle Instanzen von Device wird eine List geführt,
	 * über die zu Informationszwecken iteriert werden kann.
	 *
	 * \see getNextDevice()
	 */
	static Device* getFirstDevice(void) {
		return firstDevice;
	}

	/**
	 * @brief Gibt das nächste Feldbus-Device zurück.
	 * @return Zeiger auf das nächste Feldbus-Device im System.
	 *
	 * \see getFirstDevice()
	 */
	Device* getNextDevice(void) const {
		return myNextDevice;
	}

    /*!
     * \brief Gerätename.
	 * 
	 * Hierbei handelt es sich um den Master-seitig vergebenen Gerätenamen, der 
	 * sich durchaus von der Bezeichnung unterscheiden kann, die ein Slave über
	 * sich selbst zurück gibt.
	 * 
	 * \see receiveDeviceRealName
     */
    const char* name;

protected:
    /*!
     * \brief Sendet Daten zum Slave und empfängt eine Antwort.
     * \param[in] transmit Puffer der zu übertragenden Daten.
     * \param[in] transmit_length Länge des Puffers.
     * \param[out] receive Puffer für die Empfangsdaten.
     * \param[in] receive_length Länge des Empfangspuffers/der erwarteten Daten.
	 * \param[in] ignoreDysfunctional Sendet das Paket auch, wenn das Gerät dysfunktional
	 * ist.
     * \return True bei erfolgreicher Übertragung.
	 * 
	 * Diese Funktion kann benutzt werden, um High-Level-Zugriffsfunktionen in
	 * abgeleiteten Klassen bereitzustellen. Wenn möglich sollte allerdings 
	 * das transceive-Funktions-Template in Verbindung mit Request und Response bzw.
	 * Broadcast bevorzugt werden.
	 * 
	 * \warning Diese Funktion überschreibt den Anfang des Puffers mit der 
	 * Adresse (je nach Adresslänge) und den Ende des Puffers mit der Checksumme.
	 * Die Nutzdaten müssen also dazwischen platziert werden.
	 * 
	 * Wenn receive oder receive_length 0 ist, so wird angenommen dass eine 
	 * Broadcastübertragung gewünscht wird und statt der Geräteadresse 0 benutzt.
     */
	bool transceive(uint8_t *transmit, int transmit_length, uint8_t *receive, int receive_length, bool ignoreDysfunctional = false);

	/**
	 * \brief Gibt zurück, ob das Gerät als dysfunktional betrachtet wird.
	 * \return Wenn das Gerät dysfunktional ist true, ansonsten false.
	 * 
	 * Ein Gerät wird als dysfunktional eingestuft, wenn hintereinander zu viele 
	 * Übertragungsfehler regisitriert werden. Von diesem Zeitpunkt an werden alle
	 * Datenübertragungen abgebrochen.
	 * 
	 * Diese Funktion wird von der isAvailable()-Funktion verwendet, welche
	 * statt dieser benutzt werden sollte, da isAvailable() mehr High-Level-Logik
	 * implementiert.
	 */
    bool isDysfunctional(void) const { return myCurrentErrorCounter >= maxTransmissionErrors; }
    
	/**
	 * \brief Enthält die Geräteadresse.
	 */
    const unsigned myAddress;
	
	/**
	 * \brief Enthält, auf welche Adresslänge das Gerät konfiguriert ist.
	 */
	const unsigned myAddressLength;
	
	/**
	 * \brief Wird von isAvailable() bei der ersten Auführung auf
	 * true gesetzt.
	 */
    bool hasCheckedAvailabilityYet;
	
	/**
	 * \brief Puffert die DeviceInfo-Struktur des Gerätes.
	 */
    DeviceInfo myDeviceInfo;

private:
	bool receiveString(uint8_t command, uint8_t stringLength, char* out_string);
	bool receiveErrorCount(uint8_t command, uint32_t* buffer);
	
    const unsigned int maxTransmissionAttempts;
    const unsigned int maxTransmissionErrors;

    ChecksumType myChecksumType;
	
    unsigned int myCurrentErrorCounter;
	
    unsigned int myTotalTransmissions;
	unsigned int myTotalChecksumErrors;
	unsigned int myTotalNoAnswerErrors;
	unsigned int myTotalMissingDataErrors;
	unsigned int myTotalTransmitErrors;

	static int globalTransmissionErrorCounter;
	static unsigned addressOfLastTransmission;
	static Mutex mutex;

	Device* myNextDevice;
	static Device* firstDevice;

    FeldbusAbstraction* bus;
};



// specializations of the container templates

/**
* \brief Struktur-Template zur Verwendung mit transceive().
* 
* Für Übertragungen, die nur aus Adresse, Broadcast-ID und Checksumme bestehen.
*/
template<>
struct Device::Broadcast<void> {
	uint8_t address[2];
    uint8_t id;
    uint8_t checksum;
} _packed;

/**
* \brief Struktur-Template zur Verwendung mit transceive().
* 
* Für Übertragungen, die nur aus Adresse und Checksumme bestehen.
*/
template<>
struct Device::Request<void> {
	uint8_t address[2];
    uint8_t checksum;
} _packed;

/**
* \brief Struktur-Template zur Verwendung mit transceive().
* 
* Für Übertragungen, die nur aus Adresse und Checksumme bestehen.
*/
template<>
struct Device::Response<void> {
	uint8_t address[2];
    uint8_t checksum;
} _packed;



} // namespace Feldbus
} // namespace TURAG

#endif /* TURAGFELDBUSDEVICE_H_ */
