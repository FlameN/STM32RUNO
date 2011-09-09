//*--------------------------------------------------------------------------------------
//*      ATMEL Microcontroller Software Support  -  ROUSSET  -
//*--------------------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*--------------------------------------------------------------------------------------
//* File Name           : twi.c
//* Object              : Basic TWI EEPROM driver
//* Translator          :
//* 1.0 25/11/02 NL		: Creation
//* 1.1 31/Jan/05 JPP   : Clean For basic
//*--------------------------------------------------------------------------------------

// Include Standard LIB  files
#include "FreeRTOS.h"
#include "Board.h"
#include "Main.h"
#include "twi.h"
#include "Task.h"
#include "queue.h"
#include "Semphr.h"

#include "Error.h"


#define TWI_INTERRUPT_LEVEL	7
#define AT91C_TWI_CLOCK 	200000
#define AT91C_TWI_TMO_VOL(s)    ( 2000 )
//#define AT91C_TWI_TMO_VOL(s)    ( ( (20*s) / AT91C_TWI_CLOCK )*1000 + 100)


AT91PS_TWI TWI = AT91C_BASE_TWI;

/* Interrupt entry point written in the assembler file serialISR.s79. */
extern void AT91_TWI_ISR_ENTRY( void );
__arm void AT91_TWI_ISR(void);


volatile xQueueHandle TWI_QUEUE;
xSemaphoreHandle TWI_Semaphore = NULL;

volatile ErrorHandle TWI_NACK_Error,TWI_TMO_Error,TWI_SEM_Error,TWI_WriteData_Error,TWI_OVRE_Error;

 typedef enum
 {
  FREE,
  ON_READ_DATA,
  ON_WRITE_DATA
 }TWI_TRANSFER_STATUS;


 volatile TWI_TRANSFER_STATUS TWI_TransferStatus = FREE;

volatile unsigned int TWI_Bytes2Transfere;
volatile unsigned int TWI_ConitiosTransaction = 0;
volatile unsigned int TWI_DeviceAddr;

volatile unsigned char *TWI_DataPointer;


#define ERROR (AT91C_TWI_NACK)

void AT91F_SetTwiClock(void)
{
  int sclock;
	/* Here, CKDIV = 1 and CHDIV=CLDIV  ==> CLDIV = CHDIV = 1/4*((Fmclk/FTWI) -6)*/
	sclock = (10*MCK /AT91C_TWI_CLOCK);
	if (sclock % 10 >= 5)
		sclock = (sclock /10) - 5;
	else
		sclock = (sclock /10)- 6;
	sclock = (sclock + (4 - sclock %4)) >> 2;	// div 4
        AT91C_BASE_TWI->TWI_CWGR = ( 1<<16 ) | (sclock << 8) | sclock  ;
}

void RESET_TWI()
{
  uint8 data = AT91C_BASE_TWI->TWI_RHR;
  uint16 status = AT91C_BASE_TWI->TWI_SR;
 // Configure TWI in master mode
	 AT91F_TWI_Configure (AT91C_BASE_TWI);
         // Configure TWI PIOs
	 AT91F_TWI_CfgPIO();
      	 AT91F_PIO_CfgOpendrain(AT91C_BASE_PIOA, // PIO controller base address
    		                ((unsigned int) AT91C_PA1_TWCK    ) |
      		                ((unsigned int) AT91C_PA0_TWD  )  );

         // Configure PMC by enabling TWI clock
	 AT91F_TWI_CfgPMC ();
    	 // AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1<<AT91C_ID_TWI ) ;
	 // Set TWI Clock Waveform Generator Register
 	 AT91F_SetTwiClock();
         AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC,
                                AT91C_ID_TWI,
                                TWI_INTERRUPT_LEVEL,
                                AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE,
                                ( void (*)( void ) )AT91_TWI_ISR_ENTRY);

  	 AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TWI);
        TWI_TransferStatus = FREE;



}




