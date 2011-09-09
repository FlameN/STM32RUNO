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
#include "task.h"
#include "queue.h"
#include "error.h"
#include "spi.h"

AT91PS_SPI SPI = AT91C_BASE_SPI;
xQueueHandle SPI_QUEUE;

/* Interrupt entry point written in the assembler file serialISR.s79. */
void AT91_SPI_ISR_ENTRY( void )  __attribute__((naked));
void AT91_SPI_ISR(void);
//*------------------------- Internal Function --------------------------------
//===============================================================================================
void AT91_SPI_ISR_ENTRY( void )
{
        portSAVE_CONTEXT();
        AT91_SPI_ISR();
        portRESTORE_CONTEXT();
}
void AT91_SPI_ISR(void)
{
        unsigned int size;
        unsigned int status;
        status = SPI->SPI_SR &= SPI->SPI_IMR;
        portBASE_TYPE xTaskWokenByPost = pdFALSE;
        size = (unsigned int)SPI->SPI_TCR;
        xQueueSendFromISR( SPI_QUEUE, &size, &xTaskWokenByPost );
        AT91C_BASE_SPI->SPI_IDR = AT91C_SPI_TXEMPTY ;
        if( xTaskWokenByPost)
        {
                portYIELD_FROM_ISR();
        }
        AT91C_BASE_AIC->AIC_EOICR = 0;
}

void AT91F_SPI_TRANSACTION_BLOCK (SPI_PCD_DATA *data,int size)
{
  unsigned int Param = 0;
  portENTER_CRITICAL();
    {
        SPI->SPI_TNCR = 0;
        SPI->SPI_RNCR = 0;
        SPI->SPI_RCR  = 0;
        SPI->SPI_TCR  = 0 ;
        SPI->SPI_RPR  = (unsigned int) data;
        SPI->SPI_TPR  = (unsigned int) data;
        SPI->SPI_RCR  = size;
        SPI->SPI_TCR  = size;
    }
  portEXIT_CRITICAL();
  AT91F_SPI_EnableIt(AT91C_BASE_SPI,AT91C_SPI_TXEMPTY);
  xQueueReceive( SPI_QUEUE, &( Param ), ( portTickType ) 1000 );
}
//===============================================================================================
//===============================================================================================
void SPI_Init ( void )
{
  SPI_QUEUE = xQueueCreate( 1, sizeof( unsigned int ) );
  while( SPI_QUEUE == 0 ) ;    // Queue was not created and must not be used.

  portENTER_CRITICAL();
  {
   AT91PS_SPI pSpi = AT91C_BASE_SPI;

   AT91F_SPI_CfgPMC();
   AT91F_SPI_CfgPIO();
   AT91F_SPI_Enable(pSpi);
   AT91F_SPI_CfgMode(pSpi,AT91C_SPI_MSTR | AT91C_SPI_PS_VARIABLE |/* AT91C_SPI_PCSDEC |*/(AT91C_SPI_DLYBCS & (15<<24)));
   pSpi->SPI_CSR[0] =
   pSpi->SPI_CSR[1] =
   pSpi->SPI_CSR[2] =
   pSpi->SPI_CSR[3] = AT91C_SPI_NCPHA |
                //   AT91C_SPI_CPOL |
                   (AT91C_SPI_BITS & AT91C_SPI_BITS_16)|
                    SPI_BAUD_RATE| SPI_DLYBS |
                   (AT91C_SPI_DLYBCT & (15 << 24));

  // pSpi->SPI_CSR[3] = AT91C_SPI_NCPHA |
    //                // AT91C_SPI_CPOL |
      //               (AT91C_SPI_BITS & AT91C_SPI_BITS_16)|
        //              SPI_BAUD_RATE| SPI_DLYBS |
          //           (AT91C_SPI_DLYBCT & (15 << 24));

   //
   //* Initialize the SPI Interrupt
   AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_SPI, SPI_INTERRUPT_LEVEL,AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE, AT91_SPI_ISR_ENTRY);
   AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_SPI);

   AT91F_PDC_Open(AT91C_BASE_PDC_SPI);
   //  data_temp = pSpi->SPI_RDR;
  }
  portEXIT_CRITICAL();
}









