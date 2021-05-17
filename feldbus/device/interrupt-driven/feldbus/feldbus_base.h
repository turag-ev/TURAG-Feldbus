/**
 *  @brief		Implementiert slave-side %TURAG feldbus support
 *  @file		avr_feldbus_base.h
 *  @date		06.11.2013
 *  @author		Martin Oemus <martin@oemus.net>
 *  @see 		\ref feldbus-slave
 *  @ingroup feldbus-slave-base-avr
 */


/** 
 * @defgroup feldbus-slave-base-avr Basis-Implementierung (AVR)
 * @ingroup feldbus-slave
 * 
 * TURAG-Feldbus-Basisimplementierung für AVR-Controller ohne Betriebssystem.
 * Die Code-Struktur ist auf Interrupt-Betrieb optimiert.
 * 
 * 
 * \see \ref feldbus-slave
 * 
 * @section felbus-slave-struktur-basis Struktur der Basis-Implementierung
 * 
 * \image html feldbus-slave.png "Strukturschema der TURAG-Feldbus-Basisimplementierung (AVR)"
 * Die Basis-Implementierung sitzt zwischen Hardware und Firmware des Slave-Gerätes. 
 * Die Kommunikation mit der Hardware geschieht über ein Hardware-Interface, während für die
 * Firmware des Gerätes mit dem Protokoll-Interface eine abstrakte Schnittstelle bereitsteht,
 * um mit geringem Aufwand über den TURAG-Feldbus kommunizieren zu können.
 * 
 * Beide Interfaces sind jeweils teilweise implementiert - der andere Teil muss 
 * vom Entwickler des Slave-Gerätes bereitgestellt werden, so wie dies im Strukturschema 
 * dargestellt ist.
 * 
 * Anhand des Strukturschemas lässt sich die Arbeitsweise der Basis-Implementierung auch 
 * gut beschreiben. Der Entwickler des Gerätes ruft zu einem frühen Zeitpunkt 
 * turag_feldbus_device_init() auf. Das initialisiert interne Strukturen und ruft seinerseits
 * turag_feldbus_hardware_init() auf, welches die UART-Peripherie des Controllers initialisieren 
 * sollte. Danach müssen (falls das der Controller verlangt) die Interrupts global aktiviert 
 * werden. Ab diesem Zeitpunkt werden eintreffende Daten im Hardware-Interfaceteil der
 * Basis-Implementierung verarbeitet. Da die Paket-Erkennung im Kontext von Interrupts
 * ausgeführt wird, wird an dieser Stelle nur ein Flag gesetzt, sobald ein gültiges Paket
 * erkannt wurde. Die eigentliche Auswertung eines angekommenen Paketes wird erst gestartet,
 * wenn turag_feldbus_do_processing() aufgerufen wird. Diese Funktion deaktiviert 
 * temporär den Empfang neuer Daten, kopiert das empfangene Paket in einen Zwischenpuffer
 * und aktiviert die Kommunikation wieder. Dann wird, je nach Pakettyp, 
 * turag_feldbus_device_process_package() oder turag_feldbus_device_process_broadcast()
 * aufgerufen und das Paket wird so verarbeitet, wie das die Firmware des Slave-Gerätes
 * vorsieht.
 * 
 * Diese strikte Trennung der Kommunikation zwischen Empfang der Daten in Interrupts
 * und Verarbeitung im Kontext von main() hat zwei Vorteile:
 * - Die Verweildauer in Interrupts wird reduziert, was bei zeitkritischen 
 * Anwendungen wie Motorregelungen relevant sein kann. Der Entwickler muss sich 
 * dadurch weniger Gedanken über die Laufzeit der Paketverarbeitung machen, da 
 * diese lediglich ein Teil von main() ist.
 * - Da der komplette Datenaustausch zwischen Interrupts und main-Kontext 
 * von der Basis-Implementierung übernommen wird, muss der Entwickler keine 
 * weiteren Vorkehrungen treffen - es ist weder nötig Variablen als volatile
 * zu deklarieren, noch nicht-atomare Schreiboperationen vor Unterbrechung durch
 * Interrupts zu schützen (das gilt natürlich nur, solange das Slave-Gerät nicht
 * selbst noch weitere Interrupts benutzt).
 * 
 * **Wichtig**: damit sich das Slave-Gerät korrekt verhält, muss turag_feldbus_do_processing()
 * regelmäßig von der Hauptschleife aus aufgerufen werden.
 * 
 * Ein weiteres erwähnenswertes Feature ist der Uptime-Counter. Ein Timer des Slaves wird
 * so konfiguriert, dass er mit einer wählbaren Frequenz 
 * turag_feldbus_device_increase_uptime_counter() aufruft. Der Master kann die Laufzeit des
 * Slaves auslesen und so zum Beispiel Probleme in der Stromversorgung feststellen, was sich 
 * in Neustarts des Slaves und einem Zurücksetzen des Uptime-Counters äußern
 * würde. 
 * 
 * Wenn eine LED vorhanden ist, kann automatisch die Funktion
 * turag_feldbus_device_toggle_led() aufgerufen werden. Das Blinkmuster ist dabei
 * (soweit möglich) unabhängig von der Timer-Frequenz und fungiert außerdem als Indikator
 * für die Stabilität der Kommunikation: das Blinken der LED wird ausgesetzt, solange
 * im Gerät ein unverarbeitetes Paket vorliegt (zum Beispiel weil turag_feldbus_do_processing())
 * nicht oder nur zu selten aufgerufen wird. Zeigt die LED also ein unregelmäßiges Blinkmuster an
 * oder blinkt sie sogar gar nicht, gibt es Kommunikationsprobleme oder der Gerät
 * ist abgestürzt.
 * 
 * @section felbus-slave-struktur-anwendung Struktur der Anwendungsprotokoll-Implementierungen
 * Die Implementierungen der Anwendungsprotokolle führen im Allgemeinen zwei Änderungen
 * ein:
 * - nach turag_feldbus_device_init() muss zusätzlich eine spezialisierte Initialisierungsroutine
 * aufgerufen werden
 * - turag_feldbus_device_process_package() und turag_feldbus_device_process_broadcast()
 * werden vom Anwendungsprotokoll selbst implementiert, während die Firmware das 
 * Slave-Gerätes spezialisiertere Funktionen zu implementieren hat.
 * 
 * An der Notwendigkeit, turag_feldbus_do_processing() zyklisch aufzurufen,
 * ändert sich normalerweise nichts.
 * 
 * @section feldbus-slave-implementierungsanleitung HowTo: Softwaregerüst für ein neues Feldbusgerät erstellen
 * 1. Verzeichnisstruktur erstellen:
 * \image html feldbus-device-file-structure.png
 * 2. Als Basis für die Konfigurationsdatei sollte eine Kopie der Datei programmierung/tina/tina/feldbus/slave/avr_feldbus_config.h benutzt werden (umbenannt in %feldbus_config.h).
 *    Wichtig ist hierbei das korrekte includieren des passenden Protokoll-Headers. Basiert das Gerät auf einem Anwendungsprotokoll 
 *    (statt nur des Basis-Protokolls) werden evt. weitere Definitionen nötig, die in der Beispieldatei nicht enthalten sind. 
 * 3. Die Funktionen des Hardware-Interfaces müssen für die Plattform bereitgestellt werden (feldbus_hardware_driver.c). 
 * 4. Die Funktionen des Protokoll-Interfaces müssen gemäß dem Strukturschema bereitgestellt werden (main.c).
 * 5. Ein Makefile ist nötig, welches
 *  - alle Dateien des src-Ordners compiliert
 *  - die Feldbus-Slave-Quelldateien compiliert (es kann problemlos der komplette Ordner compiliert werden, da
 *    nur Code des gewählten Protokolls benutzt wird)
 *  - die Quelldateien des eingestellten Checksummen-Algorithmus compiliert
 *  - den tina- und den src-Ordner als Include-Pfad konfiguriert
 * 
 * Das Makefile kann üblicherweise aus alten Projekten übernommen und entsprechend angepasst werden.
 * 
 * **Wichtig:** zumindest die Datei, die die ISRs enthält (also feldbus_hardware_driver.c) muss mit -O3
 * compiliert werden! Nur dann beginnt der Compiler automatisch Aufrufe von kleinen Funktionen zu inlinen,
 * was im Falle der ISRs wichtig ist. Ansonsten leidet die Performance stark.
 * 
 * **Wichtig:** Die von der Basis-Implementerung bereitgestellten Hardware-Funktionen 
 * (wie z.B. turag_feldbus_device_receive_timeout_occured()) gehen davon aus,
 * dass ihre Ausführung nicht unterbrochen werden kann. Die Interrupts, die diese Funktionen
 * aufrefen, sollten also nicht von anderen Interrupts unterbrochen werden können.
 * 
 * @section feldbus-slave-beispiel-hardware-interface Beispiel-Implementierung des Hardware-Interfaces
 * Im folgenden wird ein Beispiel betrachtet, wie die Implementierung des Hardware-Interfaces für einen
 * ATmega88 aussehen kann.
 * 
 * Neben den AVR-Headern wird der entsprechende Protokoll-Header eingebunden.
 * \snippet feldbus_hardware_driver.c Includes
 * 
 * Als nächstes folgen einige Definitionen. Insbesondere die letzten beiden werden
 * üblicherweise über das Makefile global im Projekt bereitgestellt.
 * \snippet feldbus_hardware_driver.c Definitions
 * 
 * Danach ein paar hilfreiche Makros:
 * \snippet feldbus_hardware_driver.c Helper macros
 * 
 * Einen großen Teil der Implementierung nimmt die Bereitstellung der notwendigen
 * Hardware-Interface-Funktionen ein, darunter die Funktion \ref turag_feldbus_hardware_init() 
 * zum Initialisieren der Peripherie des Controllers.
 * \snippet feldbus_hardware_driver.c Required hardware functions
 * 
 * Zu guter letzt folgen die ISR-Handler, die die Ausführung lediglich an die entsprechenden
 * Funktionen der Basis-Implementierung weiterreichen:
 * \snippet feldbus_hardware_driver.c Interrupts
 * 
 * @section feldbus-slave-korrekt-includen Korrektes includieren der Header
 * 
 * Die folgenden Graphen zeigen an, welche Header sinnvollerweise von welchen Dateien includiert
 * werden sollten. Blau umrandete Dateien sind Teil von TinA, schwarz umrandete solche,
 * die der Entwickler des Slave-Gerätes bereitstellt.
 * 
 * Hervorzuheben sind folgende Punkte:
 * - die Header von Basis- und Anwendungsprotokoll sind auf die Existenz von 
 * "feldbus_config.h" angewiesen und die Tatsache, dass dieser Header auch 
 * den richtigen Protokollheader includiert.
 * - setzt das Slave-Gerät direkt auf dem Basis-Protokoll auf, so wird stets nur "avr_feldbus_base.h" 
 * includiert, anonsten zusätzlich der entsprechende Header des Anwendungsprotokolls.
 * 
 * \dot
 * digraph G {
 * 
 * node [shape="box", fontname="Helvetica", fontsize="12"];
 * 
 * subgraph cluster0 {
 *      a1 -> a2 -> a3 -> a4 -> a5;
 *      a3 -> a6;
 * 
 *      label = "Basis-Implementierung";
 *      fontname="Helvetica";
 *      fontsize="14";
 *      color="white";
 * }
 * 
 * subgraph cluster1 {
 *      b1 -> b2 -> b4;
 *      b1 -> b3 -> b4 -> b5 -> b6 -> b7; 
 *      b4 -> b8;
 * 
 *      label ="Stellantriebe-Anwendungsprotokoll"
 *      fontname="Helvetica";
 *      fontsize="14";
 *      color="white";
 * }
 * 
 * a1 [label="main.c; andere Dateien"];
 * a2 [label="avr_feldbus_base.h", color="blue"];
 * a3 [label="feldbus_config_check.h", color="blue"];
 * a4 [label="feldbus_config.h"];
 * a5 [label="turag_feldbus_bus_protokoll.h", color="blue"];
 * a6 [label="avr_feldbus_config_check.h", color="blue"];
 * 
 * b1 [label="main.c; andere Dateien"];
 * b2 [label="feldbus_stellantriebe.h", color="blue"];
 * b3 [label="avr_feldbus_base.h", color="blue"];
 * b4 [label="feldbus_config_check.h", color="blue"];
 * b5 [label="feldbus_config.h"];
 * b6 [label="turag_feldbus_fuer_stellantriebe.h", color="blue"];
 * b7 [label="turag_feldbus_bus_protokoll.h", color="blue"];
 * b8 [label="avr_feldbus_config_check.h", color="blue"];
 * 
 * 
 * }
 * \enddot
 * 
 * @section feldbus-slave-neues-anwendungsprotokoll Implementieren eines neuen Anwendungsprotokolls
 * Ein neues Anwendungsprotokoll ist normalerweise dann von Nöten, wenn eine 
 * neue Gerätegruppe entwickelt werden soll. Die Entwicklung eines neuen
 * Anwendungsprotokolls umfasst mehrere Schritte:
 * 
 * 1. Im [Wiki](https://intern.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus#anwendungs-protokolle) 
 * eine Seite mit den Spezifikationsdetails des Anwendungsprotokolls anlegen und auf der 
 * Hauptseite verlinken
 * 2. In turag_feldbus_bus_protokoll.h wird die ID des neuen Protokolls eingetragen
 * 3. Für das neue Protokoll wird ein eigener Protokollheader angelegt, der die in der Spezifikation
 * definierten Werte und IDs enthält
 * 4. Eine Slave-Implementierung des Anwendungsprotokolls wird angelegt, die üblicherweise
 * aus einem Header und einer Quelldatei besteht.
 * 5. Hostklassen zur Kommunikation mit den neuen Geräten werden angelegt
 * 6. In der TURAG-Console wird Support für den Gerätetyp eingebaut, was gleich 
 * zur Verifizierung der Host-Klassen benutzt werden kann.
 * 
 * Wenn möglich, sollte \ref _Static_assert benutzt werden, um sicherzustellen
 * dass mit \ref TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE ein ausreichend großer
 * Transmit-Buffer konfiguriert ist, da es ansonsten zu Pufferüberläufen kommen kann.
 * 
 * 
 * @section feldbus-slave-base-config Konfiguration
 * 
 * Siehe avr_feldbus_config.h.
 * 
 * avr_feldbus_config.h ist eine Beispielkonfigurationsdatei, die nicht direkt includiert werden darf.
 * Eine Kopie dieser sollte unter dem Namen "feldbus_config.h" im Projektverzeichnis abgelegt werden.
 *
 * 
 */

