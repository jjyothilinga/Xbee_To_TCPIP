#ifndef APP_H
#define APP_H

#include "board.h"
#include "TCPIP Stack\GenericTypeDefs.h"


#define MAX_ENTRIES 10
#define	MAX_LENGTH_OF_ENTRY 10

enum
{
	CMD_DATA = 0X80, 
	CMD_IP_SET = 0X81
};


extern void APP_init(void);
extern void APP_task(void);
UINT8 App_logRead( far UINT8** logBuff );

#endif










