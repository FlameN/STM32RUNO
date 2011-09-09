/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : clock_calendar.c
* Author             : MCD Application Team
* Version            : V1.0.0
* Date               : 12/23/2008
* Description        : Clock Calendar basic routines
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "clock_calendar.h"
#include "hwinit.h"
#include "board.h"

u32 atcTickCounter;

/* Private variables--------------------------------------------------------- */
u8 u8_ClockSource;
u8 *u8_MonthsNames[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug",\
"Sep","Oct","Nov","Dec"};
const u8 u8_CalibrationPpm[128]={0,1,2,3,4,5,6,7,8,9,10,10,11,12,13,14,15,16,17,\
                         18,19,20,21,22,23,24,25,26,27,28,29,30,31,31,32,33,34,\
                         35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,51,\
                         52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,\
                         70,71,72,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,\
                         87,88,89,90,91,92,93,93,94,95,96,97,98,99,100,101,102,\
                         103,104,105,106,107,108,109,110,111,112,113,113,114,\
                         115,116,117,118,119,120,121};
/*Structure variable declaration for system time, system date,
alarm time, alarm date */
struct Time_s s_TimeStructVar;
struct AlarmTime_s s_AlarmStructVar;
struct Date_s s_DateStructVar;
struct AlarmDate_s s_AlarmDateStructVar;

u8 u8_DisplayCurrentPos;
u8 u8_DisplayPrevPos;
u32 u32_Dummy;
u16 u16_Counter=0;
u8 u8_ArrayTime[8]={0,0,0,0,0,0,0,0};
u8 u8_CounterPosition=0;
u8 u8_MenuLevelPointer=1;
u8 u8_TotalMenuPointer=1;
u8 u8_TamperNumber; /* Keeps the number of tamper Events already happened */
u8 u8_AlarmDate=0;
u8 u8_TimeDateDisplay=0;
u32 u32_TimerFrequency;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

u8 u8_DisplayDateFlag;
u8 u8_AlarmStatus;
vu32 vu32_TimingDelay;
u8 u8_FlagKey;
u8 u8_TamperEvent;
vu16 vu16_DutyCycle = 0;
float f32_Frequency = 0;
u8 u8_BatteryRemoved;
u16 u16_SummerTimeCorrect;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : RTC_Configuration
* Description    : COnfiguration of RTC Registers, Selection and Enabling of 
*                  RTC clock
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTC_Configuration()
{
  u16 u16_WaitForOscSource;
  
  /*Allow access to Backup Registers*/
  PWR_BackupAccessCmd(ENABLE);
  
  if(BKP_ReadBackupRegister(BKP_DR1)==CONFIGURATION_RESET)
  {
    /*Enables the clock to Backup and power interface peripherals    */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP | RCC_APB1Periph_PWR,ENABLE);

    /* Backup Domain Reset */
      BKP_DeInit();
      s_DateStructVar.u8_Month=DEFAULT_MONTH ;
      s_DateStructVar.u8_Day=DEFAULT_DAY;
      s_DateStructVar.u16_Year=DEFAULT_YEAR;
      u16_SummerTimeCorrect = OCTOBER_FLAG_SET;
      BKP_WriteBackupRegister(BKP_DR7,u16_SummerTimeCorrect);
      BKP_WriteBackupRegister(BKP_DR2,s_DateStructVar.u8_Month);
      BKP_WriteBackupRegister(BKP_DR3,s_DateStructVar.u8_Day);
      BKP_WriteBackupRegister(BKP_DR4,s_DateStructVar.u16_Year);
      BKP_WriteBackupRegister(BKP_DR1, CONFIGURATION_DONE);
      u8_TamperNumber=0;
      BKP_WriteBackupRegister(BKP_DR5,u8_TamperNumber);


    /*Enable 32.768 kHz external oscillator */
    RCC_LSEConfig(RCC_LSE_ON);
  
    for(u16_WaitForOscSource=0;u16_WaitForOscSource<5000;u16_WaitForOscSource++)
    {
    }

    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    /* RTC Enabled */
    RCC_RTCCLKCmd(ENABLE);
    RTC_WaitForLastTask();
    /*Wait for RTC registers synchronisation */
    RTC_WaitForSynchro();
    RTC_WaitForLastTask();
    /* Setting RTC Interrupts-Seconds interrupt enabled */
    /* Enable the RTC Second */
    RTC_ITConfig(RTC_IT_SEC , ENABLE);
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

      BKP_WriteBackupRegister(BKP_DR6, 1);
  
      /* Set RTC prescaler: set RTC period to 1 sec */
      RTC_SetPrescaler(32765); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */
      /* Prescaler is set to 32766 instead of 32768 to compensate for
        lower as well as higher frequencies*/
      /* Wait until last write operation on RTC registers has finished */
      RTC_WaitForLastTask();


 
    /* Set default system time to 09 : 24 : 00 */
    SetTime(DEFAULT_HOURS,DEFAULT_MINUTES,DEFAULT_SECONDS);
    BKP_WriteBackupRegister(BKP_DR1, CONFIGURATION_DONE);
  }
  else
  {
	  //BKP_WriteBackupRegister(BKP_DR1, CONFIGURATION_RESET);
    /* PWR and BKP clocks selection */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    for(u16_WaitForOscSource=0;u16_WaitForOscSource<5000;u16_WaitForOscSource++);
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
    /* Enable the RTC Second */
    RTC_ITConfig(RTC_IT_SEC, ENABLE);
    RTC_WaitForLastTask();
  }
    
  /* Check if how many days are elapsed in power down/Low Power Mode-
   Updates Date that many Times*/
  CheckForDaysElapsed();
  u8_ClockSource = BKP_ReadBackupRegister(BKP_DR6);
  u8_TamperNumber = BKP_ReadBackupRegister(BKP_DR5);
  s_DateStructVar.u8_Month = BKP_ReadBackupRegister(BKP_DR2);
  s_DateStructVar.u8_Day = BKP_ReadBackupRegister(BKP_DR3);
  s_DateStructVar.u16_Year = BKP_ReadBackupRegister(BKP_DR4);
  u16_SummerTimeCorrect = BKP_ReadBackupRegister(BKP_DR7);
  s_AlarmDateStructVar.u8_Month = BKP_ReadBackupRegister(BKP_DR8);
  s_AlarmDateStructVar.u8_Day = BKP_ReadBackupRegister(BKP_DR9);
  s_AlarmDateStructVar.u16_Year = BKP_ReadBackupRegister(BKP_DR10);
}

