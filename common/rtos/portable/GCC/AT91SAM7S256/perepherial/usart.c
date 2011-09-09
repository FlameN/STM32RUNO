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

AT91_USART_DEV COM0, COM1;

/* Interrupt entry point written in the assembler file serialISR.s79. */
void AT91_US0_ISR_ENTRY(void) __attribute__((naked));

void AT91_US1_ISR_ENTRY(void) __attribute__((naked));

void AT91F_US_INTERRUPT(AT91_USART_DEV*);

//*------------------------- Internal Function --------------------------------
void AT91_US0_ISR_ENTRY(void)
{
	portSAVE_CONTEXT();
	AT91F_US_INTERRUPT(&COM0);
	portRESTORE_CONTEXT();
}
void AT91_US1_ISR_ENTRY(void)
{
	portSAVE_CONTEXT();
	AT91F_US_INTERRUPT(&COM1);
	portRESTORE_CONTEXT();
}

void AT91F_US_INTERRUPT(AT91_USART_DEV *mdev)
{
	unsigned int status = mdev->hPort->US_CSR;
	int size;
	portBASE_TYPE xTaskWokenByPost = pdFALSE;
	status &= mdev->hPort->US_IMR;

	if ((status & AT91C_US_TIMEOUT))
	{
		mdev->rxSize = mdev->bufSize - (int) mdev->hPort->US_RCR;
		xQueueSendFromISR(mdev->hRxQueue, (void *) (&size), &xTaskWokenByPost);
		AT91F_US_DisableIt(mdev->hPort, AT91C_US_TIMEOUT | AT91C_US_RXBUFF);
		AT91F_US_DisableRx(mdev->hPort);
	}
	if ((status & AT91C_US_TXEMPTY))
	{
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
	portENTER_CRITICAL();
	{
		AT91F_US_EnableIt(COM, AT91C_US_TXEMPTY);
		AT91F_US_EnableTx(COM);
		COM->US_TNCR = 0;
		COM->US_TNCR = 0;
		COM->US_TPR = (unsigned int) data;
		COM->US_TCR = size;
	}
	portEXIT_CRITICAL();
}
//===============================================================================================
inline void AT91F_US_START_RECEIVE(AT91PS_USART COM, uint8 *data, int size)
{
	portENTER_CRITICAL();
	{
		AT91F_US_TIMEOUT_SET(COM,0x20);
		AT91F_US_EnableIt(COM, AT91C_US_TIMEOUT | AT91C_US_RXBUFF);
		COM->US_RNCR = 0;
		COM->US_RNCR = 0;
		COM->US_RPR = (unsigned int) data;
		COM->US_RCR = size;
		AT91F_US_EnableRx(COM);
	}
	portEXIT_CRITICAL();
}
//===============================================================================================
void AT91F_USART_SEND(AT91_USART_DEV *dev, uint8 *data, int size)
{
	int Param;
	AT91F_US_START_TRANSMITION(dev->hPort, data, size);
	xQueueReceive(dev->hTxQueue, (void *) &Param, (portTickType) 1000);
	AT91F_US_DisableIt(dev->hPort, AT91C_US_TXEMPTY);
	AT91F_US_DisableTx(dev->hPort);
}
//===============================================================================================
int AT91F_USART_RECEIVE(AT91_USART_DEV *dev, uint8 *data, int dsize,
		int timeout)
{
	dev->bufSize = dsize;
	dev->rxSize = 0;
	AT91F_US_START_RECEIVE(dev->hPort, data, dsize);
	xQueueReceive(dev->hRxQueue, (void *) &dsize, (portTickType) timeout);
	AT91F_US_DisableIt(dev->hPort, AT91C_US_TIMEOUT);
	AT91F_US_DisableRx(dev->hPort);
	return (dev->rxSize);
}
//===============================================================================================
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
			AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA, // PIO controller base address
					(((unsigned int) AT91C_PA5_RXD0)
							| ((unsigned int) AT91C_PA6_TXD0)), // Peripheral A
					0);
			AT91F_PMC_EnablePeriphClock(AT91C_BASE_PMC, 1 << AT91C_ID_US0); // First, enable the clock of the PIOB
			AT91F_US_Configure(COM0.hPort, MCK, mode, BaudRate, 0); // Usart Configure

			AT91F_AIC_ConfigureIt(AT91C_BASE_AIC, AT91C_ID_US0,
					USART_INTERRUPT_LEVEL, AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL,
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
			COM1.id = idPort;
			COM1.hPort = AT91C_BASE_US1;
			COM1.hPDC = AT91C_BASE_PDC_US1;
			COM1.hError = ErrorCreate("US1_Error");
			AT91F_PDC_DisableTx(COM1.hPDC);
			AT91F_PDC_DisableRx(COM1.hPDC);
			AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA, // PIO controller base address
					(((unsigned int) AT91C_PA21_RXD1)
							| ((unsigned int) AT91C_PA22_TXD1)), // Peripheral A
					0);
			AT91F_PMC_EnablePeriphClock(AT91C_BASE_PMC, 1 << AT91C_ID_US1); // First, enable the clock of the PIOB
			AT91F_US_Configure(COM1.hPort, MCK, mode, BaudRate, 0); // Usart Configure
			AT91F_AIC_ConfigureIt(AT91C_BASE_AIC, AT91C_ID_US1,
					USART_INTERRUPT_LEVEL, AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL,
					(void(*)(void)) AT91_US1_ISR_ENTRY);
			AT91F_AIC_EnableIt(AT91C_BASE_AIC, AT91C_ID_US1);

			COM1.hRxQueue = xQueueCreate(1, sizeof(int));
			while (COM1.hRxQueue == 0)
				; // Queue was not created and must not be used.

			COM1.hTxQueue = xQueueCreate(1, sizeof(int));
			while (COM1.hTxQueue == 0)
				; // Queue was not created and must not be used.
			break;
		default:
			while (1)
				;// error port notfound;
		}

	}
	portEXIT_CRITICAL();
}
