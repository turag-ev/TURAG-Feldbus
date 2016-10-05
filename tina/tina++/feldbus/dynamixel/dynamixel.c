#define TURAG_DEBUG_LOG_SOURCE "B"

#include <tina/thread.h>

#include "dxl_hal.h"
#include "dynamixel.h"



///////////// set/get packet methods //////////////////////////
#define TURAG_DXL_MAXNUM_TXPARAM		(150)
#define TURAG_DXL_MAXNUM_RXPARAM		(60)

void turag_dxl_set_txpacket_id(int id);
#define TURAG_DXL_BROADCAST_ID			(254)

void turag_dxl_set_txpacket_instruction(int instruction);
#define TURAG_DXL_INST_PING			(1)
#define TURAG_DXL_INST_READ			(2)
#define TURAG_DXL_INST_WRITE			(3)
#define TURAG_DXL_INST_REG_WRITE		(4)
#define TURAG_DXL_INST_ACTION			(5)
#define TURAG_DXL_INST_RESET			(6)
#define TURAG_DXL_INST_SYNC_WRITE		(131)

void turag_dxl_set_txpacket_parameter(int index, int value);
void turag_dxl_set_txpacket_length(int length);

int turag_dxl_get_rxpacket_length(void);
int turag_dxl_get_rxpacket_parameter(int index);


// utility for value
int turag_dxl_makeword(int lowbyte, int highbyte);
int turag_dxl_get_lowbyte(int word);
int turag_dxl_get_highbyte(int word);


////////// packet communication methods ///////////////////////
void turag_dxl_tx_packet(void);
void turag_dxl_rx_packet(void);
void turag_dxl_txrx_packet(void);



#define TURAG_DXL_ID					(2)
#define TURAG_DXL_LENGTH				(3)
#define TURAG_DXL_INSTRUCTION				(4)
#define TURAG_DXL_ERRBIT				(4)
#define TURAG_DXL_PARAMETER				(5)
#define TURAG_DXL_DEFAULT_BAUDNUMBER			(1)

static unsigned char gbInstructionPacket[TURAG_DXL_MAXNUM_TXPARAM+10] = {0};
static unsigned char gbStatusPacket[TURAG_DXL_MAXNUM_RXPARAM+10] = {0};
static unsigned char gbRxPacketLength = 0;
static unsigned char gbRxGetLength = 0;
static int gbCommStatus = TURAG_DXL_COMM_RXSUCCESS;
static int giBusUsing = 0;


#warning Shit not thread-safe. DO NOT use this code from multiple threads.
//static TuragBinarySemaphore sem_;


int turag_dxl_initialize(void *bus) {
    gbCommStatus = turag_dxl_hal_open(bus);
	giBusUsing = 0;

    //turag_binary_semaphore_init(&sem_, false);

	return 1;
}


///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
//////////// packet communication methods /////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