/*******************************************************************************
* Function Name  : SummerTimeCorrection
* Description    : Summer Time Correction routine
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SummerTimeCorrection(void)
{
  u8 u8_CorrectionPending=0;
  u8 u8_CheckCorrect=0;
  
  if((u16_SummerTimeCorrect & OCTOBER_FLAG_SET)!=0)
  {
    if((s_DateStructVar.u8_Month==10) && (s_DateStructVar.u8_Day >24 ))
    {
      for(u8_CheckCorrect = 25;u8_CheckCorrect <=s_DateStructVar.u8_Day;u8_CheckCorrect++)
      {
        if(WeekDay(s_DateStructVar.u16_Year,s_DateStructVar.u8_Month,u8_CheckCorrect )==0)
        {
          if(u8_CheckCorrect == s_DateStructVar.u8_Day)
          {
            /* Check if Time is greater than equal to 1:59:59 */
            if(RTC_GetCounter()>=7199)
            {
              u8_CorrectionPending=1;
            }
          }
          else
          {
            u8_CorrectionPending=1;
          }
         break;
       }
     }
   }
   else if((s_DateStructVar.u8_Month > 10))
   {
     u8_CorrectionPending=1;
   }
   else if(s_DateStructVar.u8_Month < 3)
   {
     u8_CorrectionPending=1;
   }
   else if(s_DateStructVar.u8_Month == 3)
   {
     if(s_DateStructVar.u8_Day<24)
     {
       u8_CorrectionPending=1;
     }
     else
     {
       for(u8_CheckCorrect=24;u8_CheckCorrect<=s_DateStructVar.u8_Day;u8_CheckCorrect++)
       {
         if(WeekDay(s_DateStructVar.u16_Year,s_DateStructVar.u8_Month,u8_CheckCorrect)==0)
         {
           if(u8_CheckCorrect == s_DateStructVar.u8_Day)
           {
             /*Check if Time is less than 1:59:59 and year is not the same in which
                March correction was done */
             if((RTC_GetCounter() < 7199) && ((u16_SummerTimeCorrect & 0x3FFF) != \
                                   s_DateStructVar.u16_Year))
             {
               u8_CorrectionPending=1;
             }
             else
             {
               u8_CorrectionPending=0;
             }
             break;
            }
            else
            {
              u8_CorrectionPending=1;
            }
          }
        }
      }
    }
  }
  else if((u16_SummerTimeCorrect & MARCH_FLAG_SET)!=0)
  {
    if((s_DateStructVar.u8_Month == 3) && (s_DateStructVar.u8_Day >24 ))
    {
      for(u8_CheckCorrect = 25;u8_CheckCorrect <=s_DateStructVar.u8_Day;\
         u8_CheckCorrect++)
      {
        if(WeekDay(s_DateStructVar.u16_Year,s_DateStructVar.u8_Month,\
           u8_CheckCorrect )==0)
        {
          if(u8_CheckCorrect == s_DateStructVar.u8_Day)
          {
            /*Check if time is greater than equal to 1:59:59 */
            if(RTC_GetCounter()>=7199)
            {
              u8_CorrectionPending=1;
            }
          }
          else
          {
            u8_CorrectionPending=1;
          }
        break;
        }
      }
    }
    else if((s_DateStructVar.u8_Month > 3) && (s_DateStructVar.u8_Month < 10 ))
    {
      u8_CorrectionPending=1;
    }
    else if(s_DateStructVar.u8_Month ==10)
    {
      if(s_DateStructVar.u8_Day<24)
      {
        u8_CorrectionPending=1;
      }
      else
      {
        for(u8_CheckCorrect=24;u8_CheckCorrect<=s_DateStructVar.u8_Day;\
          u8_CheckCorrect++)
        {
          if(WeekDay(s_DateStructVar.u16_Year,s_DateStructVar.u8_Month,\
            u8_CheckCorrect)==0)
          {
            if(u8_CheckCorrect == s_DateStructVar.u8_Day)
            {
              /*Check if Time is less than 1:59:59 and year is not the same in
              which March correction was done */
              if((RTC_GetCounter() < 7199) && \
                ((u16_SummerTimeCorrect & 0x3FFF) != s_DateStructVar.u16_Year))
              {
                u8_CorrectionPending=1;
              }
              else
              {
                u8_CorrectionPending=0;
              }
            break;
            }
          }
        }
      }
    }
  }

  if(u8_CorrectionPending==1)
  {
    if((u16_SummerTimeCorrect & OCTOBER_FLAG_SET)!=0)
    {
      /* Subtract 1 hour from the current time */
      RTC_SetCounter(RTC_GetCounter() - 3599);
      /* Reset October correction flag */
      u16_SummerTimeCorrect &= 0xBFFF;
      /* Set March correction flag  */
      u16_SummerTimeCorrect |= MARCH_FLAG_SET;
      u16_SummerTimeCorrect |= s_DateStructVar.u16_Year;
      BKP_WriteBackupRegister(BKP_DR7,u16_SummerTimeCorrect);
    }
    else if((u16_SummerTimeCorrect & MARCH_FLAG_SET)!=0)
    {
     /* Add 1 hour to current time */
     RTC_SetCounter(RTC_GetCounter() + 3601);
     /* Reset March correction flag */
     u16_SummerTimeCorrect &= 0x7FFF;
     /* Set October correction flag  */
     u16_SummerTimeCorrect |= OCTOBER_FLAG_SET;
     u16_SummerTimeCorrect |= s_DateStructVar.u16_Year;
     BKP_WriteBackupRegister(BKP_DR7,u16_SummerTimeCorrect);
    }
  }
}

