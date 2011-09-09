/*
 * uart.h
 *
 *  Created on: Mar 24, 2011
 *      Author: baron
 */
#ifndef USART2_H
#define USART2_H

#include "board.h"

#define  USART2_TX_DMA            DMA1_Channel7
#define  USART2_TX_DMA_IRQn       DMA1_Channel7_IRQn
#define  USART2_TX_DMA_IRQHandler DMA1_Channel7_IRQHandler

#define  USART2_RX_DMA          DMA1_Channel6
#define  USART2_RX_DMA_IRQn     DMA1_Channel6_IRQn

typedef struct
{
	bool flag;
	bool Readflag;
	uint8_t buffer[256];
	uint8_t size;
}ReceivedMsgStr;
ReceivedMsgStr ReceivedMsg;

extern void uart2Init(uint32_t brd);
extern void uart2WriteEscaped(unsigned char *src, unsigned int len);
extern void uart2Write(unsigned char *src, unsigned int len);
extern void USART2TIMConfigure(uint16_t aTIMtime);

extern unsigned int uart2ReadEscaped(unsigned char *dsn, unsigned int max_len);
extern unsigned int uart2Read(unsigned char *dsn, unsigned int max_len);
extern void USART2_IRQHandler(void);
void DMA1_Channel6_IRQHandler(void);
void DMA1_Channel7_IRQHandler(void);

#endif /*UART_H*/
