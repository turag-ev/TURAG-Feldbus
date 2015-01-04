/**
 *  @brief		Base class for all TURAG feldbus devices
 *  @file		device.h
 *  @date		22.03.2013
 *  @author		Martin Oemus
 *
 */


#ifndef TURAGFELDBUSDEVICE_H_
#define TURAGFELDBUSDEVICE_H_

#include <memory>
#include <atomic>

#include <tina++/tina.h>
#include <tina++/thread.h>
#include <tina/feldbus/protocol/turag_feldbus_bus_protokoll.h>

/**
 * Host-Klassen, die zur Kommunikation mit Busgeräten benutzt werden können.
 * 
 * Die Basis aller Hosts-Klassen ist \ref TURAG::Feldbus::Device, das masterseitigen 
 * Support für das Basis-Protokoll bereitstellt.
 * 
 * Das Verhalten des Masters kann mit einigen Definitionen angepasst werden, für die
 * sinnvolle Standardwerte eingestellt sind. Bei Bedarf können diese über die TinA-Konfiguration
 * entsprechend überschrieben werden.
 * 
 * \section feldbus-host-threadsafety Thread-Safety
 * \note Generell ist aus Gründen der Einfachheit zu empfehlen, alle Host-Klassen
 * exklusiv von einem einzigen %Thread aus zu benutzen und den Austausch von Daten
 * über andere Mechanismen zu gewährleisten. Dadurch ist der Zugriff auf den 
 * Bus geordnet und die Komplexität des System wird nicht unnnötig gesteigert.
 * Die folgenden Informationen sind der Vollständigkeit halber angegeben.
 * 
 * Damit der Buszugriff mit mehreren Threads reibungslos funktioniert, sind
 * folgende Voraussetzungen zu erfüllen:
 * - Host-Klassen müssen mindestens reentrant sein
 * - Zugriff auf die Bus-Hardware muss synchronisert sein
 * - Bus-Transaktionen müssen atomaren Charakter besitzen.
 * 
 * Die Host-Klassen sind reentrant, aber nicht thread-safe. Das heißt, der Zugriff 
 * auf eine Instanz einer Klasse muss manuell synchronisiert werden. Unproblematisch 
 * ist hingegen die Verwendung verschiedener Instanzen einer Klasse in verschiedenen Threads.
 * Dies ist eine übliche Einschränkung, da die Gewährleistung von Threadsicherheit
 * für jede Funktion einen zu großen Overhead mit sich bringen würde. Soll ein Busgerät
 * von mehreren Threads angesprochen werden, so müssen entweder zwei Instanzen 
 * für jeden %Thread angelegt werden oder jeder Aufruf von nicht-immutable Funktionen
 * synchronisert werden. Welche Variante sinnvoller ist, hängt stark von den Details
 * der entsprechenden Klasse ab.
 * 
 * Die Synchronisierung des Zugriffs auf die Bus-Hardware obliegt der Platform-Implementierung
 * der Funktion turag_rs485_transceive(), welche das Versenden und den Empfang einer bestimmten 
 * Datenmenge bereitstellt.
 * 
 * Atomizität der Bus-Transaktionen bedeutet, dass zu einem Befehl gehörende 
 * Datenübertragungen nicht von fremden unterbrochen werden können. Dies wird zu einem
 * großen Teil schon von der Anforderung gewährleistet, dass alle Protokolle zustandslos
 * sein müssen, wodurch es unproblematisch ist, wenn Pakete zwischeneinander gemischt werden.
 * In der Praxis bedeutet das üblicherweise, dass eine Funktion einer Hostklasse maximal 
 * einmal turag_rs485_transceive() aufrufen kann. 
 * 
 * 
 * 
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
#ifndef TURAG_FELDBUS_DEVICE_CONFIG_MAX_TRANSMISSION_ERRORS
# define TURAG_FELDBUS_DEVICE_CONFIG_MAX_TRANSMISSION_ERRORS			35
#endif

/// Checksummen-Algorithmus der beim Instanziieren von Feldbusklassen
/// standardmäßig benutzt wird.
#ifndef TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE
# define TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE			TURAG::Feldbus::Device::ChecksumType::crc8_icode
#endif

/// Stellt die Größe der Geräteadressen auf dem Bus ein.
/// Alle Slaves und der Master müssen hier die gleiche Einstellung 
/// verwenden.
#ifndef TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH
# define TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH					1
#endif

/*!
 * @}
 */




namespace TURAG {

/** 
 * @addtogroup feldbus-host
 * @{
 */
 
	
/**
 * @brief TURAG-Feldbus host classes
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
 */
class Device {
public:
	