/*******************************************************************************
* Function Name  : ApplicationInit
* Description    : Apllication Initialisation Routine
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ApplicationInit(void)
{
  /* System Clocks Configuration */
  //RCC_Configuration();
  /*Enables the clock to Backup and power interface peripherals    */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP | RCC_APB1Periph_PWR,ENABLE);
  /* SysTick Configuration*/
  SysTick_ConfigX();
  /*Initialisation of TFT LCD */
  //STM3210B_LCD_Init();
   /* Unlock the Flash Program Erase controller */
  //FLASH_Unlock();
  /*RTC_NVIC Configuration */
  RTC_NVIC_Configuration();
  /* RTC Configuration*/
  RTC_Configuration();
  BKP_RTCOutputConfig(BKP_RTCOutputSource_None);
  /* General Purpose I/O Configuration */
  //GPIO_Configuration();
  /* Battery Removal Emulation   */
  //GPIO_SetBits(GPIOC, GPIO_Pin_8);
  /*
  while(!(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13)))
  {
    u8_TamperEvent=1;
  }
  */
  /* Joystick NVIC Configuration  */
  //NVIC_JoyStickConfig();
  /* Tamper pin NVIC Configuration  */
  Tamper_NVIC_Configuration();
  /* Configure PVD Supervisor to disable the Tamper Interrupt when voltage drops 
  below 2.5 volts*/
  PWR_PVDCmd(ENABLE);
  PWR_PVDLevelConfig(PWR_PVDLevel_2V5);
  PWR_BackupAccessCmd(ENABLE);
  /* Om Sai Ram  */
  /* Only JoyStick Sel Interrupt is enabled on startup */
  //SelIntExtOnOffConfig(ENABLE);
  /* Tamper FeatureRTC  -   Enable Tamper Interrupt and configure for Low level */
  BKP_ITConfig(ENABLE);
  /* Enable Tamper Pin for Active low level: Tamper level detected for low level*/
  BKP_TamperPinLevelConfig(BKP_TamperPinLevel_Low);
  /* Enable tamper Pin Interrupt */
  BKP_TamperPinCmd(ENABLE);
  /*  Menu Initialisation  */
  //MenuInit();
}

/*******************************************************************************
* Function Name  : SetTime
* Description    : Sets the RTC Current Counter Value
* Input          : Hour, Minute and Seconds data
* Output         : None
* Return         : None
*******************************************************************************/
void SetTime(u8 u8_Hour,u8 u8_Minute,u8 u8_Seconds)
{
  u32 u32_CounterValue;
  u32_CounterValue=((u8_Hour * 3600)+ (u8_Minute * 60)+u8_Seconds);
  RTC_WaitForLastTask();
  RTC_SetCounter(u32_CounterValue);
  RTC_WaitForLastTask();
}

/*******************************************************************************
* Function Name  : SetAlarm
* Description    : Sets the RTC Alarm Register Value
* Input          : Hours, Minutes and Seconds data
* Output         : None
* Return         : None
*******************************************************************************/
void SetAlarm(u8 u8_Hour,u8 u8_Minute, u8 u8_Seconds)
{
  u32 u32_CounterValue;
  
  u32_CounterValue=((u8_Hour * 3600)+ (u8_Minute * 60)+u8_Seconds);
  
  if(u32_CounterValue == 0)
  {
    u32_CounterValue = SECONDS_IN_DAY;
  }

  RTC_WaitForLastTask();
  RTC_SetAlarm(u32_CounterValue);
  RTC_WaitForLastTask();
}

