// SPI_BUS
#include <stdlib.h>
/* Scheduler include files. */
#include "logica.h"
#include "FreeRTOS.h"
#include "task.h"
#include "translator.h"

#include "stm32f10x_flash.h"
#include "usermemory.h"

void vLogicaTask(void *pvParameters)
{

	int heapSize=0;
	while (1)
	{
		heapSize = xPortGetFreeHeapSize();
					heapSize +=0;
		  GPIO_SetBits(GPIO_LED,LED);
		  		vTaskDelay(100);
		  		GPIO_ResetBits(GPIO_LED,LED);
		  		vTaskDelay(100);
		//vPortEnterCritical();

		//vPortExitCritical();

		DoProgram();
	}
}
