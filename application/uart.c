
/*
*------------------------------------------------------------------------------
* Include Files
*------------------------------------------------------------------------------
*/

#include <delays.h>
#include <string.h>
#include "uart.h"
#include <usart.h>
#include "TCPIP Stack/Delay.h"
#include "Configs/HWP PICDN2_ETH97.h"




/*
*------------------------------------------------------------------------------
* Private Defines
*------------------------------------------------------------------------------
*/
/*


*/
/*
*------------------------------------------------------------------------------
* Private Macros
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Private Data Types
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Variables
*------------------------------------------------------------------------------
*/


/*
*------------------------------------------------------------------------------
* Private Variables (static)
*------------------------------------------------------------------------------
*/
#pragma idata UART_DATA
UART uart = {0,0,0,{0}};

#pragma idata
/*
*------------------------------------------------------------------------------
* Public Constants
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Private Constants (static)
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Private Function Prototypes (static)
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Private Functions
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Functions
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* called in High ISR function
*------------------------------------------------------------------------------
*/


/*
*------------------------------------------------------------------------------
* void  UartReceiveHandler(void)
*
* Summary	: Handle the recepion of data throug UART
*
* Input		: None
*
* Output	: None
*
* Note		: PC Command Fromat: (PC to Device)
*
* Field:	SOF     STATIONID  DATALEN   CMD/STATUS    DATAN           BCC     EOF
*
* offset:	0	    1	       2	     3		       4               N+4     N+5
*
* value:	0xAA	0x00	   N+1	     CMD	       DATA[0..N-1]    BCC     0xBB
*
* Desc:		BCC = STATIONID ^ DATALEN ^ CMD/STATUS ^ DATA[0] ^ DATA[1]...^... DATA[N-1]
*
*------------------------------------------------------------------------------
*/
#pragma code
#pragma interrupt Uart_ReceiveHandler
void  Uart_ReceiveHandler(void)
{
  	volatile UINT8 data;
  	// Get the character received from the UART
  	data = Read1USART();


    uart.rxBuff[uart.rxBuffIndex++] = data;

	if( uart.rxBuffIndex >= RX_PACKET_SIZE)
	{
		uart.rxBuffIndex = 0;
	}
	uart.rxDataCount++;

	// Clear the interrupt flag
    PIR1bits.RCIF = 0;


}





/*
*
*
*/
//#pragma code
//pragma interrupt Uart_TransmitHandler
void Uart_TransmitHandler(void)
{
	uart.txDataCount--;
		
	if(uart.txDataCount > 0)
    {	
 		TXREG1 = uart.txBuff[uart.txDataIndex];

		while(Busy1USART());
		uart.txDataIndex++;
	
		if(uart.txDataIndex >= TX_PACKET_SIZE)
			uart.txDataIndex = 0;

		uart.txDataCount--;
	}
	else
		uart.initiateTx = 1;	
	
	TXSTA1bits.TXEN = 0;		//Disable Transmission
	PIR1bits.TXIF = 0;


}





/*
*------------------------------------------------------------------------------
* void InitializeUART(void)
*
* Summary	: Initialize Uart for communication with PC
*			  char conunters
*				Transimission and Reception interrupts are enabled
*				
* 				Refer PIC 18F4520 datasheet Table 18-3.
*
* Input		: None
*
* Output	: None
*------------------------------------------------------------------------------
*/
void UART_init( unsigned long baudRate )
{
	UINT8 uartConfig= /*USART_TX_INT_ON*/USART_TX_INT_OFF &
				USART_RX_INT_ON &
				USART_ASYNCH_MODE &
				USART_EIGHT_BIT &
				USART_CONT_RX &
				USART_BRGH_HIGH;

//	SPBRG = ( FOSC / (16 ( n+1) )
	baudRate += ( unsigned long ) 1;
	baudRate *= ( unsigned long ) 16;
	baudRate = GetSystemClock() / baudRate;


//	baudRate = 33;

// configure USART
	Open1USART( uartConfig , baudRate );
				
// Enable interrupt priority
 	RCONbits.IPEN = 1;
	RCSTA1bits.SPEN = 1;
	TXSTA1bits.TXEN = 1;	//disable transmission
	TXSTA1bits.CSRC = 0;
	PIE1bits.RC1IE = 1;
//	PIE1bits.TXIE = 1;

//	IPR1bits.TXIP = 1;	//make transmit interrupt high priority


 	// Make receive interrupt high priority
  	IPR1bits.RCIP = 1;

	uart.initiateTx = 1;

}



