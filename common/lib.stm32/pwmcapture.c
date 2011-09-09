/*
 * pwmcature.c
 *
 *  Created on: Mar 27, 2011
 *      Author: baron
 */

#include "pwmcapture.h"

#define PWMC_CHANELS_AMOUNT 2

uint32_t pwmcVlue[PWMC_CHANELS_AMOUNT];

void pwmcInit()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = TIM8_CC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* GPIOC Configuration:TIM8 Channel3-4 in Input */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
	/*---------------------------- TIM8 Configuration ----------------------------*/
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 65535;
	TIM_TimeBaseStructure.TIM_Prescaler = 36000;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;//TIM_CKD_DIV8;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);
	// Input Capture Configuration --------------------------------
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV8;
	TIM_ICInitStructure.TIM_ICFilter = 0x00;

	TIM_ICInit(TIM8, &TIM_ICInitStructure);
	// Input Capture Configuration --------------------------------
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = 0;//TIM_ICPSC_DIV8;
	TIM_ICInitStructure.TIM_ICFilter = 0x00;

	TIM_ICInit(TIM8, &TIM_ICInitStructure);

	// Enable the CC2 Interrupt Request
	TIM_ITConfig(TIM8, TIM_IT_CC3, ENABLE);
	// Enable the CC2 Interrupt Request
	TIM_ITConfig(TIM8, TIM_IT_CC4, ENABLE);

	// TIM enable counter
	TIM_Cmd(TIM8, ENABLE);



}
void TIM8_CC_IRQHandler(void)
{
	static uint16_t oldCC3, oldCC4,oldP4;
	static int led1 = 0, led2 = 0;
	uint16_t tmp;
	if (TIM_GetITStatus(TIM8, TIM_IT_CC3) == SET)
	{
		if (led1 == 0)
		{
			GPIO_SetBits(GPIOB, GPIO_Pin_1);
		}
		else
		{
			GPIO_ResetBits(GPIOB, GPIO_Pin_1);
		}
		led1 ^= 1;
		// Clear TIM3 Capture compare interrupt pending bit
		TIM_ClearITPendingBit(TIM8, TIM_IT_CC3);
		tmp = TIM_GetCapture3(TIM8);

		// Wind Calculation
		// 100hz = 40 ms
		pwmcVlue[0] = (8 * 100000000 / 25) / ((uint32_t) (tmp - oldCC3) * 5); // Convert in mm/s
		oldCC3 = tmp;
	}
	if (TIM_GetITStatus(TIM8, TIM_IT_CC4) == SET)
	{
		if (led2 == 0)
		{
			GPIO_SetBits(GPIOB, GPIO_Pin_0);
		}
		else
		{
			GPIO_ResetBits(GPIOB, GPIO_Pin_0);
		}
		led2 ^= 1;

		// Clear TIM3 Capture compare interrupt pending bit
		TIM_ClearITPendingBit(TIM8, TIM_IT_CC4);
		tmp = TIM_GetCapture4(TIM8);
		// Generator RPM calculation
		// 500us - 1 cycle
		pwmcVlue[1] = 60 * 10000000 / ((uint32_t) (tmp - oldCC4 + oldP4) * 5); // Convert in rpm*10^-3 in 1 minute
		oldP4 = tmp - oldCC4;
		oldCC4 = tmp;
	}

}

uint32_t pwmcGet(int i)
{
	if (i == 0)
	{
		return pwmcVlue[0];
	};
	return pwmcVlue[1];

}
