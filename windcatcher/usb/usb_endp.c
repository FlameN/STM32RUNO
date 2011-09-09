/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
 * File Name          : usb_endp.c
 * Author             : MCD Application Team
 * Version            : V3.2.1
 * Date               : 07/05/2010
 * Description        : Endpoint routines
 ********************************************************************************
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
 * AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
 * CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
 * INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_mem.h"
#include "hw_config.h"
#include "usb_istr.h"
#include "usb_pwr.h"
#include "FreeRTOS.h"
#include "task.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Interval between sending IN packets in frame number (1 frame = 1ms) */
#define VCOMPORT_IN_FRAME_INTERVAL             5

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t USB_Rx_Buffer[256];
uint8_t *USB_Tx_Buffer;
uint8_t USB_Rx_Cnt;
uint8_t USB_Tx_Cnt;

//extern  uint8_t USART_Rx_Buffer[];
//extern uint32_t USART_Rx_ptr_out;
//extern uint32_t USART_Rx_length;
//extern

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
 * Function Name  : EP1_IN_Callback
 * Description    :
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/

void UsbSendData(uint8_t *src, uint32_t len)
{
	portENTER_CRITICAL();
	{
		USB_Tx_Buffer = src;
		USB_Tx_Cnt = len;
	}
	portEXIT_CRITICAL();
	while(USB_Tx_Cnt!=0)vTaskDelay(1);
	//xQueueReceive(UsbQueue, (void *) &Param, (portTickType) 1000);
}

uint32_t UsbReceiveData(uint8_t *dsn, uint32_t maxlen)
{
	uint8_t i;
	for(i=0;i<USB_Rx_Cnt;i++)
	{
		dsn[i] = USB_Rx_Buffer[i];
		if((maxlen--)==0)break;
	}
	i= USB_Rx_Cnt;
	USB_Rx_Cnt = 0;
	return i;
}


void EP1_IN_Callback(void)
{
	if (USB_Tx_Cnt > 0)
	{
		if (USB_Tx_Cnt > VIRTUAL_COM_PORT_DATA_SIZE)
		{
			UserToPMABufferCopy(USB_Tx_Buffer, ENDP1_TXADDR,
					VIRTUAL_COM_PORT_DATA_SIZE);
			SetEPTxCount(ENDP1, VIRTUAL_COM_PORT_DATA_SIZE);
			SetEPTxValid(ENDP1);
			USB_Tx_Cnt -= VIRTUAL_COM_PORT_DATA_SIZE;
			USB_Tx_Buffer += VIRTUAL_COM_PORT_DATA_SIZE;
		}
		else
		{
			UserToPMABufferCopy(USB_Tx_Buffer, ENDP1_TXADDR, USB_Tx_Cnt);
			SetEPTxCount(ENDP1, USB_Tx_Cnt);
			SetEPTxValid(ENDP1);
			USB_Tx_Cnt = 0;
		}
	}
}

/*******************************************************************************
 * Function Name  : EP3_OUT_Callback
 * Description    :
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void EP3_OUT_Callback(void)
{
	if(USB_Rx_Cnt%VIRTUAL_COM_PORT_DATA_SIZE==0)
	{
		USB_Rx_Cnt += USB_SIL_Read(EP3_OUT, &USB_Rx_Buffer[USB_Rx_Cnt]);
	}
	// Get the received data buffer and update the counter

#ifndef STM32F10X_CL
	// Enable the receive of data on EP3
	SetEPRxValid(ENDP3);
#endif // STM32F10X_CL
}

/*******************************************************************************
 * Function Name  : SOF_Callback / INTR_SOFINTR_Callback
 * Description    :
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
#ifdef STM32F10X_CL
void INTR_SOFINTR_Callback(void)
#else
void SOF_Callback(void)
#endif /* STM32F10X_CL */
{
	static uint32_t FrameCount = 0;

	if (bDeviceState == CONFIGURED)
	{
		if (FrameCount++ == VCOMPORT_IN_FRAME_INTERVAL)
		{
			/* Reset the frame counter */
			FrameCount = 0;

			/* Check the data to be sent through IN pipe */
			Handle_USBAsynchXfer();
		}
	}
}
/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/