/*
*------------------------------------------------------------------------------
* BOOL UART1_write(UINT8 data)
*
* Summary	: Fill the transmit buffer of the uart with data
* Input		: data to be transmitted which is stored in the txBuff
*
* Output	: Boolean indicating success or failure
* 
*------------------------------------------------------------------------------
*/
BOOL UART_write(UINT8 data)
{

	//DISABLE_UART_TX_INTERRUPT();		//disable the transmit interrupt			
/*	PIE1bits.TXIE = 0;		//disable the transmit interrupt 
	uart.txBuff[uart.txBuffIndex] = data;	//store the data in the tx buffer
	uart.txBuffIndex++;
	if(uart.txBuffIndex >= TX_PACKET_SIZE)	//check for overflow
	{
		uart.txBuffIndex = 0;
	}
	
	uart.txDataCount++;											//increment the data count
	
	if( uart.initiateTx == 1 && uart.txDataCount == 1 )		
	{

		TXSTA1bits.TXEN = 1;  		// Enable Transmission
		while (!TXSTA1bits.TRMT);	//check whether ready to transmit
		TXREG1 = ((UINT8)(uart.txBuff[uart.txDataIndex]));					//load new data for transmission
			
		uart.txDataIndex++;
	 
		if( uart.txDataIndex >= TX_PACKET_SIZE)
			uart.txDataIndex = 0;



	}
	//ENABLE_UART_TX_INTERRUPT();
	PIE1bits.TXIE = 1;	//Enable the transmit interrupt 
*/

	while(Busy1USART());					//wait for current transmission if any

	//DISABLE_UART_TX_INTERRUPT();		//disable the transmit interrupt
	//PIE1bits.TXIE = 0;		//disable the transmit interrupt 
	uart.txBuff[uart.txBuffIndex++] = data;	//store the data in the tx buffer
	uart.txDataCount++;		//increment the data count
	if(uart.txBuffIndex >= TX_PACKET_SIZE)	//check for overflow
	{
		uart.txBuffIndex = 0;
	}

	//ENABLE_UART_TX_INTERRUPT();	
	//PIE1bits.TXIE = 1;	//Enable the transmit interrupt 
	return TRUE;
}


/*
*------------------------------------------------------------------------------
* BOOL UART1_transmit(void)
*
* Summary	: Fill the transmit buffer of the uart with data
* Input		: data to be transmitted which is stored in the txBuff
*
* Output	: Boolean indicating success or failure
* 
*------------------------------------------------------------------------------
*/
BOOL UART_transmit(void)
{
	UINT8 i,data;
//	TX_EN = 1;	//enable the tx control for rs485 communication
	DelayMs(1);
	while(uart.txDataCount > 0 )
	{
		
		data = uart.txBuff[uart.txDataIndex];

		while (!TXSTA1bits.TRMT);	//check whether ready to transmit
		TXREG1=data;					//load new data for transmission
	
		while (!TXSTA1bits.TRMT);	//check whether ready to transmit


		uart.txDataIndex++;		//increase the dataIndex to point to the next data to be transmitted
		if( uart.txDataIndex >= TX_PACKET_SIZE)		//on rollover
		{
			uart.txDataIndex = 0;		//reset
		}
		if( uart.txDataCount > 0)
			uart.txDataCount--;		//decrement the data count to indicate transmission
		
	}
//	TX_EN = 0;	//disable the tx control for rs485 communication
	DelayMs(1);
	

}	

