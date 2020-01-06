/**
 *  @brief		TURAG feldbus color sensor
 *  @file		farbsensor_tina.cpp
 *  @date		24.03.2013
 *  @author		Martin Oemus
 *
 */

#include <tina++/tina.h>
#if TURAG_USE_TURAG_FELDBUS_HOST

#include "farbsensor.h"
#include <tina/feldbus/protocol/turag_feldbus_fuer_lokalisierungssensoren.h>
#include <tina++/thread.h>

namespace TURAG {
namespace Feldbus {

struct hsvMsg_t {
	uint8_t h;
	uint8_t s;
	uint8_t v;
} _packed;

struct rgbMsg_t {
	uint8_t r,g,b;
} _packed;


#define WITHIN_BOUNDARIES(value, bottom, top) (((value >= bottom) && (value <= top)) ? true : false)

const char Farbsensor::myColorStrings[myNumberOfColors + 2][myColorStringSize] = {
		"white",
		"red",
		"blue",
		"black",
		"???",
		"ERROR"
};


Farbsensor::Color Farbsensor::getColor(void) {
	hsv_t hsv;
	Color result = Color::Error;
	if (!getHSV(&hsv)) {
		result = Color::Error;

	} else {
//        turag_infof("h=%d s=%d v=%d", hsv.h, hsv.s, hsv.v);

		for (int i = 0; i < myNumberOfColors; i++) {
//            turag_infof("color id=%d hmin=%d hmax=%d smin=%d smax=%d vmin=%d vmax=%d", i, myColorIndexTable[i].h_min, myColorIndexTable[i].h_max, myColorIndexTable[i].s_min, myColorIndexTable[i].s_max, myColorIndexTable[i].v_min, myColorIndexTable[i].v_max);
            if (
					myColorIndexTable[i].initialized &&
					WITHIN_BOUNDARIES(hsv.h, myColorIndexTable[i].h_min, myColorIndexTable[i].h_max) &&
					WITHIN_BOUNDARIES(hsv.s, myColorIndexTable[i].s_min, myColorIndexTable[i].s_max) &&
                    WITHIN_BOUNDARIES(hsv.v, myColorIndexTable[i].v_min, myColorIndexTable[i].v_max) ) {

				result = static_cast<Farbsensor::Color>(i);
//                turag_infof("color match");
                break;
			}
		}
	}

	lastKnownColor = result;
	return result;
}

void Farbsensor::setColorThresholds(Color color_index, uint16_t h_min, uint16_t h_max,
		uint16_t s_min, uint16_t s_max, uint8_t v_min, uint8_t v_max) {
	myColorIndexTable[static_cast<int>(color_index)].h_min = h_min;
	myColorIndexTable[static_cast<int>(color_index)].h_max = h_max;
	myColorIndexTable[static_cast<int>(color_index)].s_min = s_min;
	myColorIndexTable[static_cast<int>(color_index)].s_max = s_max;
	myColorIndexTable[static_cast<int>(color_index)].v_min = v_min;
	myColorIndexTable[static_cast<int>(color_index)].v_max = v_max;
	myColorIndexTable[static_cast<int>(color_index)].initialized = true;
}

bool Farbsensor::initiateMeasurement() {
	uint8_t msg[addressLength() + 1];
    return transceive(msg, sizeof(msg), nullptr, 0);
}

bool Farbsensor::getRGB(rgb_t* rgb) {
	if (!rgb) return false;

	if (!initiateMeasurement()) return false;

//#warning Thrad delay still implemented, test for necessity!
    CurrentThread::delay(20_ms);

	Request<uint8_t> request;
	request.data = CMD_CLR_RGB;

	Response<rgbMsg_t> response;

	bool success = transceive(request, &response);

	if (success) {
		rgb->r = response.data.r;
		rgb->g = response.data.g;
		rgb->b = response.data.b;
		return true;
	} else {
		return false;
	}
}

bool Farbsensor::getHSV(hsv_t* hsv) {
	if (!hsv) return false;

	if (!initiateMeasurement()) return false;

//#warning Thrad delay still implemented, test for necessity!
    CurrentThread::delay(20_ms);

	Request<uint8_t> request;
	request.data = CMD_CLR_HSV;

	Response<hsvMsg_t> response;

	bool success = transceive(request, &response);

	if (success) {
		hsv->h = response.data.h;
		hsv->s = response.data.s;
		hsv->v = response.data.v;
		return true;
	} else {
		return false;
	}
}

} // namespace Feldbus
} // ns TURAG

#endif // TURAG_USE_TURAG_FELDBUS_HOST
