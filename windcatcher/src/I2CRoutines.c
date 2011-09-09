/**
  ******************************************************************************
  * @file OptimizedI2Cexamples/src/I2CRoutines.c
  * @author  MCD Application Team
  * @version  V4.0.0
  * @date  06/18/2010
  * @brief  Contains the I2Cx slave/Master read and write routines.
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
#include "I2CRoutines.h"

#include "stm32f10x_i2c.h"


DMA_InitTypeDef    DMA_InitStructure;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : I2C_Master_BufferRead
* Description    : Reads buffer of bytes  from the slave.
* Input          : - pBuffer: Buffer of bytes to read from the slave.
*                  - NumByteToRead: Number of bytes to be read by the Master.
* Output         : None.
* Return         : None.
*******************************************************************************/

I2C_InitTypeDef  I2C_InitStructure;
ErrorStatus HSEStartUpStatus;
vu8 Rx_Idx = 0, Tx_Idx = 0;
vu8 NumbOfBytes;


void Master_Configuration(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);


    I2C_DeInit(I2C1);
    /* I2C1 Init */

    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x30;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = ClockSpeed;
    I2C_Init(I2C1, &I2C_InitStructure);
    I2C_ITConfig(I2C1, I2C_IT_ERR , ENABLE);

}



void I2C_Master_BufferRead(u8* pBuffer,  u16 NumByteToRead)

{

    /* Send START condition */
    I2C_GenerateSTART(I2C1, ENABLE);
    /* Test on EV5 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    /* Send slave address for read */
    I2C_Send7bitAddress(I2C1,  SLAVE_ADDRESS, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    /* While there is data to be read; here the safe procedure is implemented */
    while (NumByteToRead)
    {

        if (NumByteToRead != 3) /* Receive bytes from first byte until byte N-3 */
        {
            while ((I2C_GetLastEvent(I2C1) & 0x00004) != 0x000004); /* Poll on BTF */
            /* Read data */
            *pBuffer = I2C_ReceiveData(I2C1);
            pBuffer++;
            /* Decrement the read bytes counter */
            NumByteToRead--;
        }

        if (NumByteToRead == 3)  /* it remains to read three data: data N-2, data N-1, Data N */
        {

            /* Data N-2 in DR and data N -1 in shift register */
            while ((I2C_GetLastEvent(I2C1) & 0x000004) != 0x0000004); /* Poll on BTF */
            /* Clear ACK */
            I2C_AcknowledgeConfig(I2C1, DISABLE);
            //NVIC_SETPRIMASK();
            /* Read Data N-2 */
            *pBuffer = I2C_ReceiveData(I2C1);
            pBuffer++;
            /* Program the STOP */
            I2C_GenerateSTOP(I2C1, ENABLE);
            /* Read DataN-1 */
            *pBuffer = I2C_ReceiveData(I2C1);
          //  NVIC_RESETPRIMASK();
            pBuffer++;
            while ((I2C_GetLastEvent(I2C1) & 0x00000040) != 0x0000040); /* Poll on RxNE */
            /* Read DataN */
            *pBuffer = I2C1->DR;
            /* Reset the number of bytes to be read by master */
            NumByteToRead = 0;

        }
    }
    /* Make sure that the STOP bit is cleared by Hardware before CR1 write access */
    while ((I2C1->CR1&0x200) == 0x200);
    /* Enable Acknowledgement to be ready for another reception */
    I2C_AcknowledgeConfig(I2C1, ENABLE);
}

/*******************************************************************************
* Function Name  : I2C_Master_BufferWrite
* Description    : Send a buffer of bytes to the slave.
* Input          : - pBuffer: Buffer of bytes to be sent to the slave.
*                  - NumByteToRead: Number of bytes to be sent to the slave.
* Output         : None.
* Return         : None.
*******************************************************************************/
void I2C_Master_BufferWrite(u8* pBuffer,  u16 NumByteToWrite)

{


    /* Send START condition */
    I2C_GenerateSTART(I2C1, ENABLE);
    /* Test on EV5 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    /* Send slave address for write */
    I2C_Send7bitAddress(I2C1,SLAVE_ADDRESS, I2C_Direction_Transmitter);
    /* Test on EV6 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    I2C_SendData(I2C1, *pBuffer);
    pBuffer++;
    NumByteToWrite--;
    /* While there is data to be written */
    while (NumByteToWrite--)
    {
        while ((I2C_GetLastEvent(I2C1) & 0x04) != 0x04);  /* Poll on BTF */
        /* Send the current byte */
        I2C_SendData(I2C1, *pBuffer);
        /* Point to the next byte to be written */
        pBuffer++;

    }

    /* Test on EV8_2 and clear it, BTF = TxE = 1, DR and shift registers are
     empty */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    /* Send STOP condition */
    I2C_GenerateSTOP(I2C1, ENABLE);


}

/*******************************************************************************
* Function Name  : I2C_Master_BufferRead1Byte
* Description    : Read a byte from the slave.
* Input          : None.
* Output         : None.
* Return         : The read data byte.
*******************************************************************************/
u8 I2C_Master_BufferRead1Byte(void)
{
    u8 Data;
    vu32 temp;
    /* Send START condition */
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    /* Send EEPROM address for read */
    I2C_Send7bitAddress(I2C1, SLAVE_ADDRESS, I2C_Direction_Receiver);
    /* Wait until ADDR is set */
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR));
    /* Clear ACK */
    I2C_AcknowledgeConfig(I2C1, DISABLE);
    //NVIC_SETPRIMASK();
    /* Clear ADDR flag */
    temp = I2C1->SR2;
    /* Program the STOP */
    I2C_GenerateSTOP(I2C1, ENABLE);
    //NVIC_RESETPRIMASK();
    while ((I2C_GetLastEvent(I2C1) & 0x0040) != 0x000040); /* Poll on RxNE */
    /* Read the data */
    Data = I2C_ReceiveData(I2C1);
    /* Make sure that the STOP bit is cleared by Hardware before CR1 write access */
    while ((I2C1->CR1&0x200) == 0x200);
    /* Enable Acknowledgement to be ready for another reception */
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    return(Data);

}

/*******************************************************************************
* Function Name  : I2C_Master_BufferRead2Byte
* Description    : Read 2 data from the slave using Polling.
* Input          : - pBuffer : Buffer of bytes read from the slave.
* Output         : None
* Return         : None
*******************************************************************************/
void I2C_Master_BufferRead2Byte(u8* pBuffer)
{

    vu32 temp;
    /* Send START condition */
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    /* Send EEPROM address for read */
    I2C_Send7bitAddress(I2C1, SLAVE_ADDRESS, I2C_Direction_Receiver);
    I2C1->CR1 = 0xC01; /* ACK=1; POS =1 */
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR));
    //NVIC_SETPRIMASK();
    /* Clear ADDR */
    temp = I2C1->SR2;
    /* Disable ACK */
    I2C_AcknowledgeConfig(I2C1, DISABLE);
    //NVIC_RESETPRIMASK();
    while ((I2C_GetLastEvent(I2C1) & 0x0004) != 0x00004); /* Poll on BTF */
    //NVIC_SETPRIMASK();
    /* Program the STOP */
    I2C_GenerateSTOP(I2C1, ENABLE);
    /* Read first data */
    *pBuffer = I2C1->DR;
    pBuffer++;
    /* Read second data */
    *pBuffer = I2C1->DR;
    //NVIC_RESETPRIMASK();
    I2C1->CR1 = 0x0401; /* POS = 0, ACK = 1, PE = 1 */
}




/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
