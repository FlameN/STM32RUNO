// SPI_BUS
#include <stdlib.h>
/* Scheduler include files. */
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f10x_conf.h"

#include "relay.h"

#include "diskrets.h"
#include "usermemory.h"


void vSpibusTask(void *pvParameters)
{

	while (1)
	{
		if ((RAM.relays[0] & 1) != 0)
		{
			GPIO_SetBits(GPIOB, GPIO_Pin_0);
		}
		else
		{
			GPIO_ResetBits(GPIOB, GPIO_Pin_0);
		}
		if ((RAM.relays[0] & 2) != 0)
		{
			GPIO_SetBits(GPIOB, GPIO_Pin_1);
		}
		else
		{
			GPIO_ResetBits(GPIOB, GPIO_Pin_1);
		}
		if ((RAM.relays[0] & 4) != 0)
		{
			GPIO_SetBits(GPIOB, GPIO_Pin_2);
		}
		else
		{
			GPIO_ResetBits(GPIOB, GPIO_Pin_2);
		}


		DiskretSet(0,GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15));
		DiskretSet(1,GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_6));
		DiskretSet(2,GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7));
		DiskretSet(3,GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8));
		DiskretSet(4,GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9));
		DiskretSet(5,GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8));
		DiskretSet(6,GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_9));
		DiskretSet(7,GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_10));
		// ***
		//DiskretSet(N,AT91F_PIO_GetInput(AT91C_BASE_PIOA,RUNO_DISKRETN));
		DiskretsProcess();
		vTaskDelay(2);
	}

};