void turag_dxl_tx_packet(void)
{
	unsigned char i;
	unsigned char TxNumByte, RealTxNumByte;
	unsigned char checksum = 0;

	if( giBusUsing == 1 )
		return;
	
	giBusUsing = 1;

	if( gbInstructionPacket[TURAG_DXL_LENGTH] > (TURAG_DXL_MAXNUM_TXPARAM+2) )
	{
		gbCommStatus = TURAG_DXL_COMM_TXERROR;
		giBusUsing = 0;
		return;
	}
	
	if( gbInstructionPacket[TURAG_DXL_INSTRUCTION] != TURAG_DXL_INST_PING
		&& gbInstructionPacket[TURAG_DXL_INSTRUCTION] != TURAG_DXL_INST_READ
		&& gbInstructionPacket[TURAG_DXL_INSTRUCTION] != TURAG_DXL_INST_WRITE
		&& gbInstructionPacket[TURAG_DXL_INSTRUCTION] != TURAG_DXL_INST_REG_WRITE
		&& gbInstructionPacket[TURAG_DXL_INSTRUCTION] != TURAG_DXL_INST_ACTION
		&& gbInstructionPacket[TURAG_DXL_INSTRUCTION] != TURAG_DXL_INST_RESET
		&& gbInstructionPacket[TURAG_DXL_INSTRUCTION] != TURAG_DXL_INST_SYNC_WRITE )
	{
		gbCommStatus = TURAG_DXL_COMM_TXERROR;
		giBusUsing = 0;
		return;
	}
	
	gbInstructionPacket[0] = 0xff;
	gbInstructionPacket[1] = 0xff;
	for( i=0; i<(gbInstructionPacket[TURAG_DXL_LENGTH]+1); i++ )
		checksum += gbInstructionPacket[i+2];
	gbInstructionPacket[gbInstructionPacket[TURAG_DXL_LENGTH]+3] = ~checksum;
	
	if( gbCommStatus == TURAG_DXL_COMM_RXTIMEOUT || gbCommStatus == TURAG_DXL_COMM_RXCORRUPT )
		turag_dxl_hal_clear();

	TxNumByte = gbInstructionPacket[TURAG_DXL_LENGTH] + 4;
	RealTxNumByte = turag_dxl_hal_tx( (unsigned char*)gbInstructionPacket, TxNumByte );

	if( TxNumByte != RealTxNumByte )
	{
		gbCommStatus = TURAG_DXL_COMM_TXFAIL;
		giBusUsing = 0;
		return;
	}

	if( gbInstructionPacket[TURAG_DXL_INSTRUCTION] == TURAG_DXL_INST_READ )
		turag_dxl_hal_set_timeout( gbInstructionPacket[TURAG_DXL_PARAMETER+1] + 6 );
	else
		turag_dxl_hal_set_timeout( 6 );

	gbCommStatus = TURAG_DXL_COMM_TXSUCCESS;
}

void turag_dxl_rx_packet(void) {
	unsigned char i, j, nRead;
	unsigned char checksum = 0;

	if( giBusUsing == 0 )
		return;

    if( gbInstructionPacket[TURAG_DXL_ID] == TURAG_DXL_BROADCAST_ID ) {
		gbCommStatus = TURAG_DXL_COMM_RXSUCCESS;
		giBusUsing = 0;
		return;
	}
	
    if( gbCommStatus == TURAG_DXL_COMM_TXSUCCESS ) {
		gbRxGetLength = 0;
		gbRxPacketLength = 6;
	}
	
	nRead = turag_dxl_hal_rx( (unsigned char*)&gbStatusPacket[gbRxGetLength], gbRxPacketLength - gbRxGetLength );
	gbRxGetLength += nRead;
	if( gbRxGetLength < gbRxPacketLength )
	{
		if( turag_dxl_hal_timeout() == 1 )
		{
			if(gbRxGetLength == 0)
				gbCommStatus = TURAG_DXL_COMM_RXTIMEOUT;
			else
				gbCommStatus = TURAG_DXL_COMM_RXCORRUPT;
			giBusUsing = 0;
			return;
		}
	}
	
	// Find packet header
    for( i=0; i < (gbRxGetLength-1); i++ )
	{
		if( gbStatusPacket[i] == 0xff && gbStatusPacket[i+1] == 0xff )
		{
			break;
		}
		else if( i == gbRxGetLength-2 && gbStatusPacket[gbRxGetLength-1] == 0xff )
		{
			break;
		}
	}	
	if( i > 0 )
	{
		for( j=0; j<(gbRxGetLength-i); j++ )
			gbStatusPacket[j] = gbStatusPacket[j + i];
			
		gbRxGetLength -= i;		
	}

	if( gbRxGetLength < gbRxPacketLength )
	{
		gbCommStatus = TURAG_DXL_COMM_RXWAITING;
		return;
	}

	// Check id pairing
    if( gbInstructionPacket[TURAG_DXL_ID] != gbStatusPacket[TURAG_DXL_ID])
	{
		gbCommStatus = TURAG_DXL_COMM_RXCORRUPT;
		giBusUsing = 0;
		return;
	}
	
	gbRxPacketLength = gbStatusPacket[TURAG_DXL_LENGTH] + 4;
	if( gbRxGetLength < gbRxPacketLength )
	{
		nRead = turag_dxl_hal_rx( (unsigned char*)&gbStatusPacket[gbRxGetLength], gbRxPacketLength - gbRxGetLength );
		gbRxGetLength += nRead;
		if( gbRxGetLength < gbRxPacketLength )
		{
			gbCommStatus = TURAG_DXL_COMM_RXWAITING;
			return;
		}
	}

	// Check checksum
	for( i=0; i<(gbStatusPacket[TURAG_DXL_LENGTH]+1); i++ )
		checksum += gbStatusPacket[i+2];
	checksum = ~checksum;

	if( gbStatusPacket[gbStatusPacket[TURAG_DXL_LENGTH]+3] != checksum )
	{
		gbCommStatus = TURAG_DXL_COMM_RXCORRUPT;
		giBusUsing = 0;
		return;
	}
	
	gbCommStatus = TURAG_DXL_COMM_RXSUCCESS;
	giBusUsing = 0;
}

