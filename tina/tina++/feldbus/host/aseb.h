/**
 *  @brief		TURAG feldbus aseb base class
 *  @file		aseb.h
 *  @date		11.01.2014
 *  @author		Martin Oemus
 *
 */


#ifndef TURAGFELDBUSASEB_H_
#define TURAGFELDBUSASEB_H_

#include "device.h"
#include <tina++/tina.h>
#include <tina/feldbus/protocol/turag_feldbus_fuer_aseb.h>


namespace TURAG {
namespace Feldbus {

    
/**
 * \brief Implementiert das %TURAG-Feldbus für ASEBs-Protokoll.
 * 
 * Vor Verwendung einer Instanz dieser Klasse muss initialize() 
 * aufgerufen werden.
 * 
 * Diese Klasse verwaltet die notwendigen Puffer nicht selbst. Diese
 * müssen bei der Initialisierung mit den richtigen Größen übergeben
 * werden.
 * 
 * \note Diese Klasse ist nicht zur direkten Verwendung vorgesehen.
 * Stattdessen sollte \ref AsebTemplate benutzt werden.
 * 
 */
class Aseb : public TURAG::Feldbus::Device {
public:
		/**
	 * \brief Repräsentiert einen analogen Eingang.
	 */
	struct Analog_t {
		float factor;
        int value;

        Analog_t() : factor(0), value(0) {}
	};
	
	/**
	 * \brief Repräsentiert einen PWM-Ausgang.
	 */
	struct Pwm_t {
        uint16_t max_value;
        uint32_t frequency; /// [Hz]
        float speed; /// [%/s]
        float value; /// [%]

        Pwm_t() : max_value(0), value(0) {}
	};
	
	
	/**
	 * \brief Konstruktor
	 * \param[in] name
	 * \param[in] address
	 * \param[in] feldbus
	 * \param[in] type
	 * \param[in] addressLength
	 */
	Aseb(const char* name, unsigned int address, FeldbusAbstraction& feldbus, ChecksumType type = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE,
		const AddressLength addressLength = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_ADDRESS_LENGTH) :
				Device(name, address, feldbus, type, addressLength),
				analogInputs_(nullptr), pwmOutputs_(nullptr), syncBuffer_(nullptr),
				analogInputSize_(-1), pwmOutputSize_(-1), digitalInputSize_(-1),
				digitalOutputSize_(-1), syncSize_(0),
				digitalInputs_(0), digitalOutputs_(0),
				isSynced_(false)  { }
                
    /**
	 * \brief Initialisiert das Gerät.
	 * \param[in] sync_buffer Puffer zur Synchronisation der Eingänge.
	 * \param[in] sync_buffer_size Größe des Puffers. Muss dem mit 
	 * getSyncSize() ermittelten Wert entsprechen.
	 * \param[in] analogInputs Puffer zum Speichern der analogen Eingänge.
	 * \param[in] analogInputSize Größe des Puffers. Muss dem mit getAnalogInputSize()
	 * ermittelten Wert entsprechen.
	 * \param[in] pwmOutputs Puffer zum Speichern der PWM duty cycles.
	 * \param[in] pwmOutputSize Größe des Puffers. Muss dem mit getPwmOutputSize()
	 * ermittelten Wert entsprechen.
	 * \return True bei Erfolg, ansonsten false.
	 * 
	 * Die Puffer sind für das Funktionieren der Klasse nötig. Die übergebenen Puffer 
	 * gehören dann zum Objekt und sollten nicht länger extern manipuliert werden.
	 */
    bool initialize(uint8_t* sync_buffer, unsigned sync_buffer_size,
                    Analog_t* analogInputs, unsigned analogInputSize,
                    Pwm_t* pwmOutputs, unsigned pwmOutputSize );

	/**
	 * @brief Gibt zurück, ob das Gerät erfolgreich initialisiert wurde.
	 * @return True, wenn initialisiert, sonst false.
	 */
	bool isInitialized(void) { return syncSize_ != 0; }

	/**
	 * \brief Synchronisiert den Eingangspuffer mit den Eingängen des Gerätes.
	 * \return True bei Erfolg, ansonsten false.
	 * 
	 * Diese Funktion muss erfolgreich ausgeführt werden, um Eingänge zu lesen.
	 * \pre initialize() muss aufgerufen worden sein.
	 */
    bool sync(void);

