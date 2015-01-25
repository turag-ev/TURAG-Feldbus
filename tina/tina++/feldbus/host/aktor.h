/**
 *  @brief		TURAG feldbus actor base class
 *  @file		aktor.h
 *  @date		22.03.2013
 *  @author		Martin Oemus
 *
 */


#ifndef TURAGFELDBUSAKTOR_H_
#define TURAGFELDBUSAKTOR_H_

#include "device.h"
#include <tina++/tina.h>
#include <tina/feldbus/protocol/turag_feldbus_fuer_stellantriebe.h>
#include <tina/tina.h>

#if TURAG_FELDBUS_AKTOR_STRUCTURED_OUTPUT_AVAILABLE || defined(__DOXYGEN__)
# include <vector>
#endif

namespace TURAG {
	

namespace Feldbus {

/**
 * \brief Implementiert das %TURAG-Feldbus für Stellantriebe-Protokoll.
 * 
 * Von dieser Klasse können spezialisiertere Varianten abgeleitet werden,
 * die die grundlegende Funktionalität der getValue()- und setValue()-Funktionen
 * um benannte Funktionen erweitern, die jeweils einen Eintrag der Gerätetabelle
 * des speziellen Gerätes widerspiegeln. In einer solchen Klasse kann auch 
 * das Command_t-Array Platz finden, dass beim Aufruf von populateCommandSet()
 * benötigt wird.
 * 
 */
class Aktor : public TURAG::Feldbus::Device {
public:
	
	/**
	 * \brief Eintrag der Gerätewert-Tabelle.
	 */
	struct Command_t {
		/**
		 * 
		 */
		enum class WriteAccess : uint8_t {
			no_write = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_NO_WRITE_ACCESS,
			write = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS
		};
			
		/**
		 * 
		 */
		enum class CommandLength : uint8_t {
			none = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_NONE,
			length_char = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_CHAR,
			length_short = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT,
			length_long = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_LONG
		};

		float factor;
		union buffer_t {
			float floatValue;
			int32_t intValue;

			buffer_t() : floatValue(0) {}
		} buffer;
		WriteAccess writeAccess;
		CommandLength length;
		bool bufferValid;
		
		Command_t() : 
		factor(1), 
		buffer(),
		writeAccess(WriteAccess::no_write),
		length(CommandLength::length_char),
		bufferValid(false) {}
	};

#if TURAG_FELDBUS_AKTOR_STRUCTURED_OUTPUT_AVAILABLE || defined(__DOXYGEN__)
	/**
	 * TODO mit std::pair ersetzen
	 */
	struct StructuredDataPair_t {
		uint8_t key;
		float value;
		
		StructuredDataPair_t(void) :
			key(0), value(0.0f) { }
			
		StructuredDataPair_t(uint8_t key_, float value_) :
			key(key_), value(value_) {}
	};
#endif
    


#if TURAG_FELDBUS_AKTOR_STRUCTURED_OUTPUT_AVAILABLE || defined(__DOXYGEN__)
	/**
	 * \brief Konstruktor.
	 * \param[in] name_
	 * \param[in] address
	 * \param[in] type
	 * \param[in] addressLength
	 */
    Aktor(const char* name_, unsigned address, ChecksumType type = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE,
		const AddressLength addressLength = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_ADDRESS_LENGTH) :
                Device(name_, address, type, addressLength), commandSet(nullptr), commandSetLength(0), commandSetPopulated(false),
                structuredOutputTableLength(-1)  { }
#else
    Aktor(const char* name_, unsigned address, ChecksumType type = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE, 
		const AddressLength addressLength = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_ADDRESS_LENGTH) :
                Device(name_, address, type, addressLength), commandSet(nullptr), commandSetLength(0), commandSetPopulated(false) { }
#endif


	/**
	 * \brief Liest einen physiaklischen Gerätewert aus.
	 * \param[in] key Werte-Schlüssel.
	 * \param[out] value Puffer in dem der ausgelesene Wert gespeichert wird.
	 * \return Bei true enthält value den ausgelesenen Wert, bei false trat ein Fehler auf.
	 * \pre populateCommandSet() muss ausgeführt worden sein.
	 * \post Wenn false zurückgegeben wird, ist nicht garantiert, dass value nicht verändert wurde.
	 */
    bool getValue(uint8_t key, float* value);
	
	/**
	 * \brief Setzt einen physikalischen Wert.
	 * \param[in] key Werte-Schlüssel.
	 * \param[in] value Zu setzender Wert.
	 * \return Bei true wurde value korrekt zum Gerät übertragen, bei false trat ein Fehler auf.
	 * \pre populateCommandSet() muss ausgeführt worden sein.
	 * \post Wenn false zurückgegeben wird, kann es dennoch sein, dass der gewünschte Wert
	 * das Gerät erreicht hat und lediglich die Rückmeldung des Gerätes verloren ging. Eine
	 * eindeutige Aussage ist hier nicht möglich.
	 */
    bool setValue(uint8_t key, float value);
	
