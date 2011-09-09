/*******************************************************************************
 * Project            : STM32 MINI Digital Picture Frame
 * File Name          : main.c
 * Author             : Martin Thomas, main-skeleton based on code from the
 *                      STMicroelectronics MCD Application Team
 * Version            : see VERSION_STRING below
 * Date               : see VERSION_STRING below
 * Description        : Main program body for the SD-Card tests
 ********************************************************************************
 * License: BSD
 *******************************************************************************/
#define VERSION_STRING "V1.0.0 24.03.2011"
/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include "board.h"
#include "hwinit.h"
#include "FreeRTOS.h"
#include "task.h"
#include "gprscon.h"
#include "logica.h"
#include "reletest.h"
#include "rs485.h"
#include "usbtask.h"
#include "spibus.h"
#include "I2CRoutines.h"
#include "RTC_HardClock.h"
#include "clock_calendar.h"
#include "usermemory.h"
/* Private function prototypes -----------------------------------------------*/
void GPIO_Configuration(void);

/* Public functions -- -------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/


#define vLogicaTask_PRIORITY            ( tskIDLE_PRIORITY + 4 )
#define vRs485Task_PRIORITY            ( tskIDLE_PRIORITY + 3 )
#define vUSBTask_PRIORITY            ( tskIDLE_PRIORITY + 1 )
#define vReletestTask_PRIORITY            ( tskIDLE_PRIORITY + 5 )
#define vGprsConTask_PRIORITY           ( tskIDLE_PRIORITY + 2 )

int main(void)
{
	hwInit();
	/* Configure the GPIO ports */
	GPIO_Configuration();

	ApplicationInit();
	RCC_HSICmd(ENABLE);
	MemInit();
	/* Turn on/off LED(s) */



	xTaskCreate(vLogicaTask, (const signed portCHAR * const ) "Logica",
		vLogicaTask_STACK_SIZE, NULL, vLogicaTask_PRIORITY,
		(xTaskHandle *) NULL);

	xTaskCreate(vRs485Task, (const signed portCHAR * const ) "RS485",
		vRs485Task_STACK_SIZE, NULL, vRs485Task_PRIORITY,
		(xTaskHandle *) NULL);

	xTaskCreate(vReletestTask, (const signed portCHAR * const ) "Reletest",
				vReletestTask_STACK_SIZE, NULL, vReletestTask_PRIORITY,
				(xTaskHandle *) NULL);

	xTaskCreate(vUSBTask, (const signed portCHAR * const ) "USB",
			vUSBTask_STACK_SIZE, NULL, vUSBTask_PRIORITY,
				(xTaskHandle *) NULL);


	xTaskCreate(vGprsConTask, (const signed portCHAR * const ) "GPRS",
				vGprsConTask_STACK_SIZE, NULL, vGprsConTask_PRIORITY,
						(xTaskHandle *) NULL);

	/* Start the scheduler. */

	vTaskStartScheduler();

	while (1)
	{


	};

	return 0;
}

void vApplicationIdleHook(void)
{

}
void vApplicationTickHook(void)
{
	//RTC_Timer();

}
void vApplicationStackOverflowHook(xTaskHandle *pxTask,
		signed portCHAR *pcTaskName)
{

	while (1)
		;
}

void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/* Enable GPIOA clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	/* Enable GPIOC clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	/* Enable GPIOB clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* Configure I2C1 pins: SCL and SDA ----------------------------------------*/
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

	//===================== RS232 =========================
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	//===================== End of RS232 =========================

//===================== RS485 =========================
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
//===================== End of RS485 =========================

	/* Relays */

	//P1 P2 P3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Discrets */

	//DS1 DS9 DS10 DS11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);


	//DS2 DS3 DS4 DS5
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	//DS6 DS7 DS8
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

