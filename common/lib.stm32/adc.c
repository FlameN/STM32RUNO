/*
 * adc.c
 *
 *  Created on: Mar 18, 2011
 *      Author: baron
 */

#include "adc.h"



#define ADC1_DR_Address    ((uint32_t)0x4001244C)


#define SAMPLES_AMOUNT	256*4


#define ADC_BUFFER_SIZE       (SAMPLES_AMOUNT * CHANELS_AMOUNT)

#define ACC_FS GPIO_Pin_12 // port cStartAdc
#define ACC_ST GPIO_Pin_2  // port D
#define ACC_PD GPIO_Pin_8  // port c

uint32_t adcb[3];
uint32_t adca[3];


#define ACC_TO_MV(adc) ((adc-1900)*100)/1303  //




extern int16_t mag[3];


ADC_InitTypeDef ADC_InitStructure;
DMA_InitTypeDef DMA_InitStructure;

__IO uint16_t ADC1ConvertedValue[ADC_BUFFER_SIZE];


int32_t ADC_ACC[CHANELS_AMOUNT];


void AddMeasurments(__IO uint16_t *src)
	{
		for (int j = 0; j < CHANELS_AMOUNT; j++)
		{
			ADC_ACC[j] = 0;
		}
		for (int i = 0; i < SAMPLES_AMOUNT; i++)
		{

			for (int j = 0; j < CHANELS_AMOUNT; j++)
			{
				ADC_ACC[j] += *src;
				src++;
			}
		}
	}

void GetADC(int32_t buf[CHANELS_AMOUNT])
{
	AddMeasurments(ADC1ConvertedValue);
	for(int i=0;i<CHANELS_AMOUNT;i++)
		{
		if(ACC_TO_MV(ADC_ACC[i])>0)
		{
		buf[i] = ACC_TO_MV(ADC_ACC[i]);
		}
		else
			buf[i] = 0;
		}
}

//==============================================================================
void ADC_DMA_Init()
{
	/* DMA1 channel1 configuration ---------------------------------------------*/
	DMA_DeInit(ADC_DMA_CHANEL);
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) ADC1ConvertedValue;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = ADC_BUFFER_SIZE;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(ADC_DMA_CHANEL, &DMA_InitStructure);

	/* Enable DMA1 channel1 */
	DMA_Cmd(ADC_DMA_CHANEL, ENABLE);

	/* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = CHANELS_AMOUNT;
	ADC_Init(ADC1, &ADC_InitStructure);

	/* ADC1 regular channel1 */
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_7Cycles5); //
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_7Cycles5); //
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_7Cycles5);  //


	/* Enable ADC1 DMA */
	ADC_DMACmd(ADC1, ENABLE);

	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);

	/* Enable TempSensor and Vrefint channels: channel16 and Channel17 */
	//ADC_TempSensorVrefintCmd(ENABLE);
	/* Enable ADC1 reset calibaration register */
	ADC_ResetCalibration(ADC1);

	/* Check the end of ADC1 reset calibration register */
	while (ADC_GetResetCalibrationStatus(ADC1))
		;

	/* Start ADC1 calibaration */
	ADC_StartCalibration(ADC1);

	/* Check the end of ADC1 calibration */
	while (ADC_GetCalibrationStatus(ADC1))
		;
}

//==============================================================================
void ADC_RCC_Configuration(void)
{
	RCC_ADCCLKConfig(RCC_PCLK2_Div6); // ADC CLOCK = 12 MHZ
	//conversion time for 1 sample = sample clck+ 12.5
	// if ST = 1.5 cycl time = 1.5+12.5 = 14 clk;
	//  (1/12000000) * 14 = 1.666667 uS = 18.66666667 uS
	// TOTOAL TIME FOR 16 chanels = 1.666667*16 =
	/* Enable DMA1 clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	/* Enable peripheral clocks --------------------------------------------------*/
	/* Enable ADC1 and GPIOC clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 , ENABLE);
}
//==============================================================================
/**
 * @brief  Configures the different GPIO ports.
 * @param  None
 * @retval None
 */
void ADC_GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

}
//==============================================================================
void ADC_Start()
{
	ADC_RCC_Configuration();
	ADC_GPIO_Configuration();
	ADC_DMA_Init();

	/* Start ADC1 Software Conversion */
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);

	/* Test on Channel 1 DMA1_FLAG_TC flag */
	// while(!DMA_GetFlagStatus(DMA1_FLAG_TC1));

	/* Clear Channel 1 DMA1_FLAG_TC flag */

	//		  DMA_ClearFlag(DMA1_FLAG_TC1);
}