#ifndef TINA_FELDBUS_SLAVE_FELDBUS_H_
#define TINA_FELDBUS_SLAVE_FELDBUS_H_


#include <feldbus/util/xor_checksum.h>
#include <feldbus/util/crc_checksum.h>

#include "feldbus_config_check.h"



#ifdef __cplusplus
# include <cstdlib>
# include <cstddef>
# include <cstring>

extern "C" {
#else
# include <stdlib.h>
# include <string.h>
# include <stdbool.h>
#endif




/** @name Benötigte Hardware-Funktionen
 *  Hardware-Interface-Funktionen die von der Basis-Implementierung benutzt werden
 * und die für die Zielplattform bereitgestellt werden müssen.
 */
///@{

/**
 * This function should implement all required hardware initializations (e.g. UART peripheral).
 */
extern void turag_feldbus_hardware_init(void);

/**
 * This function should implement all logic that is required to disable the rs485 line driver thus releasing the bus.
 */
extern void turag_feldbus_device_rts_off(void);

/**
 * This function should implement all logic that is required to enable the rs485 line driver thus claiming 
 * the bus for active driving.
 */
extern void turag_feldbus_device_rts_on(void);

/**
 * This function should implement all logic that is required to 
 * activate the data register empty interrupt of the UART peripheral.
 */
extern void turag_feldbus_device_activate_dre_interrupt(void);

/**
 * This function should implement all logic that is required to 
 * deactivate the data register empty interrupt of the UART peripheral.
 */
extern void turag_feldbus_device_deactivate_dre_interrupt(void);

/**
 * This function should implement all logic that is required to 
 * activate the receive interrupt of the UART peripheral.
 */
extern void turag_feldbus_device_activate_rx_interrupt(void);

/**
 * This function should implement all logic that is required to 
 * deactivate the receive interrupt of the UART peripheral.
 */
extern void turag_feldbus_device_deactivate_rx_interrupt(void);

/**
 * This function should implement all logic that is required to 
 * activate the transmission complete interrupt of the UART peripheral.
 */
extern void turag_feldbus_device_activate_tx_interrupt(void);

/**
 * This function should implement all logic that is required to 
 * deactivate the transmission complete interrupt of the UART peripheral.
 */
extern void turag_feldbus_device_deactivate_tx_interrupt(void);

/**
 * This function should start some component that is able to call turag_feldbus_device_receive_timeout_occured.
 * The timeout is specified in the protocol definition. Whenever the function is called
 * it is required that the timer starts counting from the start regardless of its previous state.
 * 
 * After being enabled it should trigger the call of turag_feldbus_device_receive_timeout_occured only once
 * and deactivate itself afterwards.
 */
extern void turag_feldbus_device_start_receive_timeout(void);

/**
 * This function should implement some functionality that ensures code
 * being executed without being interrupted.
 * 
 * Usually this means to deactivate the global interrupt flag.
 * 
 * This function is always followed by a call to turag_feldbus_device_end_interrupt_protect().
 */
extern void turag_feldbus_device_begin_interrupt_protect(void);

/**
 * This function follows on a call to turag_feldbus_device_begin_interrupt_protect().
 * 
 * It should usually implement the enabling of the global interrupt flag.
 */
extern void turag_feldbus_device_end_interrupt_protect(void);

/**
 * Diese Funktion wird in einem
 * einem charakteristischen Frequenzmuster aufgerufen, welches
 * vom Wert von \ref TURAG_FELDBUS_DEVICE_CONFIG_UPTIME_FREQUENCY
 * abhängt.
 */
extern void turag_feldbus_device_toggle_led(void);

/**
 * Transmits on byte over the bus.
 * @param byte Data to send
 */
extern void turag_feldbus_device_transmit_byte(uint8_t byte);

/**
 * Enables the switchable part of the bus.
 */
extern void turag_feldbus_device_enable_bus_neighbours();

/**
 * Disables the switchable part of the bus.
 */
extern void turag_feldbus_device_disable_bus_neighbours();

///@}




/** @name Zu benutzende Hardware-Funktionen
 *  Funktionen, die vom Hardwaretreiber des Gerätes
 * aufgerufen werden müssen.
 */
///@{
/**
 * Receive-Complete-Interrupt Interface.
 *
 * Call this function from the Receive-Complete-Interrupt on your system.
 * @param byte Received byte
 */
static inline void turag_feldbus_device_byte_received(uint8_t byte);

/**
 * Data-Register-Empty-Interrupt-Interface.
 *
 * Call this function from the Data-Register-Empty-Interrupt on your system.
 */
static inline void turag_feldbus_device_ready_to_transmit(void);

/**
 * Transmit-Complete-Interrupt-Interface.
 *
 * Call this function from the Transmit-Complete-Interrupt on your system.
 */
static inline void turag_feldbus_device_transmission_complete(void);

/** 
 * Timeout occured.
 *
 * Call this function when the receive timeout has occured.
 *
 * If a timer is used to provide this functionality don't forget to deactivate it
 * before calling this function to avoid additional interrupts because of an overflowing timer.
 */
static inline void turag_feldbus_device_receive_timeout_occured(void);

/**
 * Inkrementiert den Uptime-Counter.
 * 
 * Diese Funktion muss periodisch mit einer konstanten Frequenz aufgerufen werden.
 * Zusätzlich muss \ref TURAG_FELDBUS_DEVICE_CONFIG_UPTIME_FREQUENCY mit der gleichen
 * Frequenz definiert werden.
 * 
 */
#if defined(__DOXYGEN__)
static inline void turag_feldbus_device_increase_uptime_counter(void);
#endif

///@}


/// \brief Rückgabewert für turag_feldbus_device_process_package(), mit dem angezeigt wird,
/// dass kein Antwortpaket zurückgesendet werden soll.
#if defined(__DOXYGEN__)
# define TURAG_FELDBUS_NO_ANSWER		<configuration-dependend>
#endif

/// \brief Typ, der für Offsets und Längen benutzt wird.
/// Größe des tatsächlichen Integer-Typs hängt von der Größe
/// des Puffers ab (\ref TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE)
#if defined(__DOXYGEN__)
typedef int FeldbusSize_t;
#endif




/** @name Benötigte Protokoll-Interface-Funktionen
 *  Protokoll-Interface-Funktionen die von der Basis-Implementierung
 * aufgerufen werden und von der Firmware des Gerätes bereitgestellt
 * werden müssen.
 */
///@{

/**
 * @param[in] message			Buffer holding the received data
 * @param[in] message_length	Size of received data
 * @param[out] response			Buffer that can be filled with the response data.
 * @return						Number of bytes put in the response buffer. You must not return 
 * more than \ref TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE - \ref TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH bytes.
 * 
 * This function gets called when the device received a package.
 * The response should be made as soon as possible. 
 * 
 * The length of the response package is controlled by the return
 * value. It is possible not to reply to a package at all by returning \ref TURAG_FELDBUS_NO_ANSWER.
 * If you return 0, an empty response package consisting only of address and
 * checksum will be generated.
 * 
 * The data held in the message buffer contain the received package
 * stripped of address and checksum. message_length is guaranteed to be >= 1 as a consequence 
 * ping-requests (empty package) being handled by this implementation.
 * 
 * \note Diese Funktion wird stets im main-Kontext aufgerufen.
 *
 * @warning Keinesfalls dürfen in response mehr Daten geschrieben werden als 
 * \ref TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE - \ref TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH bytes.
 */
typedef FeldbusSize_t (*TuragFeldbusPacketProcessor)(const uint8_t* message, FeldbusSize_t message_length, uint8_t* response);


/**
 * @param[in] message			Puffer mit den empfangenen Daten-
 * @param[in] message_length	Länge des Puffers.
 * @param[in] protocol_id		Protokoll-ID, an die der Broadcast gerichtet ist.
 *
 * Diese Funktion wird aufgerufen, wenn das Gerät ein Broadcast-Paket empfängt. 
 * Dies ist nur dann der Fall, wenn die Protokoll-ID des Broadcasts der des Gerätes
 * entspricht oder 0 ist.
 * 
 * Da Antworten auf Braodcasts nicht zulässig sind, besitzt diese Funktion weder
 * einen Rückgabetyp, noch output-Argumente.
 *
 * \note Diese Funktion wird stets im main-Kontext aufgerufen.
 */
typedef void (*TuragFeldbusBroadcastProcessor)(const uint8_t* message, FeldbusSize_t message_length, uint8_t protocol_id);
///@}





/** @name Zu benutzende Protokoll-Interface-Funktionen
 *  Funktionen, die von der Firmware des Gerätes aufgerufen
 * werden müssen.
 */
///@{

/**Initialize TURAG Feldbus support
 *
 * Call this function to set up the TURAG Feldbus module.
 *
 * If your device does not have a bootloader, you should call this function
 * only after making sure that no device is being programmed.
 */
void turag_feldbus_device_init(
		FeldbusAddress_t bus_address, uint32_t uuid,
		const char* name, const char* version_info,
		uint8_t device_protocol, uint8_t device_type,
		TuragFeldbusPacketProcessor packetProcessor,
		TuragFeldbusBroadcastProcessor broadcastProcessor);


uint32_t turag_feldbus_device_hash_uuid(const uint8_t* key, size_t length);

/**
 * This function needs to be called continuously from the device's
 * main loop. It handles all advanced communication logic.
 * 
 */
void turag_feldbus_do_processing(void);


///@}



// debugging functions
#if TURAG_FELDBUS_DEVICE_CONFIG_DEBUG_ENABLED || defined(__DOXYGEN__)
/** @name Debug-Functions
 * Diese Funktionen können verwendet werden, um Debugausgaben in Form
 * konventioneller Zeichenketten auf dem Bus auszugeben.
 * 
 * Soll eine der Debug-Funktionen von turag_feldbus_device_process_package()
 * aus aufgerufen werden, kann das Makro TURAG_FELDBUS_DEBUG_SAFE() benutzt werden,
 * welches als Argument den kompletten Funktionsaufruf entgegennimmt.
 * 
 * \pre Nur verfügbar, wenn \ref TURAG_FELDBUS_DEVICE_CONFIG_DEBUG_ENABLED auf 1 definiert ist.
 * 
 */
///@{
	/// print some text
	void print_text(const char*);	
	
	/// print signed char in hexadecimal format
	void print_char(uint8_t);	
	
	/// print unsigned short value in hexadecimal format
	void print_short(uint16_t);
	
	/// print signed short value in hexadecimal format
	void print_sshort(int16_t x);
	
	/// print unsigned long value in hexadecimal format
	void print_long(uint32_t x);
	
	/// print signed short value in decimal format
	void print_short_d(int16_t x);
	
	/// print signed long value in hexadecimal format
	void print_slong(int32_t);
	
	/// print signed short value in hexadecimal format without trailuing newline characters
	void print_sshort_nn(int16_t x);
	
	/// print unsigned short value in hexadecimal format without trailing newline characters
	void print_short_nn(uint16_t x);
	
	/// Makro, von welchem eine Debug-Funktion umschlossen werden muss, wenn es von 
	/// turag_feldbus_device_process_package() aus aufgerufen wird.
	#define TURAG_FELDBUS_DEBUG_SAFE(function) function; while (turag_feldbus_device.transmission_active)
///@}
#else
# 	define print_text(x)
# 	define print_char(x)
# 	define print_short(x)
# 	define print_sshort(x)
# 	define print_long(x)
# 	define print_short_d(x)
# 	define print_slong(x)
# 	define print_sshort_nn(x)
# 	define print_short_nn(x)
#endif
	
#ifdef __cplusplus
}
#endif
	
