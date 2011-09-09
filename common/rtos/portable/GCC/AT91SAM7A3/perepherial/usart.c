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
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "board.h"
#include "error.h"
#include "usart.h"
#define TRACE

#define AT91F_US_TIMEOUT_SET(X,VOL) {X->US_CR |= AT91C_US_STTTO ;	X->US_RTOR = VOL;}
//#define AT91F_US_ENRXTX(X)      {X->US_CR  = AT91C_US_RXEN | AT91C_US_TXEN;}
#define USART_INTERRUPT_LEVEL		7

AT91_USART_DEV COM0, COM1, COM2;
volatile int COM0_RX;

/* Interrupt entry point written in the assembler file serialISR.s79. */
void AT91_US0_ISR_ENTRY(void) __attribute__((naked));

void AT91_US1_ISR_ENTRY(void) __attribute__((naked));
void AT91_US2_ISR_ENTRY(void) __attribute__((naked));

void AT91F_US_INTERRUPT(AT91_USART_DEV*);
void AT91F_US_INTERRUPT_COM0(void);

//*------------------------- Internal Function --------------------------------
void AT91_US0_ISR_ENTRY(void)
    {
    portSAVE_CONTEXT();
    AT91F_US_INTERRUPT_COM0();
    portRESTORE_CONTEXT();
    }
void AT91_US1_ISR_ENTRY(void)
    {
    portSAVE_CONTEXT();
    AT91F_US_INTERRUPT(&COM1);
    portRESTORE_CONTEXT();
    }
void AT91_US2_ISR_ENTRY(void)
    {
    portSAVE_CONTEXT();
    AT91F_US_INTERRUPT(&COM2);
    portRESTORE_CONTEXT();
    }
//===============================================================================================
void AT91F_US_INTERRUPT_COM0()
    {
    unsigned int status = COM0.hPort->US_CSR;
    int size;
    portBASE_TYPE xTaskWokenByPost = pdFALSE;

    status &= COM0.hPort->US_IMR;

    if ((status & AT91C_US_TIMEOUT))
	{
	// COM0.rxSize = COM0.rxSize - (int) COM0.hPort->US_RCR;
	AT91F_US_DisableIt(COM0.hPort, AT91C_US_TIMEOUT | AT91C_US_RXBUFF);
	AT91F_US_DisableRx(COM0.hPort);
	COM0_RX = COM0_RX - (int) COM0.hPort->US_RCR;
	xQueueSendFromISR(COM0.hRxQueue, (void *) (&size), &xTaskWokenByPost);

	}
    if ((status & AT91C_US_TXEMPTY))
	{
	//   size = (int) COM0.hPort->US_TCR;
	xQueueSendFromISR(COM0.hTxQueue, (void *) &size, &xTaskWokenByPost);
	AT91F_US_DisableIt(COM0.hPort, AT91C_US_TXEMPTY);
	AT91F_US_DisableTx(COM0.hPort);
	}

    if (xTaskWokenByPost)
	{
	portYIELD_FROM_ISR();
	}

    AT91C_BASE_AIC->AIC_EOICR = 0;
    }
void AT91F_US_INTERRUPT(AT91_USART_DEV *mdev)
    {
    unsigned int status = mdev->hPort->US_CSR;
    int size;
    portBASE_TYPE xTaskWokenByPost = pdFALSE;
    status &= mdev->hPort->US_IMR;

    if ((status & AT91C_US_TIMEOUT))
	{
	mdev->rxSize -= (int) mdev->hPort->US_RCR;
	xQueueSendFromISR(mdev->hRxQueue, (void *) (&size), &xTaskWokenByPost);
	AT91F_US_DisableIt(mdev->hPort, AT91C_US_TIMEOUT | AT91C_US_RXBUFF);
	AT91F_US_DisableRx(mdev->hPort);
	}
    if ((status & AT91C_US_TXEMPTY))
	{
	//     size = (int) mdev->hPort->US_TCR;
	xQueueSendFromISR(mdev->hTxQueue, (void *) &size, &xTaskWokenByPost);
	AT91F_US_DisableIt(mdev->hPort, AT91C_US_TXEMPTY);
	AT91F_US_DisableTx(mdev->hPort);
	}

    if (xTaskWokenByPost)
	{
	portYIELD_FROM_ISR();
	}

    AT91C_BASE_AIC->AIC_EOICR = 0;
    }