//*=========================================================
//*		WRITE
//*=========================================================
//*----------------------------------------------------------------------------
//* \fn    AT91F_TWI_Write
//* \brief Send n bytes to a slave device
//*----------------------------------------------------------------------------
int AT91F_TWI_Write(unsigned int devaddress,unsigned int address,unsigned char *data2send, int size)
{
 unsigned int SizeNotWrited = 0;
//---------------------------------------------------------
 if( TWI_Semaphore != NULL )
    {
      if( xSemaphoreTake( TWI_Semaphore, ( portTickType ) 2000 ) == pdTRUE )
      {
        TWI_TransferStatus = ON_WRITE_DATA;
        SizeNotWrited = 0;
        TWI_DataPointer = data2send;
        TWI_Bytes2Transfere = size;
        TWI_DeviceAddr = devaddress;
        TWI_TransferStatus = ON_WRITE_DATA;
taskDISABLE_INTERRUPTS();
        AT91C_BASE_TWI->TWI_MMR = ( TWI_DeviceAddr | AT91C_TWI_IADRSZ_2_BYTE ) & ~AT91C_TWI_MREAD;	
        AT91C_BASE_TWI->TWI_IADR = address;
	AT91C_BASE_TWI->TWI_THR = *(TWI_DataPointer++);
        AT91F_TWI_EnableIt(AT91C_BASE_TWI,AT91C_TWI_TXRDY | AT91C_TWI_NACK);
        AT91C_BASE_TWI->TWI_CR = AT91C_TWI_START ;
taskENABLE_INTERRUPTS();
        xQueueReceive( TWI_QUEUE, &( SizeNotWrited ), ( portTickType ) AT91C_TWI_TMO_VOL(size));
        if( TWI_TransferStatus != FREE)
         {
          // SYSTEM_ERROR(TWI_TMO_Error);
           RESET_TWI();
         };
        TWI_TransferStatus = FREE;

        xSemaphoreGive( TWI_Semaphore );
      }
        else
        {
         //SYSTEM_ERROR(TWI_SEM_Error);
        }
    }
 return SizeNotWrited;
}
//*=========================================================
//*		READ
//*=========================================================
//*----------------------------------------------------------------------------
//* \fn    AT91F_TWI_Read
//* \brief Read n bytes from a slave device
//*----------------------------------------------------------------------------
int AT91F_TWI_Read(unsigned int devaddress,unsigned int address,unsigned char *data,int size)
{	
  unsigned int SizeReaded;
  volatile unsigned int status;
  if( TWI_Semaphore != NULL )
    {
      if( xSemaphoreTake( TWI_Semaphore, ( portTickType ) 2000 ) == pdTRUE )
      {
        SizeReaded = 0;
        AT91PS_TWI TWI = AT91C_BASE_TWI;
        TWI_DataPointer = data;
        TWI_Bytes2Transfere = size;
        TWI_DeviceAddr = devaddress;
	if(TWI_Bytes2Transfere>1)TWI_ConitiosTransaction = 1;
taskDISABLE_INTERRUPTS();
	TWI->TWI_IADR = address;
	TWI->TWI_MMR = TWI_DeviceAddr | AT91C_TWI_IADRSZ_2_BYTE | AT91C_TWI_MREAD;	
        TWI_TransferStatus = ON_READ_DATA;
        AT91F_TWI_EnableIt(TWI, AT91C_TWI_TXCOMP | AT91C_TWI_NACK | AT91C_TWI_OVRE | AT91C_TWI_UNRE);
        TWI->TWI_CR = AT91C_TWI_START | AT91C_TWI_MSEN | AT91C_TWI_STOP ;
        status = TWI->TWI_SR;
taskENABLE_INTERRUPTS();
        xQueueReceive( TWI_QUEUE, &( SizeReaded ), ( portTickType ) AT91C_TWI_TMO_VOL(size) );

   if( TWI_TransferStatus != FREE)
         {
         //  SYSTEM_ERROR(TWI_TMO_Error);
           RESET_TWI();

         } ;
        TWI_TransferStatus = FREE;
        xSemaphoreGive( TWI_Semaphore );
      }
        else
        {
         //SYSTEM_ERROR(TWI_SEM_Error);
        }
    }
  return SizeReaded;
}