void turag_dxl_txrx_packet(void)
{
	turag_dxl_tx_packet();

	if( gbCommStatus != TURAG_DXL_COMM_TXSUCCESS )
		return;	
	
    do {
		turag_dxl_rx_packet();		
    } while( gbCommStatus == TURAG_DXL_COMM_RXWAITING );
}


///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
//////////// set/get packet methods ///////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

void turag_dxl_set_txpacket_id( int id )
{
	gbInstructionPacket[TURAG_DXL_ID] = (unsigned char)id;
}

void turag_dxl_set_txpacket_instruction( int instruction )
{
	gbInstructionPacket[TURAG_DXL_INSTRUCTION] = (unsigned char)instruction;
}

void turag_dxl_set_txpacket_parameter( int index, int value )
{
	gbInstructionPacket[TURAG_DXL_PARAMETER+index] = (unsigned char)value;
}

void turag_dxl_set_txpacket_length( int length )
{
	gbInstructionPacket[TURAG_DXL_LENGTH] = (unsigned char)length;
}

int turag_dxl_get_rxpacket_length(void)
{
	return (int)gbStatusPacket[TURAG_DXL_LENGTH];
}

int turag_dxl_get_rxpacket_parameter( int index )
{
	return (int)gbStatusPacket[TURAG_DXL_PARAMETER+index];
}

int turag_dxl_makeword( int lowbyte, int highbyte )
{
	unsigned short word;

	word = highbyte;
	word = word << 8;
	word = word + lowbyte;
	return (int)word;
}

int turag_dxl_get_lowbyte( int word )
{
	unsigned short temp;

	temp = word & 0xff;
	return (int)temp;
}

int turag_dxl_get_highbyte( int word )
{
	unsigned short temp;

	temp = word & 0xff00;
	temp = temp >> 8;
	return (int)temp;
}


///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
//////////// high communication methods ///////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
bool turag_dxl_ping( int id ) {
    //turag_binary_semaphore_wait(&sem_);

    while (giBusUsing);

	gbInstructionPacket[TURAG_DXL_ID] = (unsigned char)id;
	gbInstructionPacket[TURAG_DXL_INSTRUCTION] = TURAG_DXL_INST_PING;
	gbInstructionPacket[TURAG_DXL_LENGTH] = 2;
	
	turag_dxl_txrx_packet();

    bool result = (gbCommStatus == TURAG_DXL_COMM_RXSUCCESS);

    //turag_binary_semaphore_signal(&sem_);

    return result;
}