/*******************************************************************************
* Function Name  : SetDate
* Description    : Sets the RTC Date(DD/MM/YYYY)
* Input          : DD,MM,YYYY
* Output         : None
* Return         : None
*******************************************************************************/
void SetDate(u8 u8_Day, u8 u8_Month, u16 u16_Year)
{
  u32 u32_DateTimer;
  
  //RightLeftIntExtOnOffConfig(DISABLE);
  //UpDownIntOnOffConfig(DISABLE);

  /*Check if the date entered by the user is correct or not, Displays an error
    message if date is incorrect  */
  if((( u8_Month==4 || u8_Month==6 || u8_Month==9 || u8_Month==11) && u8_Day ==31) \
    || (u8_Month==2 && u8_Day==31)|| (u8_Month==2 && u8_Day==30)|| \
      (u8_Month==2 && u8_Day==29 && (CheckLeap(u16_Year)==0)))
  {

    u32_DateTimer=RTC_GetCounter();

    while((RTC_GetCounter()-u32_DateTimer)<2)
    {
    }
  }
  /* if date entered is correct then set the date*/
  else
  {
    if(u8_AlarmDate==0)
    {
    	s_DateStructVar.u16_Year = u16_Year;
    	s_DateStructVar.u8_Month = u8_Month;
    	s_DateStructVar.u8_Day = u8_Day;
      BKP_WriteBackupRegister(BKP_DR2,u8_Month);
      BKP_WriteBackupRegister(BKP_DR3,u8_Day);
      BKP_WriteBackupRegister(BKP_DR4,u16_Year);
      u8_DisplayDateFlag=1;
    }
    else
    {
     s_AlarmDateStructVar.u8_Day = u8_Day;
     s_AlarmDateStructVar.u8_Month = u8_Month;
     s_AlarmDateStructVar.u16_Year = u16_Year;
     BKP_WriteBackupRegister(BKP_DR8,u8_Month);
     BKP_WriteBackupRegister(BKP_DR9,u8_Day);
     BKP_WriteBackupRegister(BKP_DR10,u16_Year);
     u8_AlarmDate=0;
    }
  }
}

/*******************************************************************************
* Function Name  : RTC_Application
* Description    : RTC Application runs in while loop
* Input          : None
* Output         : Displays Time, Date and Alarm Status
* Return         : None
*******************************************************************************/
void RTC_Application(void)
{
  u8 u8_ReturnValue;

  CalculateTime();
  
  if(u8_TimeDateDisplay==0)
  {
    //DisplayTime();
  }

  if(u8_DisplayDateFlag==1 && u8_TimeDateDisplay==0)
  {
    //DisplayDate();
    u8_DisplayDateFlag=0;
  }

  if(u8_AlarmStatus==1)
  {
    //DisplayAlarm();
    GPIO_SetBits(GPIOC, GPIO_Pin_6);
    u32_Dummy=RTC_GetCounter();
    u8_AlarmStatus=2;
  }
  if(((RTC_GetCounter()-u32_Dummy) == 4) && (u8_AlarmStatus==2))
  {
    GPIO_ResetBits(GPIOC, GPIO_Pin_6);
    //SelIntExtOnOffConfig(ENABLE);
    //RightLeftIntExtOnOffConfig(ENABLE);
    //UpDownIntOnOffConfig(ENABLE);
    //MenuInit();
    u32_Dummy=0;
    u8_AlarmStatus = 0;
  }

  if(u8_TamperEvent==1) /* Tamper event is detected */
  {
    u8_TamperNumber = u8_TamperNumber + 1;
    BKP_WriteBackupRegister(BKP_DR5,u8_TamperNumber);
    BKP_WriteBackupRegister(BKP_DR1, CONFIGURATION_DONE);
    BKP_WriteBackupRegister(BKP_DR2,s_DateStructVar.u8_Month);
    BKP_WriteBackupRegister(BKP_DR3,s_DateStructVar.u8_Day);
    BKP_WriteBackupRegister(BKP_DR4,s_DateStructVar.u16_Year);
    BKP_WriteBackupRegister(BKP_DR9,s_AlarmDateStructVar.u8_Day);
    BKP_WriteBackupRegister(BKP_DR8,s_AlarmDateStructVar.u8_Month);
    BKP_WriteBackupRegister(BKP_DR10,s_AlarmDateStructVar.u16_Year);
    BKP_WriteBackupRegister(BKP_DR6,u8_ClockSource);
    BKP_WriteBackupRegister(BKP_DR7,u16_SummerTimeCorrect);
    /*
    u8_ReturnValue=EE_WriteVariable(u8_TamperNumber);
    
    if(u8_ReturnValue==PAGE_FULL)
    {
      u8_ReturnValue=EE_WriteVariable(u8_TamperNumber);
    }
    
    u8_ReturnValue=EE_WriteVariable(s_DateStructVar.u8_Day);
    u8_ReturnValue=EE_WriteVariable(s_DateStructVar.u8_Month);
    u8_ReturnValue=EE_WriteVariable((s_DateStructVar.u16_Year)/100);
    u8_ReturnValue=EE_WriteVariable((s_DateStructVar.u16_Year)%100);
    u8_ReturnValue=EE_WriteVariable((s_TimeStructVar.u8_HourHigh*10)+\
                    s_TimeStructVar.u8_HourLow);
    u8_ReturnValue=EE_WriteVariable((s_TimeStructVar.u8_MinHigh*10)+\
                    s_TimeStructVar.u8_MinLow);
    u8_ReturnValue=EE_WriteVariable((s_TimeStructVar.u8_SecHigh*10)+\
                    s_TimeStructVar.u8_SecLow);*/
    while(!(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13)))
    {
    }
    
    u8_TamperEvent=0;
    BKP_TamperPinCmd(ENABLE);
  }
  
  if(u8_BatteryRemoved != 0) /* Battery tamper is detected */
  {
    u8_TamperNumber = u8_TamperNumber + 1;
    BKP_WriteBackupRegister(BKP_DR5,u8_TamperNumber);
    /*
    u8_ReturnValue=EE_WriteVariable(u8_TamperNumber);
    
    if(u8_ReturnValue==PAGE_FULL)
    {
      u8_ReturnValue=EE_WriteVariable(u8_TamperNumber);
    }
    
    u8_ReturnValue=EE_WriteVariable(s_DateStructVar.u8_Day);
    u8_ReturnValue=EE_WriteVariable(s_DateStructVar.u8_Month);
    u8_ReturnValue=EE_WriteVariable((s_DateStructVar.u16_Year)/100);
    u8_ReturnValue=EE_WriteVariable((s_DateStructVar.u16_Year)%100);
    u8_ReturnValue=EE_WriteVariable((s_TimeStructVar.u8_HourHigh*10)\
                    +s_TimeStructVar.u8_HourLow);
    u8_ReturnValue=EE_WriteVariable((s_TimeStructVar.u8_MinHigh*10)\
                    +s_TimeStructVar.u8_MinLow);
    if(u8_BatteryRemoved==1)
    {
      u8_ReturnValue=EE_WriteVariable(BATTERY_REMOVED);
      u8_BatteryRemoved=0;
    }
    else if(u8_BatteryRemoved==2)
    {
      u8_ReturnValue=EE_WriteVariable(BATTERY_RESTORED);
      u8_BatteryRemoved=0;
    }
*/
    BKP_TamperPinCmd(ENABLE);
  }
}