int AT91F_TWI_WriteByteAddress(unsigned int devaddress,unsigned int address,unsigned char *data2send, int size)
{
  unsigned int SizeNotWrited;
  if( TWI_Semaphore != NULL )
    {
      if( xSemaphoreTake( TWI_Semaphore, ( portTickType ) 2000 ) == pdTRUE )
      {
        SizeNotWrited = 0;
        TWI_DataPointer = data2send;
        TWI_Bytes2Transfere = size;
        TWI_DeviceAddr = devaddress;
        TWI_TransferStatus = ON_WRITE_DATA;
taskDISABLE_INTERRUPTS();
        AT91C_BASE_TWI->TWI_MMR = ( TWI_DeviceAddr | AT91C_TWI_IADRSZ_1_BYTE ) & ~AT91C_TWI_MREAD;	
	AT91C_BASE_TWI->TWI_IADR = address;
	AT91C_BASE_TWI->TWI_THR = *(TWI_DataPointer++);
        AT91F_TWI_EnableIt(AT91C_BASE_TWI,AT91C_TWI_TXRDY | AT91C_TWI_NACK);
        AT91C_BASE_TWI->TWI_CR = AT91C_TWI_START ;
taskENABLE_INTERRUPTS();
        xQueueReceive( TWI_QUEUE, &( SizeNotWrited ), ( portTickType ) AT91C_TWI_TMO_VOL(size) );
        if( TWI_TransferStatus != FREE)
        {
         // SYSTEM_ERROR(TWI_TMO_Error);
           RESET_TWI();
        };
        TWI_TransferStatus = FREE;
        xSemaphoreGive( TWI_Semaphore );
      }
        else
        {

        //  SYSTEM_ERROR(TWI_SEM_Error);
         TWI_TransferStatus = FREE;
        }
    }
   return SizeNotWrited;
}
//*=========================================================
//*		READ
//*=========================================================
//*----------------------------------------------------------------------------
//* \fn    AT91F_TWI_Read
//* \brief Read n bytes from a slave device
//*----------------------------------------------------------------------------
int AT91F_TWI_ReadByteAddress(unsigned int devaddress,unsigned int address,unsigned char *data, int size)
{
 unsigned int SizeReaded,status;
 if( TWI_Semaphore != NULL )
    {
      if( xSemaphoreTake( TWI_Semaphore, ( portTickType ) 2000 ) == pdTRUE )
      {
        SizeReaded = 0;

        TWI_DataPointer = data;
        TWI_Bytes2Transfere = size;
        TWI_DeviceAddr = devaddress;
	if(TWI_Bytes2Transfere>1)TWI_ConitiosTransaction = 1;
taskDISABLE_INTERRUPTS();
        TWI->TWI_IADR = address;
	TWI->TWI_MMR = TWI_DeviceAddr | AT91C_TWI_IADRSZ_1_BYTE | AT91C_TWI_MREAD;	
        TWI_TransferStatus = ON_READ_DATA;
        AT91F_TWI_EnableIt(TWI, AT91C_TWI_TXCOMP/* | AT91C_TWI_RXRDY*/|AT91C_TWI_NACK );
        status = TWI->TWI_SR;
        TWI->TWI_CR = AT91C_TWI_START | AT91C_TWI_MSEN | AT91C_TWI_STOP ;
taskENABLE_INTERRUPTS();
        xQueueReceive( TWI_QUEUE, &( SizeReaded ), ( portTickType ) AT91C_TWI_TMO_VOL(size) );
        if( TWI_TransferStatus != FREE)
        {
         // SYSTEM_ERROR(TWI_TMO_Error);
          RESET_TWI();
        };
        TWI_TransferStatus = FREE;
        xSemaphoreGive( TWI_Semaphore );
      }
        else
        {
       //  SYSTEM_ERROR(TWI_SEM_Error);
        }
    }
 return SizeReaded;
}