bool turag_dxl_read_byte( int id, int address, int* output ) {
    //turag_binary_semaphore_wait(&sem_);

    while(giBusUsing);

	gbInstructionPacket[TURAG_DXL_ID] = (unsigned char)id;
	gbInstructionPacket[TURAG_DXL_INSTRUCTION] = TURAG_DXL_INST_READ;
	gbInstructionPacket[TURAG_DXL_PARAMETER] = (unsigned char)address;
	gbInstructionPacket[TURAG_DXL_PARAMETER+1] = 1;
	gbInstructionPacket[TURAG_DXL_LENGTH] = 4;
	
	turag_dxl_txrx_packet();

	*output = (int)gbStatusPacket[TURAG_DXL_PARAMETER];
	
    bool result = (gbCommStatus == TURAG_DXL_COMM_RXSUCCESS);

    //turag_binary_semaphore_signal(&sem_);

    return result;
}

bool turag_dxl_write_byte( int id, int address, int value ) {
    //turag_binary_semaphore_wait(&sem_);

    while(giBusUsing);

	gbInstructionPacket[TURAG_DXL_ID] = (unsigned char)id;
	gbInstructionPacket[TURAG_DXL_INSTRUCTION] = TURAG_DXL_INST_WRITE;
	gbInstructionPacket[TURAG_DXL_PARAMETER] = (unsigned char)address;
	gbInstructionPacket[TURAG_DXL_PARAMETER+1] = (unsigned char)value;
	gbInstructionPacket[TURAG_DXL_LENGTH] = 4;
	
	turag_dxl_txrx_packet();
	
    bool result = (gbCommStatus == TURAG_DXL_COMM_RXSUCCESS);

    //turag_binary_semaphore_signal(&sem_);

    return result;
}

bool turag_dxl_read_word( int id, int address, int* output ) {
    //turag_binary_semaphore_wait(&sem_);

    while(giBusUsing);

	gbInstructionPacket[TURAG_DXL_ID] = (unsigned char)id;
	gbInstructionPacket[TURAG_DXL_INSTRUCTION] = TURAG_DXL_INST_READ;
	gbInstructionPacket[TURAG_DXL_PARAMETER] = (unsigned char)address;
	gbInstructionPacket[TURAG_DXL_PARAMETER+1] = 2;
	gbInstructionPacket[TURAG_DXL_LENGTH] = 4;
	
	turag_dxl_txrx_packet();

	*output = turag_dxl_makeword((int)gbStatusPacket[TURAG_DXL_PARAMETER], (int)gbStatusPacket[TURAG_DXL_PARAMETER+1]);
	
    bool result = (gbCommStatus == TURAG_DXL_COMM_RXSUCCESS);

    //turag_binary_semaphore_signal(&sem_);

    return result;
}

bool turag_dxl_write_word( int id, int address, int value ) {
    //turag_binary_semaphore_wait(&sem_);

    while(giBusUsing);

	gbInstructionPacket[TURAG_DXL_ID] = (unsigned char)id;
	gbInstructionPacket[TURAG_DXL_INSTRUCTION] = TURAG_DXL_INST_WRITE;
	gbInstructionPacket[TURAG_DXL_PARAMETER] = (unsigned char)address;
	gbInstructionPacket[TURAG_DXL_PARAMETER+1] = (unsigned char)turag_dxl_get_lowbyte(value);
	gbInstructionPacket[TURAG_DXL_PARAMETER+2] = (unsigned char)turag_dxl_get_highbyte(value);
	gbInstructionPacket[TURAG_DXL_LENGTH] = 5;
	
	turag_dxl_txrx_packet();
	
    bool result = (gbCommStatus == TURAG_DXL_COMM_RXSUCCESS);

    //turag_binary_semaphore_signal(&sem_);

    return result;
}


int turag_dxl_get_result(void) {
    //turag_binary_semaphore_wait(&sem_);

    int result = gbCommStatus;

    //turag_binary_semaphore_signal(&sem_);

    return result;
}


int turag_dxl_get_rxpacket_error( int errbit ) {
    //turag_binary_semaphore_wait(&sem_);

    int result;

    if ( gbStatusPacket[TURAG_DXL_ERRBIT] & (unsigned char)errbit ) {
        result = 1;
    } else {
        result = 0;
    }

    //turag_binary_semaphore_signal(&sem_);

    return result;
}

