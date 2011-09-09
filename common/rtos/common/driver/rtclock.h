#ifndef RTCLOCK_H
#define RTCLOCK_H
#include "typedef.h"

typedef struct
{
  uint16 Years;
  uint16 Month;
  uint16 Data;
  uint16 Day;
  uint16 Hour;
  uint16 Min;
  uint16 Sec;
  uint16 MSec;

}__attribute__((packed)) DATATIME;

extern void IncrementRTC(void);

extern void rtcGetDataTime(DATATIME *dt);
extern void rtcGetLocalDataTime(DATATIME *dt);
extern void rtcSetDataTime(DATATIME *dt);
extern void rtcSetLocalDataTime(DATATIME *dt);
extern void rtcIncrementSoftClockMs(void);
//extern bool rtcGetHardClock(DATATIME *dt);
extern void rtcInit();

extern void rtcGetiFixDateTime(char *dt);
extern void rtcSetiFixDateTime(char *dt);



extern void GetDateStringLM(char *buf);
extern void GetDateStringLD(char *buf);


#endif
