/*
 * uart.c
 *
 *  Created on: Mar 24, 2011
 *      Author: baron
 */

#include "uart.h"



void USART3_IRQHandler(void)
{
	/*
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        //USART_SendData(USART3, USART_ReceiveData(USART3));
    	uint16_t data = USART_ReceiveData(USART3);

    	RxBuffer[RxPointer++] = (uint8_t)(0x0f & data);
    	RxLength++;
    	RxBuffer[RxPointer++] = (uint8_t)(data << 8);
    	RxLength++;
    }*/
}

int dmaRxCounter = 0;

void DMARxDataCounterSet(int counter)
{
	dmaRxCounter=counter;
}

void DMARxDataCounterInc()
{
	dmaRxCounter++;
}

int DMARxDataCounterGet()
{
	return dmaRxCounter;
}



UART_FIFO_STR u1Fifo;
void TestSend()
{
	u1Fifo.txBuf[0]='A';
	u1Fifo.txBuf[1]='B';
	uartTxDMAConfiguration(USART3, USART3_TX_DMA, u1Fifo.txBuf, 2,1);
}

void uartTxDMAConfiguration(USART_TypeDef *uart, DMA_Channel_TypeDef *dmatx,
		uint8_t *txBuf, uint32_t len, bool ie)
{
	DMA_InitTypeDef DMA_InitStructure;

	/* DMA1 Channel (triggered by USART_Tx event) Config */
	DMA_DeInit(dmatx);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &uart->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) txBuf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = len;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(dmatx, &DMA_InitStructure);
	DMA_Cmd(dmatx, ENABLE);

}

/**
 * @brief  Configures the DMA.
 * @param  uart, dmatx,dmarx,interrupt enable
 * @retval : None
 */
