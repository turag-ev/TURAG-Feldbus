#ifndef TINA_FELDBUS_DYNAMIXEL_HEADER
#define TINA_FELDBUS_DYNAMIXEL_HEADER

#ifdef __cplusplus
extern "C" {
#endif
    
#include <tina/tina.h>

/////////////////////////////////////////////////////////////
// addresses
/////////////////////////////////////////////////////////////
#define TURAG_DXL_ADDRESS_MODEL_NUMBER		0
#define TURAG_DXL_ADDRESS_FIRMWARE		2
#define TURAG_DXL_ADDRESS_ID			3
#define TURAG_DXL_ADDRESS_BAUDRATE		4
#define TURAG_DXL_ADDRESS_RETURN_DELAY		5
#define TURAG_DXL_ADDRESS_CW_ANGLE_LIMT		6
#define TURAG_DXL_ADDRESS_CCW_ANGLE_LIMIT	8
#define TURAG_DXL_ADDRESS_TEMPERATURE_LIMIT	11
#define TURAG_DXL_ADDRESS_VOLTAGE_LIMIT_LOW	12
#define TURAG_DXL_ADDRESS_VOLTAGE_LIMIT_HIGH	13
#define TURAG_DXL_ADDRESS_MAX_TORQUE		14
#define TURAG_DXL_ADDRESS_STATUS_RETURN_LEVEL	16
#define TURAG_DXL_ADDRESS_ALARM_LED		17
#define TURAG_DXL_ADDRESS_ALARM_SHUTDOWN	18
#define TURAG_DXL_ADDRESS_TORQUE_ENABLE		24
#define TURAG_DXL_ADDRESS_LED			25
#define TURAG_DXL_ADDRESS_CW_COMPLIANCE_MARGIN	26
#define TURAG_DXL_ADDRESS_CCW_COMPLIANCE_MARGIN	27
#define TURAG_DXL_ADDRESS_CW_COMPLIANCE_SLOPE	28
#define TURAG_DXL_ADDRESS_CCW_COMPLIANCE_SLOPE	29
#define TURAG_DXL_ADDRESS_GOAL_POSITION		30
#define TURAG_DXL_ADDRESS_MOVING_SPEED		32
#define TURAG_DXL_ADDRESS_TORQUE_LIMIT		34
#define TURAG_DXL_ADDRESS_PRESENT_POSITION	36
#define TURAG_DXL_ADDRESS_PRESENT_SPEED		38
#define TURAG_DXL_ADDRESS_PRESENT_LOAD		40
#define TURAG_DXL_ADDRESS_PRESENT_VOLTAGE	42
#define TURAG_DXL_ADDRESS_PRESENT_TEMPERATURE	43
#define TURAG_DXL_ADDRESS_REGISTERED		44
#define TURAG_DXL_ADDRESS_MOVING		46
#define TURAG_DXL_ADDRESS_LOCK			47
#define TURAG_DXL_ADDRESS_PUNCH			48

    
///////////// device control methods ////////////////////////
int turag_dxl_initialize(void);

///////////// device specific unit factors ///////////////////////
#define TURAG_DXL_FACTOR_DEGREE     0.2929f
#define TURAG_DXL_FACTOR_VOLTAGE    10.0f
#define TURAG_DXL_FACTOR_TORQUE     10.24f
#define TURAG_DXL_FACTOR_PRESENT_SPEED  0.111f
#define TURAG_DXL_FACTOR_PRESENT_LOAD   0.09765625f
#define TURAG_DXL_FACTOR_MOVING_SPEED   0.111f


//////////// high communication methods ///////////////////////
bool turag_dxl_ping(int id);
bool turag_dxl_read_byte(int id, int address, int* output);
bool turag_dxl_write_byte(int id, int address, int value);
bool turag_dxl_read_word(int id, int address, int* output);
bool turag_dxl_write_word(int id, int address, int value);

int dxl_get_result(void);
#define	TURAG_DXL_COMM_TXSUCCESS		(0)
#define TURAG_DXL_COMM_RXSUCCESS		(1)
#define TURAG_DXL_COMM_TXFAIL			(2)
#define TURAG_DXL_COMM_RXFAIL			(3)
#define TURAG_DXL_COMM_TXERROR			(4)
#define TURAG_DXL_COMM_RXWAITING		(5)
#define TURAG_DXL_COMM_RXTIMEOUT		(6)
#define TURAG_DXL_COMM_RXCORRUPT		(7)

int turag_dxl_get_rxpacket_error(int errbit);
#define TURAG_DXL_ERRBIT_VOLTAGE		(1)
#define TURAG_DXL_ERRBIT_ANGLE			(2)
#define TURAG_DXL_ERRBIT_OVERHEAT		(4)
#define TURAG_DXL_ERRBIT_RANGE			(8)
#define TURAG_DXL_ERRBIT_CHECKSUM		(16)
#define TURAG_DXL_ERRBIT_OVERLOAD		(32)
#define TURAG_DXL_ERRBIT_INSTRUCTION		(64)



#ifdef __cplusplus
}
#endif

#endif // TINA_FELDBUS_DYNAMIXEL_HEADER