/*******************************************************************************
* Function Name  : ReturnFromStopMode
* Description    : This function is executed after wakeup from STOP mode
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ReturnFromStopMode(void)
{
  /* RCC Configuration has to be called after waking from STOP Mode*/
  RCC_Configuration();
  /*Enables the clock to Backup and power interface peripherals after Wake Up */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP | RCC_APB1Periph_PWR,ENABLE);
  /* Enable access to Backup Domain */
  PWR_BackupAccessCmd(ENABLE);
  /* LCD Reinitialisation */
  //STM3210B_LCD_Init();
  /* LED D2 goes off */
  GPIO_ResetBits(GPIOC, GPIO_Pin_9); 
  /* Enable Sel interrupt */
  //SelIntExtOnOffConfig(ENABLE);
  /* Menu initialisation */
  //MenuInit();
  /* Time display enable */
  u8_TimeDateDisplay=0;
  /* Since Sel is used to exit from STOP mode, hence when STOP mode is exited
  initial value of MenuLevelPointer is 0 */
  u8_MenuLevelPointer=0xFF;
}

/*******************************************************************************
* Function Name  :DateUpdate
* Description    :Updates the Date (This function is called when 1 Day has elapsed
* Input          :None
* Output         :None
* Return         :None
*******************************************************************************/
void DateUpdate(void)
{
  s_DateStructVar.u8_Month=BKP_ReadBackupRegister(BKP_DR2);
  s_DateStructVar.u16_Year=BKP_ReadBackupRegister(BKP_DR4);
  s_DateStructVar.u8_Day=BKP_ReadBackupRegister(BKP_DR3);
  
  if(s_DateStructVar.u8_Month == 1 || s_DateStructVar.u8_Month == 3 || \
    s_DateStructVar.u8_Month == 5 || s_DateStructVar.u8_Month == 7 ||\
     s_DateStructVar.u8_Month == 8 || s_DateStructVar.u8_Month == 10 \
       || s_DateStructVar.u8_Month == 12)
  {
    if(s_DateStructVar.u8_Day < 31)
    {
      s_DateStructVar.u8_Day++;
    }
    /* Date structure member: s_DateStructVar.u8_Day = 31 */
    else
    {
      if(s_DateStructVar.u8_Month != 12)
      {
        s_DateStructVar.u8_Month++;
        s_DateStructVar.u8_Day = 1;
      }
     /* Date structure member: s_DateStructVar.u8_Day = 31 & s_DateStructVar.u8_Month =12 */
      else
      {
        s_DateStructVar.u8_Month = 1;
        s_DateStructVar.u8_Day = 1;
        s_DateStructVar.u16_Year++;
      }
    }
  }
  else if(s_DateStructVar.u8_Month == 4 || s_DateStructVar.u8_Month == 6 \
            || s_DateStructVar.u8_Month == 9 ||s_DateStructVar.u8_Month == 11)
  {
    if(s_DateStructVar.u8_Day < 30)
    {
      s_DateStructVar.u8_Day++;
    }
    /* Date structure member: s_DateStructVar.u8_Day = 30 */
    else
    {
      s_DateStructVar.u8_Month++;
      s_DateStructVar.u8_Day = 1;
    }
  }
  else if(s_DateStructVar.u8_Month == 2)
  {
    if(s_DateStructVar.u8_Day < 28)
    {
      s_DateStructVar.u8_Day++;
    }
    else if(s_DateStructVar.u8_Day == 28)
    {
      /* Leap Year Correction */
      if(CheckLeap(s_DateStructVar.u16_Year))
      {
        s_DateStructVar.u8_Day++;
      }
      else
      {
        s_DateStructVar.u8_Month++;
        s_DateStructVar.u8_Day = 1;
      }
    }
    else if(s_DateStructVar.u8_Day == 29)
    {
      s_DateStructVar.u8_Month++;
      s_DateStructVar.u8_Day = 1;
    }
  }
  
  BKP_WriteBackupRegister(BKP_DR2,s_DateStructVar.u8_Month);
  BKP_WriteBackupRegister(BKP_DR3,s_DateStructVar.u8_Day);
  BKP_WriteBackupRegister(BKP_DR4,s_DateStructVar.u16_Year);
  u8_DisplayDateFlag=1;
}

