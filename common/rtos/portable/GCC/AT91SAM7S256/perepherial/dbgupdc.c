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

AT91PS_DBGU DBGU = AT91C_BASE_DBGU;

//*----------------------------------------------------------------------------
//* \fn
//* \brief
//*----------------------------------------------------------------------------
int AT91F_DBGU_RCR() {
	return DBGU->DBGU_RCR;
}
//*----------------------------------------------------------------------------
//* \fn
//* \brief
//*----------------------------------------------------------------------------
int AT91F_DBGU_TCR() {
	return DBGU->DBGU_TCR;
}
//*----------------------------------------------------------------------------
//* \fn
//* \brief
//*----------------------------------------------------------------------------
void AT91F_DBGU_TRANSMIT(unsigned char *data, int size) {
	portENTER_CRITICAL();
	{
		DBGU->DBGU_CR = AT91C_US_TXEN;
		DBGU->DBGU_TCR = 0;
		DBGU->DBGU_TNCR = 0;
		DBGU->DBGU_TPR = (unsigned int) data;
		DBGU->DBGU_TCR = size;
	}
	portEXIT_CRITICAL();
}
//*----------------------------------------------------------------------------
//* \fn
//* \brief
//*----------------------------------------------------------------------------
void AT91F_DBGU_RECEIVE(unsigned char *data, int size) {
	portENTER_CRITICAL();
	{
		DBGU->DBGU_CR = AT91C_US_RXEN;
		DBGU->DBGU_RCR = 0;
		DBGU->DBGU_RNCR = 0;
		DBGU->DBGU_RPR = (unsigned int) data;
		DBGU->DBGU_RCR = size;
	}
	portEXIT_CRITICAL();

}
//*----------------------------------------------------------------------------
//* \fn
//* \brief
//*----------------------------------------------------------------------------
void Dbgu_init(long BaudRate) {
	portENTER_CRITICAL();
	{
		AT91F_DBGU_CfgPIO(); //* Define RXD and TXD as peripheral
		AT91F_DBGU_CfgPMC(); //* Define RXD and TXD as peripheral
		AT91F_US_Configure((AT91PS_USART)DBGU, MCK, AT91C_US_ASYNC_MODE, BaudRate, 0); // Usart Configure
	}
	portEXIT_CRITICAL();
}
//===============================================================================================


