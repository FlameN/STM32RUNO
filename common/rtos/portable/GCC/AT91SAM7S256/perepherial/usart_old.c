//*----------------------------------------------------------------------------
//*      ATMEL Microcontroller Software Support  -  ROUSSET  -
//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : interrupt_Usart.c
//* Object              : USART Interrupt Management
//*
//* 1.0 03/Jun/03 JPP   : Creation
//*----------------------------------------------------------------------------

// Include Standard LIB  files
#include "FreeRTOS.h"
#include "board.h"
#include "usart.h"
#include "task.h"
#include "queue.h"
#include "error.h"
//#include "lib_AT91SAM7S.h"

#define TRACE

#define AT91F_US_TIMEOUT(X,VOL) {X->US_CR |= AT91C_US_STTTO ;	X->US_RTOR = VOL;}
#define AT91F_US_ENRXTX(X)      {X->US_CR  = AT91C_US_RXEN | AT91C_US_TXEN;}

#define USART_INTERRUPT_LEVEL		7

AT91PS_USART COM0 = AT91C_BASE_US0;
AT91PS_USART COM1 = AT91C_BASE_US1;

#define USART_BAUD_RATE 		115200

/* Interrupt entry point written in the assembler file serialISR.s79. */
void AT91_US0_ISR_ENTRY( void )  __attribute__((naked));
void AT91_US1_ISR_ENTRY( void )  __attribute__((naked));

void AT91_US0_ISR(void);
void AT91_US1_ISR(void);

xQueueHandle US0_QUEUE,US1_QUEUE;
ErrorHandle  US0_Error;
ErrorHandle  US1_Error;

//*------------------------- Internal Function --------------------------------
void AT91_US0_ISR_ENTRY( void )
{
	portSAVE_CONTEXT();
	AT91_US0_ISR();
	portRESTORE_CONTEXT();
}
void AT91_US1_ISR_ENTRY( void )
{
	portSAVE_CONTEXT();
	AT91_US1_ISR();
	portRESTORE_CONTEXT();
}
//===============================================================================================
__inline void AT91F_US_INTERRUPT(AT91PS_USART COM,xQueueHandle *pxQueue,ErrorHandle *pxError)
{
        unsigned int status;
        unsigned int size;
        portBASE_TYPE  xTaskWokenByPost = pdFALSE;
         /* What caused the interrupt? */
	    status = COM->US_CSR &= COM->US_IMR;
        if (status & (AT91C_US_RXBUFF | AT91C_US_FRAME | AT91C_US_PARE |AT91C_US_OVRE))
        {
           AT91F_US_DisableIt(COM, (status & (AT91C_US_RXBUFF | AT91C_US_FRAME | AT91C_US_PARE |AT91C_US_OVRE)));
        }else  {
                  if ((status & AT91C_US_TIMEOUT ))
                  {
                   AT91F_US_DisableIt(COM, AT91C_US_TIMEOUT | AT91C_US_RXBUFF );
                   AT91F_US_DisableRx(COM);
                   size = (unsigned int)COM->US_RCR;
                   xQueueSendFromISR( *pxQueue, &size, &xTaskWokenByPost );
                  }
               }
          if (status & AT91C_US_ENDTX)
               { // Data transmitted
                 AT91F_US_DisableIt(COM, AT91C_US_ENDTX );
                 AT91F_US_DisableTx(COM);
                 AT91F_US_EnableIt(COM, AT91C_US_TXEMPTY );
               }
          if (status & AT91C_US_TXEMPTY)
          {
                 size = (unsigned int)COM->US_TCR;
                 xQueueSendFromISR( *pxQueue, &size, &xTaskWokenByPost );
                 AT91F_US_DisableIt(COM, AT91C_US_TXEMPTY );
          }
        if( xTaskWokenByPost)
        {
        	portYIELD_FROM_ISR();
        }
        AT91C_BASE_AIC->AIC_EOICR = 0;
}