/*******************************************************************************
* Function Name  : CheckLeap
* Description    : Checks whether the passed year is Leap or not.
* Input          : None
* Output         : None
* Return         : 1: leap year
*                  0: not leap year
*******************************************************************************/
u8 CheckLeap(u16 u16_Year)
{
  if((u16_Year%400)==0)
  {
    return LEAP;
  }
  else if((u16_Year%100)==0)
  {
    return NOT_LEAP;
  }
  else if((u16_Year%4)==0)
  {
    return LEAP;
  }
  else
  {
    return NOT_LEAP;
  }
}

/*******************************************************************************
* Function Name  :WeekDay
* Description    :Determines the weekday
* Input          :Year,Month and Day
* Output         :None
* Return         :Returns the CurrentWeekDay Number 0- Sunday 6- Saturday
*******************************************************************************/
u16 WeekDay(u16 u16_CurrentYear,u8 u8_CurrentMonth,u8 u8_CurrentDay)
{
  u16 u16_Temp1,u16_Temp2,u16_Temp3,u16_Temp4,u16_CurrentWeekDay;
  
  if(u8_CurrentMonth < 3)
  {
    u8_CurrentMonth=u8_CurrentMonth + 12;
    u16_CurrentYear=u16_CurrentYear-1;
  }
  
  u16_Temp1=(6*(u8_CurrentMonth + 1))/10;
  u16_Temp2=u16_CurrentYear/4;
  u16_Temp3=u16_CurrentYear/100;
  u16_Temp4=u16_CurrentYear/400;
  u16_CurrentWeekDay=u8_CurrentDay + (2 * u8_CurrentMonth) + u16_Temp1 \
     + u16_CurrentYear + u16_Temp2 - u16_Temp3 + u16_Temp4 +1;
  u16_CurrentWeekDay = u16_CurrentWeekDay % 7;
  
  return(u16_CurrentWeekDay);
}



/*******************************************************************************
* Function Name  :CalculateTime
* Description    :Calcuate the Time (in hours, minutes and seconds  derived from
                  RTC COunter value
* Input          :None
* Output         :None
* Return         :None
*******************************************************************************/
void CalculateTime(void)
{
  u32 u32_TimeVar;
  
  u32_TimeVar=RTC_GetCounter();
  u32_TimeVar=u32_TimeVar % 86400;
  s_TimeStructVar.u8_HourHigh=(u8)(u32_TimeVar/3600)/10;
  s_TimeStructVar.u8_HourLow=(u8)(u32_TimeVar/3600)%10;
  s_TimeStructVar.u8_MinHigh=(u8)((u32_TimeVar%3600)/60)/10;
  s_TimeStructVar.u8_MinLow=(u8)((u32_TimeVar%3600)/60)%10;
  s_TimeStructVar.u8_SecHigh=(u8)((u32_TimeVar%3600)%60)/10;
  s_TimeStructVar.u8_SecLow=(u8)((u32_TimeVar %3600)%60)%10;
}

/*******************************************************************************
* Function Name  :CheckForDaysElapsed
* Description    :Chaeks is counter value is more than 86399 and the number of
                  days elapsed and updates date that many times
* Input          :None
* Output         :None
* Return         :None
*******************************************************************************/
void CheckForDaysElapsed(void)
{
  u8 u8_DaysElapsed;
 
  if((RTC_GetCounter() / SECONDS_IN_DAY) != 0)
  {
    for(u8_DaysElapsed = 0; u8_DaysElapsed < (RTC_GetCounter() / SECONDS_IN_DAY)\
         ;u8_DaysElapsed++)
    {
      DateUpdate();
    }

    RTC_SetCounter(RTC_GetCounter() % SECONDS_IN_DAY);
  }
}

/*******************************************************************************
* Function Name  : ManualClockCalibration
* Description    : Calibration of External crystal oscillator manually
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ManualClockCalibration(void)
{
	/*
  UpDownIntOnOffConfig(ENABLE);
  RightLeftIntExtOnOffConfig(ENABLE);
  SelIntExtOnOffConfig(DISABLE);
  LCD_Clear(Blue2);
  BKP_TamperPinCmd(DISABLE);
  BKP_RTCOutputConfig(BKP_RTCOutputSource_CalibClock);
  LCD_DisplayString(Line1,Column5,"Calibration");
  LCD_DisplayString(Line3,Column0,"LSE/64 is available");
  LCD_DisplayString(Line4,Column0,"on PC13.Measure the");
  LCD_DisplayString(Line5,Column0,"the frequency and");
  LCD_DisplayString(Line6,Column0,"press Sel to proceed");
  
  while(ReadKey()!=SEL)
  {
  }
  
  LCD_Clear(Blue2);
  BKP_RTCOutputConfig(BKP_RTCOutputSource_None);
  LCD_DisplayString(Line1,Column4,"Please enter");
  LCD_DisplayString(Line2,Column2,"Calibration Value");
  LCD_DisplayCount(Line4,Column6,1,u8_ArrayTime[0]+0x30);
  LCD_DisplayCount(Line4,Column7,0,u8_ArrayTime[1]+0x30);
  LCD_DisplayCount(Line4,Column8,0,u8_ArrayTime[2]+0x30);
  LCD_DisplayString(Line6,Column6,"(0-121)");
  LCD_SetBackColor(Green);
  SelIntExtOnOffConfig(ENABLE);*/
}

