/*
 * adc.h
 *
 *  Created on: Mar 18, 2011
 *      Author: baron
 */

#ifndef ADCBARON_H_
#define ADCBARON_H_
#include "board.h"
#define CHANELS_AMOUNT  3

#define ADC_DMA_CHANEL DMA1_Channel1


extern void GetADC(int32_t *buf);
extern void ADC_Start();

#endif /* ADC_H_ */
