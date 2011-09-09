/*
 * board.h
 *
 *  Created on: Jul 31, 2010
 *      Author: albert
 */

#ifndef BOARD_H_
#define BOARD_H_

#include "stm32f10x_conf.h"

#define RUNO


#define GPIO_LED	GPIOB
#define GPIO_RELE	GPIOB
#define GPIO_D1		GPIOB
#define GPIO_D2toD5	GPIOC
#define GPIO_D6toD8	GPIOA
#define GPIO_D9toD11	GPIOB

#define LED			GPIO_Pin_8

#define P1           GPIO_Pin_0
#define P2           GPIO_Pin_1
#define P3           GPIO_Pin_2

#define D11         GPIO_Pin_13 //ds1
#define D10         GPIO_Pin_12 //ds1
#define D9          GPIO_Pin_11 //ds1
#define D8          GPIO_Pin_10
#define D7          GPIO_Pin_9
#define D6          GPIO_Pin_8
#define D5          GPIO_Pin_9
#define D4          GPIO_Pin_8
#define D3          GPIO_Pin_7
#define D2          GPIO_Pin_6
#define D1          GPIO_Pin_15

#endif /* BOARD_H_ */
