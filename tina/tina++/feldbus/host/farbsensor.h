/**
 *  @brief		TURAG feldbus color sensor
 *  @file		farbsensor.h
 *  @date		24.03.2013
 *  @author		Martin Oemus
 * 
 */


#ifndef TURAGFELDBUSFARBSENSOR_H_
#define TURAGFELDBUSFARBSENSOR_H_

#include "sensor.h"
#include <tina++/tina.h>

namespace TURAG {
namespace Feldbus {

/*
 *
 */
class Farbsensor: public TURAG::Feldbus::Sensor {
public:
	// do not specify numbers manually!
	// topmost colors have higher priority than successive ones
	// the order of this enum must match the order of the color strings (see below)
	enum class Color {
		White,
		Red,
		Blue,
		Black,
		// Do not add any colors below!!!
		// Do not change anythong below!!!
		Unknown,		// color doesn't match any defined pattern
		Error			// couldn't receive color
	};

	typedef struct {
		uint8_t r, g, b;
	} rgb_t;

	typedef struct {
		uint8_t h,s,v;
	} hsv_t;

	typedef struct {
		uint8_t v_min, v_max;
		uint16_t h_min, h_max, s_min, s_max;
		bool initialized;
	} hsv_range_t;

protected:
	static const int myNumberOfColors = (int)Color::Unknown;
	static const int myColorStringSize = 10;
	static const char myColorStrings[myNumberOfColors + 2][myColorStringSize];

	hsv_range_t myColorIndexTable[myNumberOfColors];
	Color lastKnownColor;

	bool initiateMeassurement(void);

public:
    Farbsensor(const char* name_, unsigned int address, ChecksumType type = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE) :
            Sensor(name_, address, type), lastKnownColor(Color::Error) {
		for (int i = 0; i < myNumberOfColors; i++)
			myColorIndexTable[i].initialized = false;
	}

	Color getColor(void);
	Color getLastKnownColor(void) { return lastKnownColor; }
	bool getRGB(rgb_t* rgb);
	bool getHSV(hsv_t* hsv);
	void setColorThresholds(Color color, uint16_t h_min, uint16_t h_max, uint16_t s_min, uint16_t s_max, uint8_t v_min, uint8_t v_max);

	static const char* getColorString(Color color)  { return myColorStrings[(int)color]; }

};

} // namespace Feldbus
} // namespace TURAG

#endif /* TURAGFELDBUSFARBSENSOR_H_ */
