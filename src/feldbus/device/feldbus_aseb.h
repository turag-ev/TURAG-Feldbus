/**
 *  @brief		Implements slave side %TURAG Feldbus f�r ASEB support
 *  @file		feldbus_aseb.h
 *  @date		11.01.2014
 *  @author		Martin Oemus <martin@oemus.net>
 *  @ingroup 	feldbus-slave-aseb
 *  @see \ref feldbus-slave-aseb
 *
 *
 */

/**
 * @defgroup   feldbus-slave-aseb ASEBs
 * @ingroup	feldbus-slave
 * 
 * [ASEBs](https://intern.turag.de/wiki/doku.php/id,04_programmierung;protokolle_busse;turag-simplebus;turag_feldbus_io-boards/)
 * sind kleine und simple  Boards, die zum Verteilen von Ein- und Ausg�ngen im Roboter benutzt werden.
 * 
 * Das Interface des ASEB-Protokolls ist ebenfalls sehr simpel: 
 * der Init-Funktion turag_feldbus_aseb_init() werden Pointer auf Strukturen
 * �bergeben, die die Ein- und Ausg�nge abstrahieren, au�erdem deren Anzahl.
 * In der Hauptschleife werden die Werte aller Ausg�nge ausgelesen und entsprechend
 * in der Hardware gesetzt. Der Status aller Eing�nge wird vom Ger�t gelesen
 * und in die Strukturen eingetragen. 
 * 
 * Ein Aufruf von turag_feldbus_do_processing()
 * synchronisiert bei entsprechender Anfrage Master und Slave: neue Werte f�r Ausg�nge werden in
 * die Strukturen �bernommen und die aktuellen Eingangswerte werden an den Master
 * gesandt.
 * 
 * @section feldbus-slave-aseb-config Konfiguration
 * Die folgenden Makros werden von diesem Protokoll in feldbus_config.h 
 * erwartet:
 * 
 * **TURAG_FELDBUS_ASEB_COMMAND_NAMES_USING_AVR_PROGMEM:**\n
 * Jeder Ein- und Ausgang wird mit einem Namen gekennzeichnet. Damit diese Strings
 * auf AVR-Targets nicht zu viel RAM verbrauchen, ist es m�glich sie im
 * Flash zu speichern. Daf�r muss dieses Makro auf 1 definiert werden.
 * 
 */


#ifndef TINA_FELDBUS_SLAVE_FELDBUS_ASEB_H_
#define TINA_FELDBUS_SLAVE_FELDBUS_ASEB_H_

#include <feldbus/device/feldbus_config_check.h>



/**
 * \brief Typ zur Definition digitaler Ein-/Ausgänge.
 */
typedef struct {
	uint8_t value; 		///< Speichert den aktuellen Wert.
	const char* name; 	///< Bezeichnung.
} feldbus_aseb_digital_io_t;

/**
 * \brief Typ zur Definition analoger Eingänge.
 * 
 * factor wird vom Master verwendet, um den gelesenen rohen Input-Wert beispielsweise
 * in eine einheitenbehaftete Endgröße umzurechnen.
 */
typedef struct {
	const float factor;		///< Faktor, mit dem der gemessene Analogwert vom Master multipliziert werden soll.
	int16_t value;			///< Speichert den aktuell gemessenen Wert.
	const char* name;		///< Bezeichnung.
} feldbus_aseb_analog_t;

/**
 * \brief Typ zur Definition von PWM-Ausgängen.
 * 
 * frequency und max_value sind konstant und haben informativen Charakter.
 * 
 * Das Verhältnis von value zu max_value ergibt den eingestellten
 * duty cycle des PWM-Ausgangs.
 */
typedef struct {
	const uint32_t frequency;		///< Gibt die PWM-Frequenz des Ausgangs an.
	const uint16_t max_value;		///< Gibt den maximal einstellbaren Ausgangswert an.
	uint16_t value;					///< Speichert den aktuellen Ausgangswert.
	uint16_t target_value; 			///<Speichert den Zielwert
    uint16_t speed;                 ///<Speichert die Geschwindigkeit, welche als zu addierender/subtrahierender Wert zu value dargestellt ist
	const char* name;				///< Bezeichnung.
} feldbus_aseb_pwm_t;


/**
 * Initialisiert das Modul.
 *
 * This function calls turag_feldbus_slave_init(),
 * so you should not do this again.
 * 
 * @param[in] digital_inputs
 * @param[in] digital_inputs_size 
 * @param[in] digital_outputs 
 * @param[in] digital_outputs_size 
 * @param[in] analog_inputs 
 * @param[in] analog_inputs_size 
 * @param[in] pwm_outputs 
 * @param[in] pwm_outputs_size 
 * @param[in] analog_resolution
 */
void turag_feldbus_aseb_init(
    feldbus_aseb_digital_io_t* digital_inputs, const uint8_t digital_inputs_size,
    feldbus_aseb_digital_io_t* digital_outputs, const uint8_t digital_outputs_size,
    feldbus_aseb_analog_t* analog_inputs, const uint8_t analog_inputs_size,
	feldbus_aseb_pwm_t* pwm_outputs, const uint8_t pwm_outputs_size,
	const uint8_t analog_resolution);


FeldbusSize_t turag_feldbus_aseb_process_package(const uint8_t* message, FeldbusSize_t message_length, uint8_t* response);

#endif /* TINA_FELDBUS_SLAVE_FELDBUS_ASEB_H_ */
