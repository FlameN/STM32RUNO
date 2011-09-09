/*
 * uart.c
 *
 *  Created on: Mar 24, 2011
 *      Author: baron
 */

#include "uart.h"
#include "usart2.h"

UART_FIFO_STR u2Fifo;
//ReceivedMsgStr ReceivedMsg;

void USART2TIMConfigure(uint16_t aTIMtime)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period = 10000;
	TIM_TimeBaseStructure.TIM_Prescaler = 1000;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);

	TIM_ARRPreloadConfig(TIM5, ENABLE);

	TIM_ICInitTypeDef TIM_ICInitStructure;
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;

	//TIM5->ARR = aTIMtime;
	//TIM_ARRPreloadConfig(TIM5,ENABLE);
	TIM_ICInit(TIM5, &TIM_ICInitStructure);

	//TIM_SelectInputTrigger(TIM5, TIM_TS_TI2FP2);
	TIM_SelectSlaveMode(TIM5, TIM_SlaveMode_Reset);
	TIM_SelectMasterSlaveMode(TIM5, TIM_MasterSlaveMode_Enable);

	TIM_Cmd(TIM5, ENABLE);

	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);

	TIM_ClearITPendingBit(TIM5, TIM_IT_Update);



/*
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period = 10000;
	TIM_TimeBaseStructure.TIM_Prescaler = 1000;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_ARRPreloadConfig(TIM2, ENABLE);


	TIM_Cmd(TIM2, ENABLE);
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);

	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
*/
}


void TIM5_IRQHandler()
{
	TIM_ClearITPendingBit(TIM5, TIM_IT_Update);

	int asize = uart2Read(ReceivedMsg.buffer,256);
	if(asize>0)
	{
		if(ReceivedMsg.size == 0)
		{
			ReceivedMsg.flag = true;
			ReceivedMsg.Readflag = false;
			ReceivedMsg.size = asize;
		}
		//TIM_Cmd(TIM2, DISABLE);
	}

}

void uart2Init(uint32_t baudrate)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);

	USART_DeInit(USART2);


	USART_InitTypeDef USART_InitStructure;

	/* Enable AFIO,  clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	/* Enable GPIO clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOA
	|RCC_APB2Periph_GPIOB, ENABLE);

	/* Enable USART2,  clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	//GPIO_PinRemapConfig(GPIO_PartialRemap_USART2,ENABLE);


	USART_InitStructure.USART_BaudRate = baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl
			= USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	/* Configure USART3 */
	USART_Init(USART2, &USART_InitStructure);

	USART_ClearFlag(USART2, USART_FLAG_CTS | USART_FLAG_LBD  |
						USART_FLAG_TC  | USART_FLAG_RXNE );

	uartRxDMAConfiguration(USART2, USART2_RX_DMA, u2Fifo.rxBuf,
			UARTRX_FIFO_SIZE);

	//USART2TIMConfigure(50);

	USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
	/* Enable USART_Rx DMA Receive request */
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);

	  /* Enable USART_Rx Receive interrupt */
	  //USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	    /* Configure USART3 interrupt */
	    NVIC_SetPriority(USART2_RX_DMA_IRQn, 0x0);
	    NVIC_EnableIRQ(USART2_RX_DMA_IRQn);

	USART_Cmd(USART2, ENABLE);

}
//-----------------------------------------------------------------------------------
unsigned int uart2FindEscapedPack()
{
	unsigned int i = u2Fifo.rxCurrent;

	while (i != (UARTRX_FIFO_SIZE - DMA_GetCurrDataCounter(USART2_RX_DMA)))
	{
		if (u2Fifo.rxBuf[i] == 0x55)
		{
			i++;
			i &= UARTRX_FIFO_SIZE_MASK;
			if (i == (USART2_RX_DMA->CMAR - (uint32_t) &u2Fifo.rxBuf[0]))
				return 0;//return (isStart & isEnd);

			if (u2Fifo.rxBuf[i] == 0x03)
			{
				return 1;
			}
		}
		i++;
		i &= UARTRX_FIFO_SIZE_MASK;
	}
	return 0;

}
//-----------------------------------------------------------------------------------