//===============================================================================================
inline void AT91F_US_START_TRANSMITION(AT91PS_USART COM, uint8 *data, int size)
    {
    AT91F_US_EnableIt(COM, AT91C_US_TXEMPTY);
    AT91F_US_EnableTx(COM);
    COM->US_TNCR = 0;
    COM->US_TNCR = 0;
    COM->US_TPR = (unsigned int) data;
    COM->US_TCR = size;
    }

//===============================================================================================
inline void AT91F_US_START_RECEIVE(AT91PS_USART COM, uint8 *data, int size)
    {
    portENTER_CRITICAL();
	{
	AT91F_US_TIMEOUT_SET(COM,0x20);
	AT91F_US_EnableIt(COM, AT91C_US_TIMEOUT | AT91C_US_RXBUFF);
	AT91F_US_EnableRx(COM);
	COM->US_RNCR = 0;
	COM->US_RNCR = 0;
	COM->US_RPR = (unsigned int) data;
	COM->US_RCR = size;
	}
    portEXIT_CRITICAL();

    }
//===============================================================================================
void AT91F_USART_SEND_COM0(uint8 *data, int size)
    {
    int Param;

    portENTER_CRITICAL();
	{
	AT91F_US_START_TRANSMITION(COM0.hPort, data, size);
	}
    portEXIT_CRITICAL();
    xQueueReceive(COM0.hTxQueue, (void *) &Param, (portTickType) 1000);
    AT91F_US_DisableIt(COM0.hPort, AT91C_US_TXEMPTY);
    AT91F_US_DisableTx(COM0.hPort);

    }

//===============================================================================================
void AT91F_USART_SEND(AT91_USART_DEV *dev, uint8 *data, int size)
    {
    int Param;
    dev->rxSize = 0;
    portENTER_CRITICAL();
	{
	AT91F_US_START_TRANSMITION(dev->hPort, data, size);
	}
    portEXIT_CRITICAL();

    xQueueReceive(dev->hTxQueue, (void *) &Param, (portTickType) 1000);
    AT91F_US_DisableIt(dev->hPort, AT91C_US_TXEMPTY);
    AT91F_US_DisableTx(dev->hPort);

    }
//===============================================================================================
int AT91F_USART_RECEIVE(AT91_USART_DEV *dev, uint8 *data, int dsize,
	int timeout)
    {
    dev->rxSize = dsize;
    AT91F_US_START_RECEIVE(dev->hPort, data, dsize);

    if (xQueueReceive(dev->hRxQueue, (void *) &dsize, (portTickType) timeout)
	    == pdFALSE)
	{
	dev->rxSize = 0;
	}
    AT91F_US_DisableIt(dev->hPort, AT91C_US_TIMEOUT);
    AT91F_US_DisableRx(dev->hPort);
    return (dev->rxSize);
    }
//===============================================================================================
int AT91F_USART_RECEIVE_COM0(uint8 *data, int dsize, int timeout)
    {
    int temp;
    COM0_RX = dsize;
    AT91F_US_START_RECEIVE(COM0.hPort, data, COM0_RX);
    if (xQueueReceive(COM0.hRxQueue, (void *) &dsize, (portTickType) timeout)
	    == pdFALSE)
	{
	COM0_RX = 0;
	}
    temp = COM0_RX;
    COM0_RX = 0;
    AT91F_US_DisableIt(COM0.hPort, AT91C_US_TIMEOUT);
    AT91F_US_DisableRx(COM0.hPort);
    return (temp);
    }