	/**
	 * \brief Liest einen Kontrollwert aus.
	 * \param[in] key Werte-Schlüssel.
	 * \param[out] value Puffer in dem der ausgelesene Wert gespeichert wird.
	 * \return Bei true enthält value den ausgelesenen Wert, bei false trat ein Fehler auf.
	 * \pre populateCommandSet() muss ausgeführt worden sein.
	 * \post Wenn false zurückgegeben wird, ist nicht garantiert, dass value nicht verändert wurde.
	 */
    bool getValue(uint8_t key, int32_t* value);
	
	/**
	 * \brief Setzt einen Kontrollwert.
	 * \param[in] key Werte-Schlüssel.
	 * \param[in] value Zu setzender Wert.
	 * \return Bei true wurde value korrekt zum Gerät übertragen, bei false trat ein Fehler auf.
	 * \pre populateCommandSet() muss ausgeführt worden sein.
	 * \post Wenn false zurückgegeben wird, kann es dennoch sein, dass der gewünschte Wert
	 * das Gerät erreicht hat und lediglich die Rückmeldung des Gerätes verloren ging. Eine
	 * eindeutige Aussage ist hier nicht möglich.
	 */
    bool setValue(uint8_t key, int32_t value);

	/**
	 * \brief Fragt die Anzahl der Einträge der Gerätewert-Tabelle ab.
	 * \return Anzahl der Einträge der Gerätewert-Tabelle, 0 wenn ein Fehler auftrat.
	 * 
	 * Diese Funktion puffert das Ergebnis und verursacht nur einmalig Buslast.
	 */
    unsigned int getCommandsetLength(void);
	
	/**
	 * \brief Initialisiert die interne Gerätewert-Tabelle.
	 * \param[out] commandSet_ Puffer der die Tabelle aufnimmt.
	 * \param[in] commandSetLength_ Anzahl der abzufragenden Einträge.
	 * \return True bei Erfolg, ansonsten false.
	 * 
	 * Diese Funktion füllt die interne Gerätewert-Tabelle mit Daten vom Gerät. Ihre
	 * erfolgreiche Ausführung ist die Voraussetzung für das Funktionieren der meisten
	 * anderen Funktionen.
	 */
    bool populateCommandSet(Command_t* commandSet_, unsigned int commandSetLength_);
	
	/**
	 * \brief Fragt die Länge der Bezeichnung eines Gerätewertes ab.
	 * \param[in] key Werte-Schlüssel.
	 * \return Länge der Bezeichnung, 0 falls ein Fehler auftrat.
	 * \pre populateCommandSet() muss ausgeführt worden sein.
	 * 
	 * Die zurückgegebene Länge enthält nicht das '\0'-zeichen.
	 */
    unsigned int getCommandNameLength(uint8_t key);

 	/**
	 * \brief Fragt die Bezeichnung eines Gerätewertes ab.
	 * \param[in] key Werte-Schlüssel.
	 * \param[out] name Puffer, in den der Name geschrieben wird.
	 * \return True bei Erfolg, anonsten false.
	 * \pre populateCommandSet() muss ausgeführt worden sein.
	 * 
	 * \warning getCommandNameLength() gibt die Länge der Bezeichnung 
	 * ohne Nullzeichen an, während diese Funktion einen null-terminierten
	 * String zurückgibt. Daher muss name ein Byte größer reserviert
	 * werden!
	 */
   bool getCommandName(uint8_t key, char* name);
   
#if TURAG_FELDBUS_AKTOR_STRUCTURED_OUTPUT_AVAILABLE || defined(__DOXYGEN__)
 	/**
	 * \brief Fragt die Tabellengröße für die zusammenhängende
	 * Datenausgabe ab.
	 * \return Tabellengröße, im Fehlerfall -1.
	 * 
	 * Diese Funktion puffert das Ergebnis und verursacht nur einmalig Buslast.
	 */
    int getStructuredOutputTableLength(void);
	
	/**
	 * \brief Überträgt eine Tabelle für die zusammenhängende Datenausgabe.
	 * \param[in] keys Vektor der die keys aller gewünschten Gerätewerte enthält.
	 * \return True bei Erfolg, ansonsten false.
	 * \pre populateCommandSet() muss ausgeführt worden sein.
	 */
    bool setStructuredOutputTable(const std::vector<uint8_t>& keys);
	
	/**
	 * \brief Fordert die Ausgabe der zusammenhängenden Daten an.
	 * \param[out] values Puffer, in den die Daten geschrieben werden.
	 * \return True bei Erfolg, anonsten false.
	 * \pre populateCommandSet() muss ausgeführt worden sein.
	 * \pre setStructuredOutputTable() muss ausgeführt worden sein.
	 */
    bool getStructuredOutput(std::vector<StructuredDataPair_t>* values);
#endif

protected:
    Command_t* commandSet;
    unsigned int commandSetLength;
    bool commandSetPopulated;

#if TURAG_FELDBUS_AKTOR_STRUCTURED_OUTPUT_AVAILABLE || defined(__DOXYGEN__)
    int structuredOutputTableLength;
    std::vector<uint8_t> structuredOutputTable;
#endif    

    
};

} // namespace Feldbus
} // namespace TURAG

#endif /* TURAGFELDBUSAKTOR_H_ */
