#ifndef TURAG_CHIBIOS_DYNAMIXEL_HAL_HEADER
#define TURAG_CHIBIOS_DYNAMIXEL_HAL_HEADER


#ifdef __cplusplus
extern "C" {
#endif


int turag_dxl_hal_open(int deviceIndex, float baudrate);
void turag_dxl_hal_close(void);
int turag_dxl_hal_set_baud( float baudrate );
void turag_dxl_hal_clear(void);
int turag_dxl_hal_tx( unsigned char *pPacket, int numPacket );
int turag_dxl_hal_rx( unsigned char *pPacket, int numPacket );
void turag_dxl_hal_set_timeout( int NumRcvByte );
int turag_dxl_hal_timeout(void);



#ifdef __cplusplus
}
#endif

#endif // TURAG_CHIBIOS_DYNAMIXEL_HAL_HEADER