	/**
	 * Typ, der die Geräteadresse speichert. kann 8 oder 16 Bit lang sehen,
	 * je nach Einstellung von \ref TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH.
	 */
#if TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH == 1 || defined(__DOXYGEN__)
		typedef uint8_t FeldbusAddressType;
#elif TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH == 2
		typedef uint16_t FeldbusAddressType;
#else
# error TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH with invalid value
#endif

	/*!
	 * Verfügbare Checksummenalgorithmen.
	 * \see \ref checksums
	 */
	enum class ChecksumType {
		xor_based = TURAG_FELDBUS_CHECKSUM_XOR, ///< XOR-Checksumme.
        crc8_icode = TURAG_FELDBUS_CHECKSUM_CRC8_ICODE ///< CRC8-Checksumme.
	};

    /*!
     *
     */
    template<typename T = void>
    struct Broadcast {
        FeldbusAddressType address; // must be 0
        uint8_t id;
        T data;
        uint8_t checksum;
    } _packed;

    template<typename T = void>
    struct Request {
        FeldbusAddressType address;
        T data;
        uint8_t checksum;
    } _packed;

    template<typename T = void>
    struct Response {
        FeldbusAddressType address;
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
        uint8_t bufferSize;
        uint8_t nameLength;
        uint8_t versioninfoLength;
        uint16_t uptimeFrequency;
        bool packageStatisticsAvailable;
    };


	/**
	 * \brief Konstruktor.
	 * \param[in] name_
	 * \param[in] address
	 * \param[in] type
	 * \param[in] max_transmission_attempts
	 * \param[in] max_transmission_errors
	 */
    Device(const char* name_, FeldbusAddressType address, ChecksumType type = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE,
           unsigned int max_transmission_attempts = TURAG_FELDBUS_DEVICE_CONFIG_MAX_TRANSMISSION_ATTEMPTS,
           unsigned int max_transmission_errors = TURAG_FELDBUS_DEVICE_CONFIG_MAX_TRANSMISSION_ERRORS) :
        name(name_),
        myAddress(address),
        hasCheckedAvailabilityYet(false),
        maxTransmissionAttempts(max_transmission_attempts),
        maxTransmissionErrors(max_transmission_errors),
        myChecksumType(type),
		myCurrentErrorCounter(0),
		myTotalTransmissions(0),
		myTotalChecksumErrors(0),
		myTotalNoAnswerErrors(0),
		myTotalMissingDataErrors(0),
		myTotalTransmitErrors(0)
    {
        myDeviceInfo.bufferSize = 0;
    }

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
	 * Aufrufs mit dem forceUpdate-Parameter erzwungen werden.
	 * 
	 * Gibt die Funktion einmal false zurück, so wird sich daran von selbst
	 * nichts mehr ändern, weil zu diesem Zeitpunkt sämtliche Pakete verworfen
	 * werden. Ist man der Meinung, dass das Gerät wieder funktionieren könnte,
	 * können die internen Fehlerzustände mit clearTransmissionCounters()
	 * zurückgesetzt werden.
	 */
    bool isAvailable(bool forceUpdate = false);
	
	/*!
	 * \brief Gibt die Adresse des Gerätes zurück.
	 * \return Adresse des Gerätes.
	 */
    FeldbusAddressType getAddress(void) const { return myAddress; }
	
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
	 * \see https://www.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus/#fehlerzustaende
	 */
	bool receiveNumberOfAcceptedPackages(uint32_t* packageCount);
	
	/**
	 * \brief Fragt die Anzahl aufgetretener Pufferüberlaufe vom Slave ab.
	 * \param[out] overflowCount Puffer in dem der Wert gespeichert wird.
	 * \return True bei Erfolg, ansonsten false.
	 * \see https://www.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus/#fehlerzustaende
	 */
	bool receiveNumberOfOverflows(uint32_t* overflowCount);
	
	/**
	 * \brief Fragt die Anzahl verlorener Pakete vom Slave ab.
	 * \param[out] lostPackagesCount Puffer in dem der Wert gespeichert wird.
	 * \return True bei Erfolg, ansonsten false.
	 * \see https://www.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus/#fehlerzustaende
	 */
	bool receiveNumberOfLostPackages(uint32_t* lostPackagesCount);
	
	/**
	 * \brief Fragt die Anzahl aufgetretener Checksummenfehler vom Slave ab.
	 * \param[out] checksumErrorCount Puffer in dem der Wert gespeichert wird.
	 * \return True bei Erfolg, ansonsten false.
	 * \see https://www.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus/#fehlerzustaende
	 */
	bool receiveNumberOfChecksumErrors(uint32_t* checksumErrorCount);
	
