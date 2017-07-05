/*
 * feldbus_slave_base.h
 *
 *  Created on: 20.11.2016
 *      Author: martin
 */

#ifndef SLAVE_FELDBUS_SLAVE_BASE_H_
#define SLAVE_FELDBUS_SLAVE_BASE_H_

/**
 * @defgroup feldbus-slave-base Basis-Implementierung
 * @ingroup feldbus-slave
 * 
 * TURAG-Feldbus-Basisimplementierung. Implementiert das Basis-Protokoll des %TURAG Feldbus.
 * 
 * \see \ref feldbus-slave
 * 
 * \section Struktur
 * 
 * \image html felbus-slave-chibios.svg "Strukturschema der TURAG-Feldbus-Basisimplementierung (AVR)"
 * \ref TURAG::Feldbus::Slave::Base behandelt die Verarbeitung von Standardpaketen, prüft und
 * generiert Checksummen, verwaltet Traffic-Statistiken und stellt eine Hilfsfunktion bereit, um das 
 * LED-Blinkmuster zu generieren. 
 * Es ist auf die Existenz einer plattform-abhängigen TURAG::Feldbus::Slave::Driver-Klasse angewiesen. 
 * 
 * Diese ist für Empfang, Versand und die initiale, adress-basierte Paket-Filterung zuständig. 
 * Empangene Pakete müssen an TURAG::Feldbus::Slave::Base::processPacket() übergeben und die Antwort 
 * versendet werden. Dies sollte in einem eigenen Arbeits-Thread geschehen. Treten Fehler auf, muss dies
 * durch Aufruf von TURAG::Feldbus::Slave::Base::increaseBufferOverflow() oder 
 * TURAG::Feldbus::Slave::Base::increasePacketLost() signalisiert werden. Weiterhin muss 
 * TURAG::Feldbus::Slave::Base::doLedPattern() mit einer festen Frequenz aufgerufen werden und Funktionen
 * bereitgestellt werden, um Debug-Daten auszugeben, sowie die LED tatsächlich zu schalten.
 * 
 * Die Funktionen beider Klassen sind statisch, um eine Weiterverwendung der Anwendungsprotokoll-
 * Implementierungen zu erlauben. Diese werden als Funktionspointer an TURAG::Feldbus::Slave::Base::init()
 * übergeben und, wenn zutreffend, im Kontext von TURAG::Feldbus::Slave::Base::processPacket() aufgerufen.
 * Dies macht es zwar unmöglich, gleichzeitig zwei verschiedene Slave-Treiber in einem Gerät zu benutzen,
 * im Kontext der vom Protokoll festgelegten Einfachheit von Feldbus-Geräten wäre der Nutzen 
 * allerdings sowieso fragwürdig.
 */


#include <tina++/tina.h>
#include <tina++/thread.h>
#include <tina++/crc.h>

#include <tina/feldbus/slave/feldbus_config_check.h>


namespace TURAG {
namespace Feldbus {
    
/**
 * @brief Feldbus-Slave-Klassen und Debug-Funktionen.
 * @ingroup feldbus-slave-base
 */
namespace Slave {

/**
 * @brief Plattform-unabhängiger Teil der Felbus-Slave Implementierung.
 * 
 * Diese Klassse ist auf die Existenz einer plattform-abhängigen
 * Slave::Driver-Klasse angewiesen.
 * 
 * \see \ref feldbus-slave-base
 */
class Base {
public:
	/**
	 * @brief Funktionstyp zur Weiterverarbeitung eines Pakets.
	 * @param[in] message Pointer auf den Datenteil des empfangenen Paketes (ohne Adresse).
	 * @param[in] message_length Länge des Datenteils des empfangenen Paketes (ohne Adresse und Checksumme).
	 * @param[out] response Puffer, in den die Antwort geschrieben werden kann. Die Länge entspricht 
     * TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE - (TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH + 1).
	 * @return Länge der in response geschriebenen Antwort.
     * 
     * Eine Funktion dieses Typs wird aufgerufen nachdem Adresse und Checksumme auf Korrektheit geprüft wurden.
	 */
	typedef FeldbusSize_t (*PacketProcessor)(const uint8_t* message, FeldbusSize_t message_length, uint8_t* response);

	/**
	 * @brief Funktionstyp zur Weiterverarbeitung eines Broadcasts.
	 * @param[in] message Pointer auf den Datenteil des empfangenen Broadcast-Paketes (ohne Adresse).
	 * @param[in] message_length Länge des Datenteils des empfangenen Paketes (ohne Adresse und Checksumme).
	 * @param[in] protocol_id Broadcast-Protokoll-ID.
     * 
     * Eine Funktion dieses Typs wird aufgerufen nachdem Adresse und Checksumme auf Korrektheit geprüft wurden.
	 */
	typedef void (*BroadcastProcessor)(const uint8_t* message, FeldbusSize_t message_length, uint8_t protocol_id);



