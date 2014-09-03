/**
 *  @brief		Implements slave side TURAG feldbus für ASEB support
 *  @file		feldbus_aseb.h
 *  @date		11.01.2014
 *  @author		Martin Oemus <martin@oemus.net>
 *  @ingroup 	feldbus-slave-aseb
 *
 *
 */

/**
 *  @defgroup   feldbus-slave-aseb ASEBs
 *  @ingroup	feldbus-slave
*/

#ifndef TINA_FELDBUS_SLAVE_FELDBUS_ASEB_H_
#define TINA_FELDBUS_SLAVE_FELDBUS_ASEB_H_

#include <tina/tina.h>
#include <feldbus_config.h>
#include "feldbus.h"


#if (TURAG_FELDBUS_DEVICE_PROTOCOL==TURAG_FELDBUS_DEVICE_PROTOCOL_ASEB) || defined(DOXYGEN)


typedef struct {
	uint8_t value;
	char* name;
} feldbus_aseb_digital_io_t;

typedef struct {
	float factor;
	int16_t value;
	char* name;
} feldbus_aseb_analog_t;

typedef struct {
	uint32_t frequency;
	uint16_t max_value;
	uint16_t value;
	char* name;
} feldbus_aseb_pwm_t;


/**
 * Sets up this module.
 *
 * This function calls turag_feldbus_slave_init(),
 * so you should not do this again.
 * 
 * @param digital_inputs
 * @param digital_inputs_size 
 * @param digital_outputs 
 * @param digital_outputs_size 
 * @param analog_inputs 
 * @param analog_inputs_size 
 * @param pwm_outputs 
 * @param pwm_outputs_size 
 * @param analog_resolution
 */
void turag_feldbus_aseb_init(
    feldbus_aseb_digital_io_t* digital_inputs, uint8_t digital_inputs_size,
    feldbus_aseb_digital_io_t* digital_outputs, uint8_t digital_outputs_size,
    feldbus_aseb_analog_t* analog_inputs, uint8_t analog_inputs_size,
	feldbus_aseb_pwm_t* pwm_outputs, uint8_t pwm_outputs_size,
	uint8_t analog_resolution);


/*!
 * \brief This function toggles some status led.
 * 
 * The device implementation needs to provide this function.
 */
extern void turag_feldbus_aseb_toggle_status_led(void);

/*!
 * \brief Needs to be called peridocally with a frequency
 * of approximately 75 Hz.
 * 
 * This function toggles the status led with some characteristic
 * pattern and increments the uptime counter.
 */
void turag_feldbus_aseb_periodic_function(void);


#endif

#endif /* TINA_FELDBUS_SLAVE_FELDBUS_ASEB_H_ */