/*
*------------------------------------------------------------------------------
BOOL UART_hasData(void)
*
* Summary	: indicates whether data has been received from the uart
* Input		: None
*
* Output	: the data from the rxBuff at index = rxDataIndex
* 
*------------------------------------------------------------------------------
*/


BOOL UART_hasData(void)
{
	BOOL result = FALSE;

//	DISABLE_UART_RX_INTERRUPT();
	PIE1bits.RC1IE = 0;		//disable the Receive interrupt 
	if(uart.rxDataCount > 0)
		result = TRUE; 
//	ENABLE_UART_RX_INTERRUPT();
	PIE1bits.RC1IE = 1;		//Enable the Receive interrupt 

		
	return result;
}




/*
*------------------------------------------------------------------------------
UINT8 UART_read(void)
*
* Summary	: return the data from the rxBuff 
* Input		: None
*
* Output	: the data from the rxBuff at index = rxDataIndex
* 
*------------------------------------------------------------------------------
*/
UINT8 UART_read(void)
{
	UINT8 data = 0;

//	DISABLE_UART_RX_INTERRUPT();
	PIE1bits.RC1IE = 0;		//disable the Receive interrupt 

	data = uart.rxBuff[uart.rxDataIndex++];
	if( uart.rxDataIndex >= RX_PACKET_SIZE)
	{
		uart.rxDataIndex = 0;
	}

	if( uart.rxDataCount > 0 )
		--uart.rxDataCount;

//	ENABLE_UART_RX_INTERRUPT();
	PIE1bits.RC1IE = 1;		//Enable the Receive interrupt 


	return data;
}





#if 0

/*
*------------------------------------------------------------------------------
* void PutrsUART(const rom char *data)
*
* Summary	: Send the rom character string through UART
*
* Input		: const rom char *data - pointer to the null terminated string rom.
*
* Output	: None
*------------------------------------------------------------------------------
*/
void PutrsUART(const rom char *data)
{
//	TX_EN = 1;	//enable the tx control for rs485 communication
	Delay10us(1);
  do
  {
	// Wait till the last bit trasmision
    while(BusyUSART());
	// Transmit a byte
    putcUSART(*data++);
  } while( *data != '\0');

//	TX_EN = 0;	//disable the tx control for rs485 communication
	Delay10us(1);
}

/*
*------------------------------------------------------------------------------
* void PutrsUARTRam(const UINT8 *data)
*
* Summary	: Send the ram character string through UART
*
* Input		: const UINT8 *data - pointer to the null terminated string in ram.
*
* Output	: None
*------------------------------------------------------------------------------
*/
void PutrsUARTRam(const UINT8 *data)
{
//	TX_EN = 1;	//enable the tx control for rs485 communication
	Delay10us(1);
  do
  {
	// Wait till the last bit trasmision
    while(BusyUSART());
	// Transmit a byte
    putcUSART(*data++);
  } while( *data != '\0');

//	TX_EN = 0;	//disable the tx control for rs485 communication
	Delay10us(1);

}

/*
*------------------------------------------------------------------------------
* void PutcUARTRam(const UINT8 data)
*
* Summary	: Send the ram character through UART
*
* Input		: const char *data - pointer to the null terminated string in ram.
*
* Output	: None
*------------------------------------------------------------------------------
*/
void PutcUARTRam(const UINT8 data)
{
/*
	// Wait till the last bit trasmision
    while(BusyUSART());
	TX_EN = 1;	//enable the tx control for rs485 communication
	DelayMs(10);

	// Transmit a byte
    putcUSART(data);

	while(BusyUSART());
	TX_EN = 0;	//disable the tx control for rs485 communication
	DelayMs(10);

*/

	
	

}

#endif

/*
*  End of uart_driver.c
*/