	/**
	 * \brief Gibt zurück, ob die letzte Synchronisation erfolgreich verlief.
	 * \return True wenn die letzte Synchronisation erfolgreich war, ansonten false.
	 * \see sync()
	 */
    bool isSynced(void) { return isSynced_; }

	/**
	 * \brief Gibt den Wert eines digitalen Eingangs zurück.
	 * \param[in] key Key das auszugebenden Channels (0-15).
	 * \return Der Zustand des digitalen Eingangs. Wenn initialize() nicht ausgeführt 
	 * wurde oder key außerhalb des gültigen 
	 * Wertebreichs liegt, wird false zurückgegeben.
	 * 
	 * Diese Funktion verursacht keine Buslast.
	 * \pre initialize() muss aufgerufen worden sein.
	 * \pre sync() muss ausgeführt werden, damit der Puffer einen aktuellen
	 * Wert enthält.
	 */
	bool getDigitalInput(unsigned key);
	
	/**
	 * \brief Gibt den Wert eines analogen Eingangs zurück.
	 * \param[in] key Key das auszugebenden Channels (0-15).
	 * \return Der Wert des analogen Eingangs. Physikalische Dimension und
	 * Wertebreich hängen vom jeweiligen Gerät ab. Wenn initialize() nicht ausgeführt 
	 * wurde, oder key außerhalb des gültigen
	 * Wertebreichs liegt, wird 0.0f zurückgegeben.
	 * 
	 * Diese Funktion verursacht keine Buslast.
	 * \pre initialize() muss aufgerufen worden sein.
	 * \pre sync() muss ausgeführt werden, damit der Puffer einen aktuellen
	 * Wert enthält.
	 */
    float getAnalogInput(unsigned key);
	
	/**
	 * \brief Gibt den Zustand eines digitalen Ausgangs zurück.
	 * \param[in] key Key das auszugebenden Channels (0-15).
	 * \return Der Zustand des digitalen Ausgangs. Wenn initialize() nicht ausgeführt 
	 * wurde oder key außerhalb des gültigen 
	 * Wertebreichs liegt, wird false zurückgegeben.
	 * 
	 * Diese Funktion verursacht keine Buslast.
	 * \pre initialize() muss aufgerufen worden sein.
	 */
	bool getDigitalOutput(unsigned key);
	
	/**
	 * \brief Setzt den Zustand eines digitalen Ausgangs.
	 * \param[in] key Key das auszugebenden Channels (0-15).
	 * \param[in] value Zustand, den der Ausgang annehmen soll.
	 * \return True bei Erfolg, ansonsten false.
	 * 
	 * \pre initialize() muss aufgerufen worden sein.
	 */
    bool setDigitalOutput(unsigned key, bool value);
	
    /**
     * \brief Gibt die Stellgeschwindigkeit des eines PWM-Channels zurück.
     * \param[in] key Key des auszugebenden Channels (0-15).
     * \return Stellgeschwindigkeit des Channels in %/s, unendlich wenn die Stellgeschwindigkeit nicht vom ASEB begrenzt wird
     * Wenn initialize() nicht ausgeführt
     * wurde oder key außerhalb des gültigen
     * Wertebreichs liegt, wird 0.0f zurückgegeben.
     *
     * Diese Funktion verursacht keine Buslast.
     * \pre initialize() muss aufgerufen worden sein.
     */
    float getPwmSpeed(unsigned key);

    /**
     * \brief Setzt die Stellgeschwindigkeit eines PWM-Channels.
     * \param[in] key Key das zu setzenden Channels (0-15).
     * \param[in] speed Stellgeschwindigkeit des Channels, angegeben in %/s
     * \return True bei Erfolg, ansonsten false.
     * \pre initialize() muss aufgerufen worden sein.
     */
    bool setPwmSpeed(unsigned key, float speed);

	/**
	 * \brief Gibt den duty cycle eines PWM-Channels zurück.
     * \param[in] key Key des auszugebenden Channels (0-15).
	 * \return Duty cycle des Channels; mögliche Werte: 0 - 100 %.  
	 * Wenn initialize() nicht ausgeführt 
	 * wurde oder key außerhalb des gültigen 
	 * Wertebreichs liegt, wird 0.0f zurückgegeben.
	 * 
	 * Diese Funktion verursacht keine Buslast.
	 * \pre initialize() muss aufgerufen worden sein.
	 */
    float getPwmOutput(unsigned key);
	
