#include "usart3notDMA.h"

typedef struct
{
	uint32_t rxCurrent;
	uint32_t txCurrentStart;
	uint32_t txCurrentEnd;
	uint8_t rxBuf[UARTRX_FIFO_SIZE];
	uint8_t txBuf[UARTTX_FIFO_SIZE];
	uint8_t txBufB[UARTTX_FIFOB_SIZE];
} USART_FIFO_STR;

USART_FIFO_STR u3Fifo;

void GPIOforUSART3cfg(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
                           RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);

    // Tx on PC10 as alternate function push-pull
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* Rx on PC11 as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void usart3Init(uint32_t brd)
{
	GPIOforUSART3cfg();
	USART_InitTypeDef USART_InitStructure;

	// USART 3 init
	USART_DeInit(USART3);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, DISABLE);
	GPIO_PinRemapConfig(GPIO_PartialRemap_USART3,ENABLE);

	USART_InitStructure.USART_BaudRate = brd;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);

	USART_ClearFlag(USART3, USART_FLAG_CTS | USART_FLAG_LBD  |
						USART_FLAG_TC  | USART_FLAG_RXNE );

	//----------DMA------------
	uartRxDMAConfiguration(USART3, USART3_RX_DMA, u1Fifo.rxBuf,
				UARTRX_FIFO_SIZE);

	uartTxDMAConfiguration(USART3, USART3_TX_DMA, u1Fifo.txBuf, cnt,
						1);

	USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
	USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);

	/* Enable USART_Rx Receive interrupt */
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
		    /* Configure USART3 interrupt */
	NVIC_SetPriority(USART3_RX_DMA_IRQn, 0x0);
	NVIC_EnableIRQ(USART3_RX_DMA_IRQn);
	//----------------------------
	USART_Cmd(USART3, ENABLE);
}

void DMA1_Channel4_IRQHandler(void)
{

	/* Disable DMA1_Channel4 transfer*/
	DMA_Cmd(USART3_TX_DMA, DISABLE);
	/*  Clear DMA1_Channel4 Transfer Complete Flag*/
	DMA_ClearFlag(DMA1_FLAG_TC4);
	uart3TxTask();
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
	//if (ie)
	{
		DMA_ITConfig(USART3_TX_DMA, DMA_IT_TC, ENABLE);
	}
	/* Enable USART_Tx DMA Tansmit request */
	/* Configure DMA1_Channel_Tx interrupt */
	NVIC_SetPriority(USART3_TX_DMA_IRQn, 0x01);
	NVIC_EnableIRQ(USART3_TX_DMA_IRQn);
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

	DMA_ITConfig(USART3_RX_DMA, DMA_IT_TC, ENABLE);

	/* Enable USART_Rx DMA Tansmit request */
	/* Configure DMA1_Channel_Rx interrupt */
	NVIC_SetPriority(USART3_RX_DMA_IRQn, 0x02);
	NVIC_EnableIRQ(USART3_RX_DMA_IRQn);
}