/*******************************************************************************
* Function Name  : AutoClockCalibration
* Description    : Calibration of External crystal oscillator auto(through Timer
                   peripheral)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void AutoClockCalibration(void)
{
  RCC_ClocksTypeDef ClockValue;
  u16 u16_TimerPrescalerValue=0x0003;
  u16 u16_CountWait;
  u16 u16_DeviationInteger;
  u32 u32_CalibrationTimer;
  float f32_Deviation;
  TIM_ICInitTypeDef  TIM_ICInitStructure;
  
  TIM_DeInit(TIM2);
  BKP_TamperPinCmd(DISABLE);
  BKP_RTCOutputConfig(BKP_RTCOutputSource_CalibClock);
  
  /* TIM2 configuration: PWM Input mode ------------------------
     The external signal is connected to TIM2 CH2 pin (PA.01),
     The Rising edge is used as active edge,
     The TIM2 CCR2 is used to compute the frequency value
     The TIM2 CCR1 is used to compute the duty cycle value
  ------------------------------------------------------------ */
  TIM_PrescalerConfig(TIM2,u16_TimerPrescalerValue,TIM_PSCReloadMode_Immediate);
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICFilter = 0x00;
  TIM_PWMIConfig(TIM2, &TIM_ICInitStructure);
  TIM_ICInit(TIM2, &TIM_ICInitStructure);
  /* Select the TIM2 Input Trigger: TI2FP2 */
  TIM_SelectInputTrigger(TIM2, TIM_TS_TI2FP2);
  /* Select the slave Mode: Reset Mode */
  TIM_SelectSlaveMode(TIM2, TIM_SlaveMode_Reset);
  /* Enable the Master/Slave Mode */
  TIM_SelectMasterSlaveMode(TIM2, TIM_MasterSlaveMode_Enable);
  /* TIM enable u16_Counter */
  TIM_Cmd(TIM2, ENABLE);

  /* Wait for 2 seconds */
  u32_CalibrationTimer = RTC_GetCounter();
 
  while((RTC_GetCounter() - u32_CalibrationTimer) < 2)
  {
  }
  
  RCC_GetClocksFreq(&ClockValue);
  u32_TimerFrequency=(ClockValue.PCLK1_Frequency * 2)/(u16_TimerPrescalerValue+1);
   /* Enable the CC2 Interrupt Request */
  TIM_ITConfig(TIM2, TIM_IT_CC2, ENABLE);
   /* Wait for 2 seconds */
  u32_CalibrationTimer = RTC_GetCounter();
  
  while((RTC_GetCounter() - u32_CalibrationTimer) < 2)
  {
  }

  if(!(TIM_GetFlagStatus(TIM2, TIM_FLAG_CC1)))
   /* There is no signal at the timer TIM2 peripheral input */
  {

  }
  else
  {
    /* Calulate Deviation in ppm  using the formula :
    Deviation in ppm = (Deviation from 511.968/511.968)*1 million*/
    if(f32_Frequency > 511.968)
    {
      f32_Deviation=((f32_Frequency-511.968)/511.968)*1000000;
    }
    else
    {
      f32_Deviation=((511.968-f32_Frequency)/511.968)*1000000;
    }
     u16_DeviationInteger = (u16)f32_Deviation;
    
    if(f32_Deviation >= (u16_DeviationInteger + 0.5))
    {
      u16_DeviationInteger = ((u16)f32_Deviation)+1;
    }
    
   u16_CountWait=0;

   /* Frequency deviation in ppm should be les than equal to 121 ppm*/
   if(u16_DeviationInteger <= 121)
   {
     while(u16_CountWait<128)
     {
       if(u8_CalibrationPpm[u16_CountWait] == u16_DeviationInteger)
       break;
       u16_CountWait++;
     }

     BKP_SetRTCCalibrationValue(u16_CountWait);

     u16_CountWait=u16_CountWait/10;

     u16_CountWait=u16_CountWait/10;
   
     if(u16_CountWait>0)
     {

     }
   }
   else /* Frequency deviation in ppm is more than 121 ppm, hence calibration
           can not be done */
   {

   }
  }
  
  BKP_RTCOutputConfig(BKP_RTCOutputSource_None);
  TIM_ITConfig(TIM2, TIM_IT_CC2, DISABLE);
  TIM_Cmd(TIM2, DISABLE);
  TIM_DeInit(TIM2);
  u32_CalibrationTimer=RTC_GetCounter();
  
  /*  Wait for 2 seconds  */
  while((RTC_GetCounter() - u32_CalibrationTimer) < 5)
  {
  }
  
  //MenuInit();
}

/*******************************************************************************
* Function Name  : Tamper_NVIC_Configuration
* Description    : Configures the Tamper Interrupts
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Tamper_NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = TAMPER_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
* Function Name  : RTC_NVIC_Configuration
* Description    : Configures RTC Interrupts
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTC_NVIC_Configuration(void)
{

  NVIC_InitTypeDef NVIC_InitStructure;
  //EXTI_InitTypeDef EXTI_InitStructure;
  /*
  EXTI_DeInit();
  NVIC_DeInit();
 */
#ifdef  VECT_TAB_RAM
  /* Set the Vector Table base location at 0x20000000 */

  //NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x00);