//===============================================================================================
__inline AT91F_US_START_TRANSMITION(AT91PS_USART COM, uint8 *data, int size)
{
        portENTER_CRITICAL();
        {
         AT91F_US_EnableTx(COM);
         AT91F_US_EnableIt(COM,AT91C_US_TXEMPTY | AT91C_US_TXEN );
         COM->US_TCR = 0;
	 COM->US_TNCR = 0;
         COM->US_TPR = (unsigned int) data;
         COM->US_TCR = size;
        }
        portEXIT_CRITICAL();
}
//===============================================================================================
__inline AT91F_US_START_RECEIVE(AT91PS_USART COM, uint8 *data, int size)
{
        portENTER_CRITICAL();
        {
         AT91F_US_TIMEOUT(COM,0x40);
         AT91F_US_EnableIt(COM,AT91C_US_RXBUFF | AT91C_US_TIMEOUT | AT91C_US_RXBUFF | AT91C_US_FRAME | AT91C_US_PARE |AT91C_US_OVRE);
         AT91F_US_EnableRx(COM);
         COM->US_RCR = 0;
	 COM->US_RNCR = 0;
         COM->US_RPR = (unsigned int) data;
         COM->US_RCR = size;
        }
        portEXIT_CRITICAL();
}
//===============================================================================================
void AT91_US0_ISR(void)
{
    AT91F_US_INTERRUPT(COM0,&US0_QUEUE,&US0_Error);
}
//===============================================================================================
void AT91_US1_ISR(void)
{
   AT91F_US_INTERRUPT(COM1,&US1_QUEUE,&US1_Error);
}
//===============================================================================================
void AT91F_US0_TRANSMITT_BLOCK (uint8 *data,int size)
{
        unsigned int Param;
        AT91F_US_START_TRANSMITION(COM0,data,size);
        xQueueReceive( US0_QUEUE, &( Param ), ( portTickType ) 1000 );
}
//===============================================================================================
int AT91F_US0_RECEIVE_BLOCK (uint8 *data,int size)
{
        unsigned int rxSize;
        AT91F_US_START_RECEIVE(COM0,data,size);
        xQueueReceive( US0_QUEUE, &( rxSize ), ( portTickType ) 1000 );
        return rxSize;
}
//===============================================================================================
void AT91F_US1_TRANSMITT_BLOCK (uint8 *data,int size)
{
        unsigned int Param;
        AT91F_US_START_TRANSMITION(COM1,data,size);
        xQueueReceive( US1_QUEUE, &( Param ), ( portTickType ) 1000 );
}
//===============================================================================================
int AT91F_US1_RECEIVE_BLOCK (uint8 *data,int size)
{
        unsigned int rxSize;
        AT91F_US_START_RECEIVE(COM1,data,size);
        xQueueReceive( US1_QUEUE, &( rxSize ), ( portTickType ) 1000 );
        return rxSize;
}
void Usart0_init ( long BaudRate )
{
    US0_QUEUE = xQueueCreate( 10, sizeof( unsigned int ) );
    US0_Error = ErrorCreate("US0_Error");
    while( US0_QUEUE == 0 ) ;    // Queue was not created and must not be used.
        portENTER_CRITICAL();
        {
         AT91F_US0_CfgPIO() ;        //* Define RXD and TXD as peripheral
      	 AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1<<AT91C_ID_US0 ) ;       // First, enable the clock of the PIOB
	AT91F_US_Configure (COM0, MCK,AT91C_US_ASYNC_MODE, BaudRate , 0);        // Usart Configure
	AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC,
                                AT91C_ID_US0,
                                USART_INTERRUPT_LEVEL,
                                AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL,
                                ( void (*)( void ) )AT91_US0_ISR_ENTRY);
 	AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_US0);
        }
        portEXIT_CRITICAL();
}
//===============================================================================================
void Usart1_init ( long BaudRate )
{
    US1_QUEUE = xQueueCreate( 10, sizeof( unsigned int ) );
    US1_Error = ErrorCreate("US1_Error");
    while( US1_QUEUE == 0 ) ;    // Queue was not created and must not be used.
        portENTER_CRITICAL();
        {
        AT91F_US1_CfgPIO() ;        //* Define RXD and TXD as peripheral
    	AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1<<AT91C_ID_US1 ) ;        // First, enable the clock of the PIOB
	AT91F_US_Configure (COM1, MCK,AT91C_US_ASYNC_MODE, BaudRate , 0);        // Usart Configure
        AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC,
                                AT91C_ID_US1,
                                USART_INTERRUPT_LEVEL,
                                AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL,
                                ( void (*)( void ) )AT91_US1_ISR_ENTRY);

 	AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_US1);
        }
        portEXIT_CRITICAL();
}




















