/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : clock_calendar.h
* Author             : MCD Application Team
* Version            : V1.0.0
* Date               : 12/23/2008
* Description        : This files contains the Clock Calendar functions prototypes
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CLOCK_CALENDAR_H
#define __CLOCK_CALENDAR_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Time Structure definition */
struct Time_s
{
  u8 u8_SecLow;
  u8 u8_SecHigh;
  u8 u8_MinLow;
  u8 u8_MinHigh;
  u8 u8_HourLow;
  u8 u8_HourHigh;
};
extern struct Time_s s_TimeStructVar;

/* Alarm Structure definition */
struct AlarmTime_s
{
  u8 u8_SecLow;
  u8 u8_SecHigh;
  u8 u8_MinLow;
  u8 u8_MinHigh;
  u8 u8_HourLow;
  u8 u8_HourHigh;
};
extern struct AlarmTime_s s_AlarmStructVar;

/* Date Structure definition */
struct Date_s
{
  u8 u8_Month;
  u8 u8_Day;
  u16 u16_Year;
};
extern struct Date_s s_DateStructVar;

/* Alarm Date Structure definition */
struct AlarmDate_s
{
  u8 u8_Month;
  u8 u8_Day;
  u16 u16_Year;
};
extern struct AlarmDate_s s_AlarmDateStructVar;

extern vu32 vu32_TimingDelay;
extern u8 u8_TimeDateDisplay;
extern u8  u8_DisplayTimeFlag;
extern u32 u32_Dummy;
extern u8 u8_DisplayDateFlag;
extern u8  u8_AlarmStatus;
extern u8 u8_TotalMenuPointer;
extern u8 u8_FlagKey;
extern u8 u8_MenuLevelPointer;
extern u8 u8_TamperEvent;
extern u8 u8_TamperNumber;
extern float f32_Frequency;
extern u8 u8_BatteryRemoved;
extern u16 u16_SummerTimeCorrect;
extern u8 u8_AlarmDate;
extern u32 u32_TimerFrequency;
extern u8 u8_ArrayTime[8];


/* Exported constants --------------------------------------------------------*/
#define BATTERY_REMOVED 98
#define BATTERY_RESTORED 99
#define SECONDS_IN_DAY 86399
#define CONFIGURATION_DONE 0xAAAA
#define CONFIGURATION_RESET 0x0000
#define OCTOBER_FLAG_SET 0x4000
#define MARCH_FLAG_SET 0x8000
#define DEFAULT_DAY 15
#define DEFAULT_MONTH 5
#define DEFAULT_YEAR 2008
#define DEFAULT_HOURS 9
#define DEFAULT_MINUTES 24
#define DEFAULT_SECONDS 0
#define LEAP 1
#define NOT_LEAP 0
#define SYSTICK_FREQUENCY 480000
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void CalendarInit(void);
void RTC_Configuration(void);
void SetTime(u8,u8,u8);
void SetAlarm(u8,u8,u8);
void SetDate(u8,u8,u16);
void RTC_Application(void);
void DateUpdate(void);
u16 WeekDay(u16,u8,u8);
u8 CheckLeap(u16);
void CalculateTime(void);
void RTC_NVIC_Configuration(void);
void ApplicationInit(void);
void GPIO_Configuration(void);
void DelayLowPower(vu32 nCount);
void SysTick_ConfigX(void);
void CheckForDaysElapsed(void);
void SummerTimeCorrection(void);
void Tamper_NVIC_Configuration(void);
void ManualClockCalibration(void);
void AutoClockCalibration(void);
void ReturnFromStopMode(void);

void PVD_IRQHandler(void);
void TAMPER_IRQHandler(void);
void RTC_IRQHandler(void);


extern void atcIncrementCounter();
extern void atcResetCounter();
extern u32 atcGetCounter();
extern void GprsIdleIncMSec();
#endif /* __CLOCK_CALENDAR_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