#else  /* VECT_TAB_FLASH  */
  /* Set the Vector Table base location at 0x08000000 */
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x00);
#endif

  /* Configure one bit for preemption priority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  /* Enable the RTC Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable the EXTI Line17 Interrupt */
  /*
  EXTI_ClearITPendingBit(EXTI_Line17);
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Line = EXTI_Line17;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
*/
  /* Enable the RTC Alarm Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
/*
  EXTI_ClearITPendingBit(EXTI_Line16 );
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Line = EXTI_Line16;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
*/
  NVIC_InitStructure.NVIC_IRQChannel = PVD_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable the TIM2 global Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
* Function Name  : SysTick_Config
* Description    : Configure a SysTick Base time to 10 ms.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SysTick_ConfigX(void)
{
  /* Configure HCLK clock as SysTick clock source */
  SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);

  /* SysTick interrupt each 100 Hz with HCLK equal to 72MHz */
  //SysTick_SetReload(SYSTICK_FREQUENCY);

  /* Enable the SysTick Interrupt */
  //SysTick_ITConfig(ENABLE);
}


void TIM2_IRQHandler(void)
{
  vu16 vu16_IC2Value = 0;

  /* Get the Input Capture value */
  vu16_IC2Value = TIM_GetCapture2(TIM2);

  if(vu16_IC2Value != 0)
  {
    /* Duty cycle computation */
    vu16_DutyCycle = (TIM_GetCapture1(TIM2) * 100) / vu16_IC2Value;

    /* Frequency computation */
    f32_Frequency = ((float)u32_TimerFrequency) / ((float)vu16_IC2Value);

    /* TIM  counter disable */
    TIM_Cmd(TIM2, DISABLE);
    /* Disable the CC2 Interrupt Request */
    TIM_ITConfig(TIM2, TIM_IT_CC2, DISABLE);
    TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);
  }
  else
  {
    vu16_DutyCycle = 0;
    f32_Frequency = 0;
  }
}

void SysTickHandler(void)
{
  if (vu32_TimingDelay != 0x00)
  {
    vu32_TimingDelay--;
  }
}



/*******************************************************************************
* Function Name  : PVD_IRQHandler
* Description    : This function handles PVD interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PVD_IRQHandler(void)
{
  BKP_TamperPinCmd(DISABLE);
  //EXTI_ClearITPendingBit(EXTI_Line16);
  //NVIC_ClearIRQChannelPendingBit(PVD_IRQn);
}

/*******************************************************************************
* Function Name  : TAMPER_IRQHandler
* Description    : This function handles Tamper interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TAMPER_IRQHandler(void)
{
  BKP_TamperPinCmd(DISABLE);
  u8_TamperEvent=1;
  BKP_ClearITPendingBit();
  BKP_ClearFlag();

  //NVIC_ClearIRQChannelPendingBit(TAMPER_IRQn);
}

/*******************************************************************************
* Function Name  : RTC_IRQHandler
* Description    : This function handles RTC global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTC_IRQHandler(void)
{
  u8 u8_Month,u8_Day;
  u16 u16_Year;

  atcIncrementCounter();
  GprsIdleIncMSec();

  u8_Month = BKP_ReadBackupRegister(BKP_DR2);
  u8_Day = BKP_ReadBackupRegister(BKP_DR3);
  u16_Year = BKP_ReadBackupRegister(BKP_DR4);



  //NVIC_ClearIRQChannelPendingBit(RTC_IRQn);
  RTC_ClearITPendingBit(RTC_IT_SEC);

  /* If counter is equal to 86399: one day was elapsed */
  /* This takes care of date change and resetting of counter in case of
  power on - Run mode/ Main supply switched on condition*/
  if(RTC_GetCounter() == 86399)
  {
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
    /* Reset counter value */
    RTC_SetCounter(0x0);
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

    /* Increment the date */
    DateUpdate();
  }

  if((RTC_GetCounter()/3600 == 1)&&(((RTC_GetCounter()%3600)/60) == 59)&&
     (((RTC_GetCounter()%3600)%60) == 59))
  {
    /* March Correction */
    if((u8_Month == 3) && (u8_Day >24))
    {
      if(WeekDay(u16_Year,u8_Month,u8_Day)==0)
      {
        if((u16_SummerTimeCorrect & 0x8000) == 0x8000)
        {
          RTC_SetCounter(RTC_GetCounter() + 3601);

          /* Reset March correction flag */
          u16_SummerTimeCorrect &= 0x7FFF;

          /* Set October correction flag  */
          u16_SummerTimeCorrect |= 0x4000;
          u16_SummerTimeCorrect |= u16_Year;
          BKP_WriteBackupRegister(BKP_DR7,u16_SummerTimeCorrect);
        }
      }
    }
      /* October Correction */
    if((u8_Month == 10) && (u8_Day >24))
    {
      if(WeekDay(u16_Year,u8_Month,u8_Day)==0)
      {
        if((u16_SummerTimeCorrect & 0x4000) == 0x4000)
        {
          RTC_SetCounter(RTC_GetCounter() - 3599);

          /* Reset October correction flag */
          u16_SummerTimeCorrect &= 0xBFFF;

          /* Set March correction flag  */
          u16_SummerTimeCorrect |= 0x8000;
          u16_SummerTimeCorrect |= u16_Year;
          BKP_WriteBackupRegister(BKP_DR7,u16_SummerTimeCorrect);
        }
      }
    }
  }
}

void atcIncrementCounter()
{
	atcTickCounter++;
}

void atcResetCounter()
{
	atcTickCounter = 0;
}



u32 atcGetCounter()
{
	return atcTickCounter;
}

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