//*=========================================================
//*		INIT
//*=========================================================
//*----------------------------------------------------------------------------
//* \fn    AT91F_SetTwiClock
//* \brief Initialization
//*----------------------------------------------------------------------------
//===============================================================================================
__arm void AT91_TWI_ISR(void)
{
  unsigned int status;
  volatile int SizeNotWrited;
  portBASE_TYPE xTaskWokenByTx = pdFALSE, xTaskWokenByPost = pdFALSE;
  // What caused the interrupt?
  status = AT91C_BASE_TWI->TWI_SR &= AT91C_BASE_TWI->TWI_IMR;
  SizeNotWrited = SizeNotWrited;
  if(status & AT91C_TWI_OVRE)
  {
      AT91F_TWI_DisableIt(AT91C_BASE_TWI,AT91C_TWI_TXRDY | AT91C_TWI_NACK | AT91C_TWI_TXCOMP|AT91C_TWI_RXRDY );
      AT91C_BASE_TWI->TWI_CR = AT91C_TWI_STOP;
     // SYSTEM_ERROR(TWI_OVRE_Error);
      xTaskWokenByPost = xQueueSendFromISR( TWI_QUEUE, &TWI_Bytes2Transfere, xTaskWokenByPost );
      TWI_TransferStatus = FREE;
  }else
  if (status & AT91C_TWI_NACK)
   {
      AT91F_TWI_DisableIt(AT91C_BASE_TWI,AT91C_TWI_TXRDY | AT91C_TWI_NACK | AT91C_TWI_TXCOMP|AT91C_TWI_RXRDY );
      AT91C_BASE_TWI->TWI_CR = AT91C_TWI_STOP;
     // SYSTEM_ERROR(TWI_NACK_Error);
      xTaskWokenByPost = xQueueSendFromISR( TWI_QUEUE, &TWI_Bytes2Transfere, xTaskWokenByPost );
      TWI_TransferStatus = FREE;
   }else
     {
       switch(TWI_TransferStatus)
       {
       case ON_READ_DATA:
          if(status & AT91C_TWI_TXCOMP)
             { // next char can be read
               if(TWI_Bytes2Transfere-- > 1)
                 {
                     *(TWI_DataPointer++) = AT91C_BASE_TWI->TWI_RHR;

                    if(TWI_ConitiosTransaction)
                     {
                       TWI_ConitiosTransaction = 0;
                       TWI->TWI_MMR = TWI_DeviceAddr | AT91C_TWI_IADRSZ_NO | AT91C_TWI_MREAD;	
                      // AT91F_TWI_EnableIt(TWI, AT91C_TWI_TXCOMP | AT91C_TWI_NACK );
                     //  AT91C_BASE_TWI->TWI_CR = AT91C_TWI_START | AT91C_TWI_MSEN | AT91C_TWI_STOP ;
                     }
                     AT91C_BASE_TWI->TWI_CR = AT91C_TWI_START | AT91C_TWI_MSEN | AT91C_TWI_STOP;

                 } else
                     {
                       *(TWI_DataPointer++) = AT91C_BASE_TWI->TWI_RHR;
                       AT91F_TWI_DisableIt(AT91C_BASE_TWI,AT91C_TWI_TXCOMP|AT91C_TWI_NACK);
                       AT91C_BASE_TWI->TWI_CR = AT91C_TWI_STOP;
                      xTaskWokenByPost = xQueueSendFromISR( TWI_QUEUE, &TWI_Bytes2Transfere, xTaskWokenByPost );
                       TWI_TransferStatus = FREE;
                      //TWI_TransferStatus = FREE;
                     }
              }else
                {
                  AT91C_BASE_TWI->TWI_CR = AT91C_TWI_STOP;
                }

            //-----------------------------
         break;
       case ON_WRITE_DATA:
               if(status& AT91C_TWI_TXRDY)
             { // next char can be write
               if(TWI_Bytes2Transfere-- > 1)
                 {
                     AT91C_BASE_TWI->TWI_THR = *(TWI_DataPointer++) ;
                 } else
                     {
                       AT91F_TWI_DisableIt(AT91C_BASE_TWI,AT91C_TWI_TXRDY|AT91C_TWI_NACK);
                       AT91C_BASE_TWI->TWI_CR = AT91C_TWI_STOP;
                       xTaskWokenByPost = xQueueSendFromISR( TWI_QUEUE, &TWI_Bytes2Transfere, xTaskWokenByPost );
                       TWI_TransferStatus = FREE;
                     }
              }
//              TWI_TransferStatus = FREE;
            //-----------------------------
         break;
       default:
            AT91F_TWI_DisableIt(AT91C_BASE_TWI,AT91C_TWI_RXRDY|AT91C_TWI_TXCOMP|AT91C_TWI_NACK);
            xTaskWokenByPost = xQueueSendFromISR( TWI_QUEUE, &TWI_Bytes2Transfere, xTaskWokenByPost );
            TWI_TransferStatus = FREE;
       }
     }
  portEND_SWITCHING_ISR( ( xTaskWokenByPost || xTaskWokenByTx ) );
  AT91F_AIC_AcknowledgeIt(AT91C_BASE_AIC);
  // AT91C_BASE_AIC->AIC_EOICR = 0;
}

//===============================================================================================
//*----------------------------------------------------------------------------
//* \fn    AT91F_TWI_Open
//* \brief Initializes TWI device
//*----------------------------------------------------------------------------
void AT91F_TWI_Init(void)
{


        TWI_QUEUE = xQueueCreate( 1, sizeof( unsigned int ) );
        TWI_NACK_Error = ErrorCreate("TWI Nack");
        TWI_TMO_Error  = ErrorCreate("TWI TimeOut");
        TWI_SEM_Error  = ErrorCreate("TWI Access");
        TWI_WriteData_Error = ErrorCreate("TWI Write");
        TWI_OVRE_Error = ErrorCreate("TWI OVRE");
         vSemaphoreCreateBinary( TWI_Semaphore );
        while( TWI_QUEUE == 0 ) ;    // Queue was not created and must not be used.
        portENTER_CRITICAL();
        {
 	 // Configure TWI in master mode
	 AT91F_TWI_Configure (AT91C_BASE_TWI);
         // Configure TWI PIOs
      	 AT91F_PIO_CfgOutput(AT91C_BASE_PIOA, AT91C_PA1_TWCK );
         AT91F_PIO_CfgOpendrain(AT91C_BASE_PIOA, // PIO controller base address
    		                ((unsigned int) AT91C_PA1_TWCK    ) |
      		                ((unsigned int) AT91C_PA0_TWD  )  );

         AT91F_TWI_CfgPIO();

         // Configure PMC by enabling TWI clock
	 AT91F_TWI_CfgPMC ();
    	 // AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1<<AT91C_ID_TWI ) ;
	 // Set TWI Clock Waveform Generator Register
 	 AT91F_SetTwiClock();
         AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC,
                                AT91C_ID_TWI,
                                TWI_INTERRUPT_LEVEL,
                                AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE,
                                ( void (*)( void ) )AT91_TWI_ISR_ENTRY);

  	 AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TWI);
        TWI_TransferStatus = FREE;
        }
        portEXIT_CRITICAL();
}