void uartRxDMAConfiguration(USART_TypeDef *uart, DMA_Channel_TypeDef *dmarx,
		unsigned char *rxBuf, uint32_t len)
{
	DMA_InitTypeDef DMA_InitStructure;

	/* DMA1 Channel (triggered by USART1 Rx event) Config */
	DMA_DeInit(dmarx);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &uart->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) rxBuf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = len;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(dmarx, &DMA_InitStructure);
	DMA_Cmd(dmarx, ENABLE);
	//DMA_ITConfig(USART3_RX_DMA, DMA_IT_TC, ENABLE);
}
void uart3Init(uint32_t baudrate)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);

	//	/* Configure USART1 TX (PA.09) as alternate function push-pull */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(GPIOC, &GPIO_InitStructure);

		/* Configure USART1 RX (PA.10) as input floating */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOC, &GPIO_InitStructure);

	USART_DeInit(USART3);


	USART_InitTypeDef USART_InitStructure;

	/* Enable AFIO,  clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	/* Enable GPIO clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOA
	|RCC_APB2Periph_GPIOB, ENABLE);

	/* Enable USART3,  clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	GPIO_PinRemapConfig(GPIO_PartialRemap_USART3,ENABLE);


	USART_InitStructure.USART_BaudRate = baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl
			= USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	/* Configure USART3 */
	USART_Init(USART3, &USART_InitStructure);

	USART_ClearFlag(USART3, USART_FLAG_CTS | USART_FLAG_LBD  |
						USART_FLAG_TC  | USART_FLAG_RXNE );

	uartRxDMAConfiguration(USART3, USART3_RX_DMA, u1Fifo.rxBuf,
			UARTRX_FIFO_SIZE);
	USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
	/* Enable USART_Rx DMA Receive request */
	USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);

	  /* Enable USART_Rx Receive interrupt */
	  //USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	    /* Configure USART3 interrupt */
	    NVIC_SetPriority(USART3_RX_DMA_IRQn, 0x7);
	    NVIC_EnableIRQ(USART3_RX_DMA_IRQn);
	/* Enable the USART3 */
	USART_Cmd(USART3, ENABLE);
}
//-----------------------------------------------------------------------------------
unsigned int uart3FindEscapedPack()
{
	unsigned int i = u1Fifo.rxCurrent;

	while (i != (UARTRX_FIFO_SIZE - DMA_GetCurrDataCounter(USART3_RX_DMA)))
	{


		if (u1Fifo.rxBuf[i] == 0x55)
		{
			i++;
			i &= UARTRX_FIFO_SIZE_MASK;
			if (i == (USART3_RX_DMA->CMAR - (uint32_t) &u1Fifo.rxBuf[0]))
				return 0;//return (isStart & isEnd);

			if (u1Fifo.rxBuf[i] == 0x03)
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

int uart3GetChar(unsigned char *ch)
{
	if (u1Fifo.rxCurrent != (UARTRX_FIFO_SIZE - DMA_GetCurrDataCounter(USART3_RX_DMA)))
	{
		*ch = u1Fifo.rxBuf[u1Fifo.rxCurrent];
		u1Fifo.rxCurrent++;
		u1Fifo.rxCurrent &= UARTRX_FIFO_SIZE_MASK;
		return 1;
	}
	return 0;
}
//-----------------------------------------------------------------------------------
int uart3PutChar(unsigned char ch)
{

	//	IEC2bits.DMA4IE = 0;
	u1Fifo.txBufB[u1Fifo.txCurrentEnd] = ch;
	u1Fifo.txCurrentEnd++;
	u1Fifo.txCurrentEnd &= UARTTX_FIFOB_SIZE_MASK;

	if (u1Fifo.txCurrentEnd == u1Fifo.txCurrentStart)
	{
		u1Fifo.txCurrentEnd--;
		u1Fifo.txCurrentEnd &= UARTTX_FIFOB_SIZE_MASK;
		//		IEC2bits.DMA4IE = 1;
		return 0;
	}
	//	IEC2bits.DMA4IE = 1;
	return 1;
}
//-----------------------------------------------------------------------------------
void uart3TxTask()
{
	unsigned int cnt = 0;
	// If DMA COMPLITE
	if ((USART3_TX_DMA->CCR & ((uint32_t) 0x00000001)) == 0)
	{
		while ((u1Fifo.txCurrentStart != u1Fifo.txCurrentEnd) && (cnt
				< UARTTX_FIFO_SIZE))
		{
			u1Fifo.txBuf[cnt] = u1Fifo.txBufB[u1Fifo.txCurrentStart];
			u1Fifo.txCurrentStart++;
			u1Fifo.txCurrentStart &= UARTTX_FIFOB_SIZE_MASK;
			cnt++;
		}
		if (cnt > 0)
		{
			uartTxDMAConfiguration(USART3, USART3_TX_DMA, u1Fifo.txBuf, cnt,
					1);
			DMA_ITConfig(USART3_TX_DMA, DMA_IT_TC, ENABLE);
				//}
				/* Enable USART_Tx DMA Tansmit request */
				/* Configure DMA1_Channel_Tx interrupt */
				NVIC_SetPriority(USART3_TX_DMA_IRQn, 0x06);
				NVIC_EnableIRQ(USART3_TX_DMA_IRQn);
		}
	}
}
//-----------------------------------------------------------------------------------
unsigned int uart3ReadEscaped(unsigned char *dsn, unsigned int max_len)
{
	unsigned short cnt = 0, crc = 0;
	unsigned char v, *ptr = dsn;
	if (uart3FindEscapedPack() == 0)
	{
		return 0;
	}
	while (uart3GetChar(&v) && (max_len > cnt))
	{
		if (v == 0x55)
		{
			if (!uart3GetChar(&v))
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
unsigned int uart3Read(unsigned char *dsn, unsigned int max_len)
{
	unsigned short cnt = 0, crc = 0;
	unsigned char v, *ptr = dsn;
	/*
	u1Fifo.rxBuf[0] = '$';
	for (int i = 0; i < 32; i++)
	{
		v = u1Fifo.rxBuf[i];
		*ptr = v;
		ptr++;
	}
return 32;
*/
	while (uart3GetChar(&v) && (max_len > cnt))
	{
		*ptr = v;
		ptr++;
		crc += v;
		cnt++;
	}
	return cnt; //  error here
}

//-----------------------------------------------------------------------------------
void uart3WriteEscaped(unsigned char *src, unsigned int len)
{
	unsigned char ch;
	unsigned char crc = 0;//getCRC(src, len);
	unsigned int rezult;
	if (len == 0)
		return;
	uart3PutChar(0x55);
	uart3PutChar(0x01);
	while (len--)
	{
		ch = *src;
		rezult = uart3PutChar(ch);
		if (ch == 0x55)
		{
			rezult = uart3PutChar(0x02);
		}
		src++;
	}
	ch = crc;
	rezult = uart3PutChar(ch);
	if (ch == 0x55)
	{
		rezult = uart3PutChar(0x02);
	}

	rezult = uart3PutChar(0x55);
	rezult = uart3PutChar(0x03);
	uart3TxTask();
}
void uart3Write(unsigned char *src, unsigned int len)
{
	unsigned char ch;
	unsigned int rezult;
	if (len == 0)
		return;
	while (len--)
	{
		ch = *src;
		rezult = uart3PutChar(ch);
		src++;
	}
	uart3TxTask();
}

//UART 1 DMA INTERRUPT
void DMA1_Channel2_IRQHandler(void)
{
	/* Disable DMA1_Channel2 transfer*/
	DMA_Cmd(USART3_TX_DMA, DISABLE);
	/*  Clear DMA1_Channel2 Transfer Complete Flag*/
	DMA_ClearFlag(DMA1_FLAG_TC2);
	uart3TxTask();
}

void DMA1_Channel3_IRQHandler(void)
{
	/* Disable DMA1_Channel2 transfer*/
	//DMA_Cmd(USART3_TX_DMA, DISABLE);
	/*  Clear DMA1_Channel2 Transfer Complete Flag*/
	DMA_ClearFlag(DMA1_FLAG_TC3);

	u1Fifo.rxCurrent+=0;
	int tempTest = DMA_GetCurrDataCounter(USART3_RX_DMA);
			tempTest +=0;
	//DMARxDataReceived();
	//uart3TxTask();
}



