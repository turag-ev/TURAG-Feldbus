#ifndef TINA_FELDBUS_DYNAMIXEL_HEADER
#define TINA_FELDBUS_DYNAMIXEL_HEADER

#ifdef __cplusplus
extern "C" {
#endif
    
#include <tina/tina.h>

///////////// device control methods ////////////////////////
int turag_dxl_initialize();


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
