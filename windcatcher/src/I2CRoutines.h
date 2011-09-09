/**
  ******************************************************************************
  * @file OptimizedI2Cexamples/inc/I2CRoutines.h
  * @author  MCD Application Team
  * @version  V4.0.0
  * @date  06/18/2010
  * @brief  Header for I2CRoutines.c
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */
/* Includes ------------------------------------------------------------------*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __I2CROUTINES_H
#define __I2CROUTINES_H

#include "stm32f10x.h"

#define Polling_Master_Transmit
#define Polling_Master_Receive
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

#define SLAVE_ADDRESS     0x0
#define BufferSize            6
#define ClockSpeed            400000
#define I2C1_DR_Address    0x40005410
#define I2C2_DR_Address    0x40005810
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Master_Configuration(void);
void I2C_Master_BufferRead(u8* pBuffer,  u16 NumByteToRead);
void I2C_Master_BufferWrite(u8* pBuffer,  u16 NumByteToWrite);
u8 I2C_Master_BufferRead1Byte(void);
void I2C_Master_BufferRead2Byte(u8* pBuffer);

#endif

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