	/**
	 * \brief Setzt den duty cycle eines PWM-Channels.
	 * \param[in] key Key das zu setzenden Channels (0-15).
	 * \param[in] duty_cycle Duty-Cycle des Channels, angegeben in 0 - 100 %.
	 * \return True bei Erfolg, ansonsten false.
	 * \pre initialize() muss aufgerufen worden sein.
	 */
	bool setPwmOutput(unsigned key, float duty_cycle);

 	/**
	 * \brief Fragt die Bezeichnung eines Channels ab.
	 * \param[in] raw_key Key des Channels (1-64)
	 * \param[out] name Puffer, in den der Name geschrieben wird.
	 * \return True bei Erfolg, anonsten false.
	 * \pre initialize() muss aufgerufen worden sein.
	 * 
	 * \warning getCommandNameLength() gibt die Länge der Bezeichnung 
	 * ohne Nullzeichen an, während diese Funktion einen null-terminierten
	 * String zurückgibt. Daher muss name ein Byte größer reserviert
	 * werden!
	 * \warning Im Gegensatz zu den meisten anderen Funktionen wird hier
	 * ein key benötigt, der für alle Channel-Typen eindeutig ist. Um diesen 
	 * zu berechnen muss auf den normalen Channel-Index (0-15) das Offset 
	 * der Channel-Art addiert werden:
	 * - \ref TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_INPUT
	 * - \ref TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_OUTPUT
	 * - \ref TURAG_FELDBUS_ASEB_INDEX_START_ANALOG_INPUT
	 * - \ref TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT
	 * \see turag_feldbus_fuer_aseb.h
	 */
    bool getCommandName(unsigned raw_key, char* name);
 
	/**
	 * \brief Fragt die Länge der Bezeichnung eines Channels ab.
	 * \param[in] raw_key Key des Channels (1-64)
	 * \return Länge der Bezeichnung, 0 falls ein Fehler auftrat.
	 * \pre initialize() muss aufgerufen worden sein.
	 * 
	 * Die zurückgegebene Länge enthält nicht das '\0'-zeichen.
	 * \warning Im Gegensatz zu den meisten anderen Funktionen wird hier
	 * ein key benötigt, der für alle Channel-Typen eindeutig ist. Um diesen 
	 * zu berechnen muss auf den normalen Channel-Index (0-15) das Offset 
	 * der Channel-Art addiert werden:
	 * - \ref TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_INPUT
	 * - \ref TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_OUTPUT
	 * - \ref TURAG_FELDBUS_ASEB_INDEX_START_ANALOG_INPUT
	 * - \ref TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT
	 * \see turag_feldbus_fuer_aseb.h
	 */
	unsigned int getCommandNameLength(unsigned raw_key);
	
    /**
     * \brief Gibt die Frequenz eines PWM-Channels zurück.
     * \param[in] key Key das auszugebenden Channels (0-15).
     * \return Frequenz des Channels in Hz
     * Wenn initialize() nicht ausgeführt
     * wurde oder key außerhalb des gültigen
     * Wertebreichs liegt, wird 0 zurückgegeben.
     *
     * Diese Funktion verursacht keine Buslast.
     * \pre initialize() muss aufgerufen worden sein.
     */
    uint32_t getPwmFrequency(unsigned key);
	
	/**
	 * \brief Fragt die Auflösung der analogen Eingänge vom Gerät ab.
	 * \param[out] resolution Puffer, in dem die Auflösung gespeichert wird.
	 * \return True bei Erfolg, ansonsten false.
	 */
    bool getAnalogResolution(unsigned* resolution);
	
	/**
	 * \brief Fragt die sync-Größe vom Gerät ab.
	 * \param[out] size Puffer, in dem die Größe gespeichert wird.
	 * \return True bei Erfolg, ansonsten false.
	 */
    bool getSyncSize(int* size);
	
	/**
	 * \brief Fragt die Anzahl digitaler Eingänge vom Gerät ab.
	 * \param[out] size Puffer, in dem die Größe gespeichert wird.
	 * \return True bei Erfolg, ansonsten false.
	 */
    bool getDigitalInputSize(int* size);
	