	/**
	 * \brief Fragt die Anzahl korrekt empfangener Pakete, aufgetretener Pufferüberlaufe, verlorener Pakete und 
	 * aufgetretener Checksummenfehler vom Slave ab.
	 * \param[out] counts Puffer in dem der Wert gespeichert wird.
	 * \return True bei Erfolg, ansonsten false.
	 * \see https://www.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus/#fehlerzustaende
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
	 */
	bool sendPing(void);

	/**
	 * \brief Gibt die Anzahl der im Master aufgetretenen Checksummenfehler zurück.
	 * \return Anzahl der Checksummenfehler.
	 * \see https://www.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus/#fehlerzustaende
	 */
	unsigned int getChecksumErrors(void) const { return myTotalChecksumErrors; }
	
	/**
	 * \brief Gibt die Anzahl der im Master aufgetretenen Antwortfehler zurück.
	 * \return Anzahl der Antwortfehler.
	 * \see https://www.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus/#fehlerzustaende
	 */
	unsigned int getNoAnswerErrors(void) const { return myTotalNoAnswerErrors; }
	
	/**
	 * \brief Gibt die Anzahl der im Master aufgetretenen Datenfehler zurück.
	 * \return Anzahl der Datenfehler.
	 * \see https://www.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus/#fehlerzustaende
	 */
	unsigned int getMissingDataErrors(void) const { return myTotalMissingDataErrors; }

	/**
	 * \brief Gibt die Anzahl der im Master aufgetretenen Sendefehler zurück.
	 * \return Anzahl der Sendefehler.
	 * \see https://www.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus/#fehlerzustaende
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

    /*!
	 * \brief Blockierende Standard-Datenüberträgung.
	 * \param[in] transmit Zu sendende Daten, verpackt in einer Request-Struktur.
	 * \param[out] receive Pointer auf eine Response-Struktur, die nach dem Aufruf die
	 * Antwortdaten enthält.
	 * \return True bei erfolgreicher Übertragung.
	 * 
	 * Sendet ein Datenpaket an das Slavegerät und blockiert solange, bis entweder 
	 * die erwartete Anzahl an Daten eingetroffen ist oder ein timeout erreicht wurde.
	 * 
	 * Diese Funktion benutzt intern die plattformabhängige Funktion turag_rs485_transceive(),
	 * deren Implementierung bestimmt, wie die Übertragung im Detail stattfindet.
	 */
	template<typename T, typename U> _always_inline
	bool transceive(Request<T>& transmit, Response<U>* receive) {
	transmit.address = myAddress;
	return transceive(reinterpret_cast<uint8_t*>(std::addressof(transmit)),
										sizeof(Request<T>),
										reinterpret_cast<uint8_t*>(receive),
										sizeof(Response<U>));
	}

	/*!
	 * \brief Versendet einen Broadcast.
	 * \param[out] transmit Zu sendende Daten, verpackt in eine Broadcast-Struktur.
	 * \return True bei erfolgreicher Übertragung.
	 * 
	 * Sendet einen %Broadcast an die in transmit eingestellte Gerätegruppe und
	 * kehrt zurück, sobald alle Daten geschrieben wurden.
	 */
    template<typename T> _always_inline
	bool transceive(Broadcast<T>& transmit) {
		transmit.address = TURAG_FELDBUS_BROADCAST_ADDR;
		return transceive(static_cast<uint8_t*>(static_cast<void*>(std::addressof(transmit))),
											sizeof(Broadcast<T>), nullptr, 0);
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
     * \return True bei erfolgreicher Übertragung.
	 * 
	 * Diese Funktion kann benutzt werden, um High-Level-Zugriffsfunktionen in
	 * abgeleiteten Klassen bereitzustellen. Wenn möglich sollte allerdings 
	 * das transceive-Funktions-Template in Verbindung mit Request und Response bzw.
	 * Broadcast bevorzugt werden.
	 * 
	 * \warning Diese Funktion geht stillschweigend davon aus, dass das letzte Byte 
	 * des transmit-Puffers für die Checksumme vorgesehen ist und überschreibt
	 * dieses entsprechend.
     */
    bool transceive(uint8_t *transmit, int transmit_length, uint8_t *receive, int receive_length);

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
    const FeldbusAddressType myAddress;
	
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

	static std::atomic_int globalTransmissionErrorCounter;
};

/// for zero size requests/responces
template<>
struct Device::Broadcast<void> {
    uint8_t address; // must be 0
    uint8_t id;
    uint8_t checksum;
} _packed;

template<>
struct Device::Request<void> {
    uint8_t address;
    uint8_t checksum;
} _packed;

template<>
struct Device::Response<void> {
    uint8_t address;
    uint8_t checksum;
} _packed;



} // namespace Feldbus

/**
 * @}
 */

} // namespace TURAG

#endif /* TURAGFELDBUSDEVICE_H_ */