int uart2GetChar(unsigned char *ch)
{
	if (u2Fifo.rxCurrent != (UARTRX_FIFO_SIZE - DMA_GetCurrDataCounter(USART2_RX_DMA)))
	{
		*ch = u2Fifo.rxBuf[u2Fifo.rxCurrent];
		u2Fifo.rxCurrent++;
		u2Fifo.rxCurrent &= UARTRX_FIFO_SIZE_MASK;
		return 1;
	}
	return 0;
}
//-----------------------------------------------------------------------------------
int uart2PutChar(unsigned char ch)
{

	//	IEC2bits.DMA4IE = 0;
	u2Fifo.txBufB[u2Fifo.txCurrentEnd] = ch;
	u2Fifo.txCurrentEnd++;
	u2Fifo.txCurrentEnd &= UARTTX_FIFOB_SIZE_MASK;

	if (u2Fifo.txCurrentEnd == u2Fifo.txCurrentStart)
	{
		u2Fifo.txCurrentEnd--;
		u2Fifo.txCurrentEnd &= UARTTX_FIFOB_SIZE_MASK;
		//		IEC2bits.DMA4IE = 1;
		return 0;
	}
	//	IEC2bits.DMA4IE = 1;
	return 1;
}
//-----------------------------------------------------------------------------------
void uart2TxTask()
{
	unsigned int cnt = 0;
	// If DMA COMPLITE
	if ((USART2_TX_DMA->CCR & ((uint32_t) 0x00000001)) == 0)
	{
		while ((u2Fifo.txCurrentStart != u2Fifo.txCurrentEnd) && (cnt
				< UARTTX_FIFO_SIZE))
		{
			u2Fifo.txBuf[cnt] = u2Fifo.txBufB[u2Fifo.txCurrentStart];
			u2Fifo.txCurrentStart++;
			u2Fifo.txCurrentStart &= UARTTX_FIFOB_SIZE_MASK;
			cnt++;
		}
		if (cnt > 0)
		{
			uartTxDMAConfiguration(USART2, USART2_TX_DMA, u2Fifo.txBuf, cnt,
					1);
			DMA_ITConfig(USART2_TX_DMA, DMA_IT_TC, ENABLE);
				//}
				/* Enable USART_Tx DMA Tansmit request */
				/* Configure DMA1_Channel_Tx interrupt */
				NVIC_SetPriority(USART2_TX_DMA_IRQn, 0x02);
				NVIC_EnableIRQ(USART2_TX_DMA_IRQn);
		}
	}
}
//-----------------------------------------------------------------------------------
unsigned int uart2ReadEscaped(unsigned char *dsn, unsigned int max_len)
{

	unsigned short cnt = 0, crc = 0;
	unsigned char v, *ptr = dsn;
	if (uart2FindEscapedPack() == 0)
	{
		return 0;
	}
	while (uart2GetChar(&v) && (max_len > cnt))
	{
		if (v == 0x55)
		{
			if (!uart2GetChar(&v))
				return 0;
			if (v == 0x1) // start pack here;
			{
				ptr = dsn;
				cnt = 0;
				crc = 0;
				continue;
			}
			else if (v == 0x2)
			{
				v = 0x55;
			}
			else if (v == 0x3)
			{
				ptr--;
				if (((crc - *ptr) & 0x00FF) != *ptr)
					return 0; // crc fail
				return cnt;
			}
		}
		*ptr = v;
		ptr++;
		crc += v;
		cnt++;
	}
	return 0; //  error here
}
//-----------------------------------------------------------------------------------
unsigned int uart2Read(unsigned char *dsn, unsigned int max_len)
{
	unsigned short cnt = 0, crc = 0;
	unsigned char v, *ptr = dsn;
	/*
	u2Fifo.rxBuf[0] = '$';
	for (int i = 0; i < 32; i++)
	{
		v = u2Fifo.rxBuf[i];
		*ptr = v;
		ptr++;
	}
return 32;
*/
	while (uart2GetChar(&v) && (max_len > cnt))
	{
		*ptr = v;
		ptr++;
		crc += v;
		cnt++;
	}
	return cnt; //  error here
}

//-----------------------------------------------------------------------------------
void uart2WriteEscaped(unsigned char *src, unsigned int len)
{
	unsigned char ch;
	unsigned char crc = 0;//getCRC(src, len);
	unsigned int rezult;
	if (len == 0)
		return;
	uart2PutChar(0x55);
	uart2PutChar(0x01);
	while (len--)
	{
		ch = *src;
		rezult = uart2PutChar(ch);
		if (ch == 0x55)
		{
			rezult = uart2PutChar(0x02);
		}
		src++;
	}
	ch = crc;
	rezult = uart2PutChar(ch);
	if (ch == 0x55)
	{
		rezult = uart2PutChar(0x02);
	}

	rezult = uart2PutChar(0x55);
	rezult = uart2PutChar(0x03);
	uart2TxTask();
}
void uart2Write(unsigned char *src, unsigned int len)
{
	unsigned char ch;
	unsigned int rezult;
	if (len == 0)
		return;
	while (len--)
	{
		ch = *src;
		rezult = uart2PutChar(ch);
		src++;
	}
	uart2TxTask();
}

//UART 1 DMA INTERRUPT
void DMA1_Channel7_IRQHandler(void)
{
	/* Disable DMA1_Channel2 transfer*/
	DMA_Cmd(USART2_TX_DMA, DISABLE);
	/*  Clear DMA1_Channel2 Transfer Complete Flag*/
	DMA_ClearFlag(DMA1_FLAG_TC7);
	uart2TxTask();
}

