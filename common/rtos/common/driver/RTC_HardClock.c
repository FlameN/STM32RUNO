#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_bkp.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_usart.h"
#include "misc.h"
#include "RTC_HardClock.h"




/***************************************************************************//**
 * Declare function prototypes
 ******************************************************************************/

void RTC_Timer(void);
void RTC_IRQHandler(void);
void RTC_Configuration(void);
void SetHardClock(uint32 time);
uint32 GetHardClock();
/**
  * @brief  The Low Speed External (LSE) clock is used as RTC clock source.
  * The RTC clock can be output on the Tamper pin (PC.13). To enable this functionality,
  * uncomment the corresponding line: #define RTCClockOutput_Enable.
  * The RTC is in the backup (BKP) domain, still powered by VBAT when VDD is switched off,
  * so the RTC configuration is not lost if a battery is connected to the VBAT pin.
  * A key value is written in backup data register1 (BKP_DR1) to indicate if the RTC
  * is already configured.
  * @param  None
  * @retval None
  */
void RTC_Timer(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;


    /* NVIC configuration */
    NVIC_Configuration();

    if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
    {
        /* Backup data register value is not correct or not yet programmed (when
           the first time the program is executed) */

        //printf("\r\n\n RTC not yet configured....");

        /* RTC Configuration */
        RTC_Configuration();

        //printf("\r\n RTC configured....");

        /* Adjust time by values entred by the user on the hyperterminal */
        SetHardClock(0);

        BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
    }
    else
    {
        /* Check if the Power On Reset flag is set */
        if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
        {
            //printf("\r\n\n Power On Reset occurred....");
        }
        /* Check if the Pin Reset flag is set */
        else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
        {
            //printf("\r\n\n External Reset occurred....");
        }

        //printf("\r\n No need to configure RTC....");
        /* Wait for RTC registers synchronization */
        RTC_WaitForSynchro();

        /* Enable the RTC Second */
        RTC_ITConfig(RTC_IT_SEC, ENABLE);
        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();
    }

    /* Clear reset flags */
    RCC_ClearFlag();

    /* Display time in infinite loop */
    //Time_Show();
}

/**
  * @brief  This function handles RTC global interrupt request.
  * @param  None
  * @retval None
  */


/**
  * @brief  Configures the RTC.
  * @param  None
  * @retval None
  */




/**
  * @brief  Adjusts time.
  * @param  None
  * @retval None
  */
void SetHardClock(uint32 time)
{
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
    /* Change the current time */
    RTC_SetCounter(time);
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
}

uint32 GetHardClock()
{
	uint32_t time;
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
    /* Change the current time */
    time = RTC_GetCounter();
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
    return time;
}


