#include "app.h"
#include "communication.h"
#include "config.h"
#include "uart.h"
#include "string.h"
/*
*------------------------------------------------------------------------------
* Structures
*------------------------------------------------------------------------------
*/

typedef struct _EVENT_LOG
{
	UINT8 writeIndex;
	UINT8 readIndex;
	UINT8 logBuffer[MAX_ENTRIES][MAX_LENGTH_OF_ENTRY];
}EVENT_LOG;


/*
*------------------------------------------------------------------------------
* Variables
*------------------------------------------------------------------------------
*/

#pragma idata LOG_DATA
EVENT_LOG log = {0};
#pragma idata


/*------------------------------------------------------------------------------
* Private Functions
*------------------------------------------------------------------------------
*/

void logWrite( far UINT8 *data, UINT8 length );
UINT8 APP_comCallBack( far UINT8 *rxPacket,  far UINT8* txCode, far UINT8** txPacket);

/*
*------------------------------------------------------------------------------
* void APP-init(void)
*------------------------------------------------------------------------------
*/

void APP_init(void)
{

	COM_init(CMD_SOP , CMD_EOP ,RESP_SOP , RESP_EOP , APP_comCallBack);
}


/*
*------------------------------------------------------------------------------
* void APP_task(void)
*------------------------------------------------------------------------------
*/

void APP_task(void)
{
	UINT8 data = 0;

	if( UART_hasData() )
	{
		data = UART_read();
		UART_write( data );
	}


}
/*---------------------------------------------------------------------------------------------------------------
*	UINT8 logRead(void)
*----------------------------------------------------------------------------------------------------------------
*/
UINT8 App_logRead( far UINT8** logBuff )
{
	UINT8 length = 0;

	if( log.writeIndex == log.readIndex )
	{
		*logBuff = 0;
	}
	else
	{
		*logBuff = log.logBuffer[log.readIndex];
		length = strlen( log.logBuffer[log.readIndex] );
		//length = 4;
		
		log.readIndex++;
		if( log.readIndex >= MAX_ENTRIES)
			log.readIndex = 0;	
	}

	return length;
			
}


/*---------------------------------------------------------------------------------------------------------------
*	void updateLog(void)
*----------------------------------------------------------------------------------------------------------------
*/
void logWrite( far UINT8 *data, UINT8 length )
{
	UINT8 i;
	UINT8 temp;

	//Store data in the buffer
	for( i = 0; i < length; i++ )
	{
		log.logBuffer[log.writeIndex][i] = *(data+i);
	}

	log.writeIndex++;
	if( log.writeIndex >= MAX_ENTRIES)
		log.writeIndex = 0;


}
/*
*------------------------------------------------------------------------------
* void APP_comCallBack(void)
*
* Summary	: 
*
* Input		: None
*
* Output	: None
*------------------------------------------------------------------------------
*/

UINT8 APP_comCallBack( UINT8 *rxPacket, UINT8* txCode, UINT8** txPacket)
{

	UINT8 i;
   	

	UINT8 rxCode = rxPacket[0];

	UINT8 length = strlen(rxPacket);
		    	

	logWrite(rxPacket, length);

	return length;

}