// hide some uninteresting stuff from documentation
#if (!defined(__DOXYGEN__))
		
typedef struct {
	// holds the number of bytes in txbuf
	FeldbusSize_t transmitLength;
	// offset in txbuf
	FeldbusSize_t txOffset;
	// offset in rxBuf
	FeldbusSize_t rxOffset;
	// if not 0, then there is a package waiting for processsing
	volatile FeldbusSize_t rx_length;
	// overflow detected
	bool overflow;
	// package loss detected and counter must be increased
	bool package_lost_flag;
	// overflow detected and counter must be increased
	bool buffer_overflow_flag;
#if TURAG_FELDBUS_DEVICE_CONFIG_DEBUG_ENABLED
	volatile bool transmission_active;
#endif
	volatile bool toggleLedBlocked;
	TuragFeldbusPacketProcessor packet_processor;
	TuragFeldbusBroadcastProcessor broadcast_processor;
	// bus address of the device
	FeldbusAddress_t my_address;
	const char* name;
	size_t name_length;
	const char* versioninfo;
	size_t version_info_length;
	uint8_t device_protocol;
	uint8_t device_type;
	uint32_t packagecount_correct;
	uint32_t packagecount_buffer_overflow;
	uint32_t packagecount_lost;
	uint32_t packagecount_chksum_mismatch;
	uint32_t uptime_counter;
	// uuid of the device
	uint8_t uuid[4] __attribute__((aligned(4)));
	uint8_t txbuf[TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE + TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH + TURAG_FELDBUS_DEVICE_CRC_SIZE] __attribute__((aligned(4)));
	uint8_t rxbuf[TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE + TURAG_FELDBUS_DEVICE_CONFIG_ADDRESS_LENGTH + TURAG_FELDBUS_DEVICE_CRC_SIZE] __attribute__((aligned(4)));
} turag_feldbus_device_t;