	/**
	 * @brief Initialisiert den Treiber.
	 * @param[in] packetProcessor Pointer auf eine Funktion, die Pakete weiterverarbeitet.
	 * @param[in] broadcastProcessor Pointer auf eine Funktion, die Broadcasts weiterverarbeitet.
     * 
     * Mit den beiden Parametern können Funktionen angegeben werden, die die weitere Paketverarbeitung 
     * übernehmen, sofern es sich nicht um Standard-Pakete handelt. Sie werden dann im Kontext von
     * processPacket() aufgerufen.
	 */
	static void init(PacketProcessor packetProcessor = nullptr, BroadcastProcessor broadcastProcessor = nullptr);

	/**
	 * @brief Verarbeitet ein Packet.
	 * @param[in] message Nachricht incl. Adresse und Checksumme.
	 * @param[in] message_length Nachrichtenlänge.
	 * @param[out] response Antwort, die zurückgesendet werden soll.
	 * Es wird angenommen, dass die Adresse bereits eingetragen ist
	 * und wird folglich nicht von dieser Funktion geschrieben.
	 * @return Anzahl zurückzusendender Bytes. Bei 0 soll keine
	 * Antwort gesendet werden.
     * 
     * Diese Funktion sollte vom plattformabhängigen Teil des Slave-Treibers aufgerufen werden,
     * wenn ein Paket empfangen wurde und es eine korrekte Adresse aufweist.
     * Die Checksummenprüfung übernimmt diese Funktion.
	 */
	static FeldbusSize_t processPacket(const uint8_t* message, FeldbusSize_t message_length, uint8_t* response);

#if TURAG_FELDBUS_SLAVE_CONFIG_FLASH_LED || defined(__DOXYGEN__)
	/**
	 * @brief Erzeugt das charakteristische Feldbus-Blinkmuster.
	 * @param[in] frequency Frequenz, mit der diese Funktion aufgerufen wird.
     * 
     * Diese Funktion sollte vom plattformabhängigen Teil des Slave-Treibers mit 
     * gleichbleibender Häufigkeit aufgerufen werden. Durch die Angabe der Frequenz
     * wird TURAG::Feldbus::Slave::Driver::toggleLed() so aufgerufen, dass ein 
     * charakterstisches Blinkmuster entsteht.
	 */
	static void doLedPattern(unsigned frequency);
#endif
    
#if TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE || defined(__DOXYGEN__)
	/**
	 * @brief Inkrementiert den Zähler für verlorene Pakete.
     * 
     * Diese Funktion sollte vom plattformabhängigen Teil des Slave-Treibers aufgerufen werden,
     * wenn ein empfangenes Paket verworfen werden musste, da vor dessen Bearbeitung ein neues
     * empfangen wurde.
	 */
	static void increasePacketLost(void) {
		++info.packetcount_lost;
	}

	/**
	 * @brief Inkrementiert den Zähler für Pufferüberläufe.
     * 
     * Diese Funktion sollte vom plattformabhängigen Teil des Slave-Treibers aufgerufen werden,
     * wenn ein Paket nicht in den Eingangspuffer passt (vorausgesetzt, das Paket hatte die 
     * richtige Adresse).
	 */
	static void increaseBufferOverflow(void) {
		++info.packetcount_buffer_overflow;
	}
#endif

private:
	struct Info {
		const char* name;
		const char* versioninfo;
	#if TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE
		uint32_t packetcount_correct;
		uint32_t packetcount_buffer_overflow;
		uint32_t packetcount_lost;
		uint32_t packetcount_chksum_mismatch;
	#endif
	};

	static PacketProcessor packetProcessor;
	#if TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE
		static BroadcastProcessor broadcastProcessor;
	#endif

	static Info info;



};




// debugging functions
#if TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED || defined(__DOXYGEN__)
/** @name Debug-Functions
 * Diese Funktionen können verwendet werden, um Debugausgaben in Form
 * konventioneller Zeichenketten auf dem Bus auszugeben.
 *
 *
 * \pre Nur verfügbar, wenn \ref TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED auf 1 definiert ist.
 *
 */
///@{
	/**
     * @brief printf-artige Funktion.
     * @param[in] fmt Formatstring.
     * 
     * Nutzt \ref mini_vsnprintf() um die Ausgabe zu generieren.
     */
	void printf(const char *fmt, ...);

    /**
     * @brief Gibt einen String aus.
     * @param[in] s Auszugebender String.
     */
	void puts(const char* s);
///@}
#else
	inline void printf(const char *, ...) {}
	inline void puts(const char* ) {}
#endif


}
}
}





#endif /* SLAVE_FELDBUS_SLAVE_BASE_H_ */
