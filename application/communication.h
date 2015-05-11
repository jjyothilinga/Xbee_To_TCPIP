
#ifndef __COMMUNICATION__
#define __COMMUNICATION__

//#include "board.h"
#include "GenericTypeDefs.h"



#define SUCCESS			1
#define FAILURE			0

typedef enum
{
	COM_RESP_NONE = 0xFF,
	COM_RESP_OK = 0,
	COM_RESP_OVERRUN = 0x90,
	COM_RESP_CHECKSUM_ERROR = 0x91,
	COM_RESP_INVALID_CMD = 0x92, 
	COM_RESP_INVALID_DATA = 0x93
}COM_RESP_CODE;




enum 
{ 
	COM_RESET = 0,
	COM_START = 1,
	COM_IN_PACKET_COLLECTION = 2,
	COM_IN_TX_DATA = 3
};




enum 
{
	COM_DEVICE_ADDRESS_INDEX = 0,
	COM_TX_CODE_INDEX = 1,
	COM_TX_DATA_START_INDEX = 2,
	COM_RX_DATA_START_INDEX = 2
};





void COM_init(UINT8 cmd_sop , UINT8 cmd_eop ,UINT8 resp_sop , UINT8 resp_eop , UINT8 (*callBack)(UINT8* rxdata, UINT8* txCode,UINT8** txPacket));
void COM_txCMD(UINT8 deviceAddress, 
		/*	UINT8 cmd,*/ UINT8 *buffer , UINT8 length);
void COM_task(void);
void COM_txStr(rom UINT8 *str);
#endif


