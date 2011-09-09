#ifndef RTCHARDCLOCK_H
#define RTCHARDCLOCK_H
#include "typedef.h"



extern void RTC_Timer(void);
extern void RTC_IRQHandler(void);
extern void RTC_Configuration(void);
extern void SetHardClock(uint32 time);
extern uint32 GetHardClock();


#endif