extern turag_feldbus_device_t turag_feldbus_device;




#endif // (!defined(__DOXYGEN__))

	
#ifdef __cplusplus
extern "C" {
#endif

static inline void turag_feldbus_device_byte_received(uint8_t data) {
	// if at this point rx_length is not 0, obviously the last 
	// received package was not processed yet. This package
	// will be overwritten now and is lost.
	// Once rx_length is cleared, turag_feldbus_do_processing() will no longer
	// try to copy packages from the in-buffer.
	if (turag_feldbus_device.rx_length) {
		turag_feldbus_device.rx_length = 0;
		turag_feldbus_device.package_lost_flag = true;
	}

	// We need to check for overflow before actually storing the received
	// byte. Otherwise we always get an overflow when the last byte in the
	// buffer gets filled.
	if (turag_feldbus_device.rxOffset >= TURAG_FELDBUS_DEVICE_CONFIG_BUFFER_SIZE) {
		turag_feldbus_device.rxOffset = 0;
		
		// We have a buffer overflow. If this happens for the 
		// first time for this package, we check the address.
		// If the package was for us, we increase the counter for 
		// package overflow.
		if (!turag_feldbus_device.overflow) {
			if (*((FeldbusAddress_t*)turag_feldbus_device.rxbuf) == turag_feldbus_device.my_address || *((FeldbusAddress_t*)turag_feldbus_device.rxbuf) == TURAG_FELDBUS_BROADCAST_ADDR)
			{
				turag_feldbus_device.buffer_overflow_flag = true;
			}
			turag_feldbus_device.overflow = true;
		}
	}

	// accessing the buffer as an array is more effective than using a pointer
	// and increasing it.
	turag_feldbus_device.rxbuf[turag_feldbus_device.rxOffset] = data;
	++turag_feldbus_device.rxOffset;

	// activate timer to recognize end of command
	turag_feldbus_device_start_receive_timeout();
}

static inline void turag_feldbus_device_ready_to_transmit() {
	// accessing the buffer as an array is more effective than using a pointer
	// and increasing it.
	turag_feldbus_device_transmit_byte(turag_feldbus_device.txbuf[turag_feldbus_device.txOffset]);
	++turag_feldbus_device.txOffset;

	if (turag_feldbus_device.txOffset == turag_feldbus_device.transmitLength) {
		turag_feldbus_device_deactivate_dre_interrupt();
		turag_feldbus_device_activate_tx_interrupt();
	}
}

static inline void turag_feldbus_device_transmission_complete() {
	// turn off transmitter and release bus, turn on receiver
	turag_feldbus_device_rts_off();
	turag_feldbus_device_deactivate_tx_interrupt();
	turag_feldbus_device_activate_rx_interrupt();

#if TURAG_FELDBUS_DEVICE_CONFIG_DEBUG_ENABLED
	turag_feldbus_device.transmission_active = false;
#endif
}


static inline void turag_feldbus_device_receive_timeout_occured() {
	if (turag_feldbus_device.package_lost_flag) {
		++turag_feldbus_device.packagecount_lost;
		turag_feldbus_device.package_lost_flag = false;
	}
	
	if (turag_feldbus_device.buffer_overflow_flag) {
		++turag_feldbus_device.packagecount_buffer_overflow;
		turag_feldbus_device.buffer_overflow_flag = false;
	}

	if ((*((FeldbusAddress_t*)turag_feldbus_device.rxbuf) == turag_feldbus_device.my_address || *((FeldbusAddress_t*)turag_feldbus_device.rxbuf) == TURAG_FELDBUS_BROADCAST_ADDR) &&
			!turag_feldbus_device.overflow &&
			turag_feldbus_device.rxOffset > 1)
	{
		// package ok -> signal main loop that we have package ready
		turag_feldbus_device.rx_length = turag_feldbus_device.rxOffset;
		
		// we stop the led blinking until the user program starts the package
		// processing
		turag_feldbus_device.toggleLedBlocked = true;
	}

	// reset rxOffset and overflow-flag to ensure correct
	// receiving of future packages
	turag_feldbus_device.rxOffset = 0;
	turag_feldbus_device.overflow = 0;
}

static inline void turag_feldbus_device_increase_uptime_counter(void) {
	++turag_feldbus_device.uptime_counter;

	// we only toggle the led if there is no package
	// waiting to be processsed.
	// We use this as an indicator for the user whether
	// there is something wrong with the communication.
	if (!turag_feldbus_device.toggleLedBlocked) {
#  if TURAG_FELDBUS_DEVICE_CONFIG_UPTIME_FREQUENCY >= 12
#   define COUNT_MAX (TURAG_FELDBUS_DEVICE_CONFIG_UPTIME_FREQUENCY / 12 - 1)
#    if COUNT_MAX > 255
		static uint16_t count = 0;
#    else
		static uint8_t count = 0;
#    endif
		static uint8_t subcount = 0;
	
		++count;
		if (count > COUNT_MAX) {
			if (subcount == 0) {
				turag_feldbus_device_toggle_led();
			} else if (subcount == 1) {
				turag_feldbus_device_toggle_led();
			}
			subcount = (subcount+1) & 7;
			count = 0;
		}
#  elif TURAG_FELDBUS_DEVICE_CONFIG_UPTIME_FREQUENCY >= 2
		static uint8_t count = 0;
		
		++count;
		if (count > (TURAG_FELDBUS_DEVICE_CONFIG_UPTIME_FREQUENCY / 2 - 1)) {
			turag_feldbus_device_toggle_led();
			count = 0;
		}
#  else
		turag_feldbus_device_toggle_led();
#  endif
	}
}



#ifdef __cplusplus
}
#endif


#endif /* TINA_FELDBUS_SLAVE_FELDBUS_H_ */