	/**
	 * \brief Fragt die Anzahl digitaler Ausgänge vom Gerät ab.
	 * \param[out] size Puffer, in dem die Größe gespeichert wird.
	 * \return True bei Erfolg, ansonsten false.
	 */
    bool getDigitalOutputSize(int* size);
	
 	/**
	 * \brief Fragt die Anzahl analoger Eingänge vom Gerät ab.
	 * \param[out] size Puffer, in dem die Größe gespeichert wird.
	 * \return True bei Erfolg, ansonsten false.
	 */
   bool getAnalogInputSize(int * size);
   
	/**
	 * \brief Fragt die Anzahl von PWM-Ausgängen vom Gerät ab.
	 * \param[out] size Puffer, in dem die Größe gespeichert wird.
	 * \return True bei Erfolg, ansonsten false.
	 */
    bool getPwmOutputSize(int* size);

	
private:
	bool initDigitalOutputBuffer(void);
	bool initPwmOutputBuffer(void);

	Analog_t* analogInputs_;
	Pwm_t* pwmOutputs_;
	uint8_t* syncBuffer_;

	int analogInputSize_;
    int pwmOutputSize_;
	int digitalInputSize_;
	int digitalOutputSize_;
	int syncSize_;

    uint16_t digitalInputs_;
    uint16_t digitalOutputs_;

    bool isSynced_;
};

/**
 * \brief Vereinfacht die Benutzung der Klasse Aseb.
 * \tparam AnalogInputSize Anzahl der analogen Eingänge.
 * \tparam PwmOutputSize Anzahl der PWM-Ausgänge.
 * \tparam HasDigitalInputs True, wenn digitale Eingänge vorhanden sind, ansonsten false.
 * \tparam AddressSize Auf dem Bus verwendete Adresslänge vom Typ Device::AddressLength.
 * 
 * Diese Klasse verwaltet ihre benötigten Puffer
 * selbst. Die Größe kann mit den Template-Parametern 
 * eingestellt werden.
 * 
 * Für jede verbaute ASEB-Platine muss dadurch lediglich eine
 * Instanz dieses Klassen-Templates mit den richtigen
 * Template-Parametern gebildet werden, alles Weitere geschieht im 
 * Hintergrund.
 * 
 */
template<std::size_t AnalogInputSize, std::size_t PwmOutputSize, bool HasDigitalInputs,
         Device::AddressLength AddressSize = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_ADDRESS_LENGTH>
class AsebTemplate : public Aseb {
public:
    static constexpr std::size_t SyncSize = static_cast<uint8_t>(AddressSize) + 2*AnalogInputSize + (HasDigitalInputs?2:0) + 1;
	/**
	 * \brief Konstruktor
     * \param[in] name Interner Name des Gerätes, der von name() zurückgeliefert wird.
     * \param[in] address Busadresse des Gerätes.
     * \param[in] feldbus Referenz auf eine FeldbusAbstraction-Instanz, die den zu verwendenden Bus repräsentiert.
     * \param[in] type Typ der verwendeten Checksumme.
	 */
    AsebTemplate(const char* name, unsigned int address, FeldbusAbstraction& feldbus, ChecksumType type = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE):
        Aseb(name, address, feldbus, type, AddressSize)  { }
	
	/**
	 * \brief Initialisiert das Gerät.
	 * \return True bei Erfolg, ansonsten false.
	 */
    bool initialize(void) {
        return Aseb::initialize(syncBuffer.get(), SyncSize, analog.get(), AnalogInputSize, pwm.get(), PwmOutputSize);
    }

private:
	template<typename T, std::size_t size>
	struct ConditionalArray { T data[size]; T* get() { return data;} };
	
	template<typename T>
	struct ConditionalArray<T, 0> { T* get() { return nullptr;} };
	
    ConditionalArray<uint8_t, SyncSize> syncBuffer;
    ConditionalArray<Analog_t, AnalogInputSize> analog;
    ConditionalArray<Pwm_t, PwmOutputSize> pwm;
};


} // namespace Feldbus
} // namespace TURAG

#endif /* TURAGFELDBUSASEB_H_ */