void AT91F_USART_OPEN(AT91_USART_ID idPort, long BaudRate, int mode)
    {
    portENTER_CRITICAL();
	{
	switch (idPort)
	    {
	case AT91_USART_COM0_ID:
	    COM0.id = idPort;
	    COM0.hPort = AT91C_BASE_US0;
	    COM0.hError = ErrorCreate("US0_Error");
	    COM0.hPDC = AT91C_BASE_PDC_US0;

	    AT91F_PDC_Open(COM0.hPDC);
	   // AT91F_US0_CfgPIO(); //* Define RXD and TXD as peripheral
	    AT91F_PIO_CfgPeriph(
	    		AT91C_BASE_PIOA, // PIO controller base address
	    		(((unsigned int) AT91C_PA2_RXD0    ) |
	    		((unsigned int) AT91C_PA3_TXD0    )), // Peripheral A
	    		0); // Peripheral B
	    AT91F_PMC_EnablePeriphClock(AT91C_BASE_PMC, 1 << AT91C_ID_US0); // First, enable the clock of the PIOB
	    AT91F_US_Configure(COM0.hPort, MCK, mode, BaudRate, 0); // Usart Configure

	    AT91F_AIC_ConfigureIt(AT91C_BASE_AIC, AT91C_ID_US0,
		    USART_INTERRUPT_LEVEL,
		    AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE,
		    (void(*)(void)) AT91_US0_ISR_ENTRY);

	    AT91F_AIC_EnableIt(AT91C_BASE_AIC, AT91C_ID_US0);
	    COM0.hRxQueue = xQueueCreate(1, sizeof(int));
	    while (COM0.hRxQueue == 0)
		; // Queue was not created and must not be used.
	    COM0.hTxQueue = xQueueCreate(1, sizeof(int));
	    while (COM0.hTxQueue == 0)
		; // Queue was not created and must not be used.

	    break;

	case AT91_USART_COM1_ID:
	    COM1.hPort = AT91C_BASE_US1;
	    COM1.hPDC = AT91C_BASE_PDC_US1;
	    COM1.hError = ErrorCreate("US1_Error");
	    AT91F_PDC_DisableTx(COM1.hPDC);
	    AT91F_PDC_DisableRx(COM1.hPDC);
	    AT91F_US1_CfgPIO(); //* Define RXD and TXD as peripheral
	    AT91F_PMC_EnablePeriphClock(AT91C_BASE_PMC, 1 << AT91C_ID_US1); // First, enable the clock of the PIOB
	    AT91F_US_Configure(COM1.hPort, MCK, mode, BaudRate, 0); // Usart Configure
	    AT91F_AIC_ConfigureIt(AT91C_BASE_AIC, AT91C_ID_US1,
		    USART_INTERRUPT_LEVEL,
		    AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE,
		    (void(*)(void)) AT91_US1_ISR_ENTRY);
	    AT91F_AIC_EnableIt(AT91C_BASE_AIC, AT91C_ID_US1);

	    COM1.hRxQueue = xQueueCreate(1, sizeof(int));
	    while (COM1.hRxQueue == 0)
		; // Queue was not created and must not be used.

	    COM1.hTxQueue = xQueueCreate(1, sizeof(int));
	    while (COM1.hTxQueue == 0)
		; // Queue was not created and must not be used.
	    break;
	case AT91_USART_COM2_ID:
	    COM2.hPort = AT91C_BASE_US2;
	    COM2.hPDC = AT91C_BASE_PDC_US2;
	    COM2.hError = ErrorCreate("US2_Error");
	    AT91F_PDC_DisableTx(COM2.hPDC);
	    AT91F_PDC_DisableRx(COM2.hPDC);
	    AT91F_US2_CfgPIO(); //* Define RXD and TXD as peripheral
	    AT91F_PMC_EnablePeriphClock(AT91C_BASE_PMC, 1 << AT91C_ID_US2); // First, enable the clock of the PIOB
	    AT91F_US_Configure(COM2.hPort, MCK, mode, BaudRate, 0); // Usart Configure
	    AT91F_AIC_ConfigureIt(AT91C_BASE_AIC, AT91C_ID_US2,
		    USART_INTERRUPT_LEVEL,
		    AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE,
		    (void(*)(void)) AT91_US2_ISR_ENTRY);
	    AT91F_AIC_EnableIt(AT91C_BASE_AIC, AT91C_ID_US2);
	    COM2.hRxQueue = xQueueCreate(1, sizeof(int));
	    while (COM2.hRxQueue == 0)
		; // Queue was not created and must not be used.
	    COM2.hTxQueue = xQueueCreate(1, sizeof(int));
	    while (COM2.hTxQueue == 0)
		; // Queue was not created and must not be used.
	    break;
	default:
	    while (1)
		;// error port notfound;
	    }

	}
    portEXIT_CRITICAL();
    }
