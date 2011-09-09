#include "rtclock.h"
//#include "RTC_HardClock.h"
//#include "journal.h"
#include "stm32f10x_i2c.h"
#include "board.h"
#include "clock_calendar.h"

#define RTCCLOCK_CORRECTOR 14975
#define RTC_DADR      (0x68)

DATATIME RTClock[2];
bool iFixLocal;

unsigned int RTCCorrector = 0;

uint32 atcTickCounter;
uint32 atcCounterHZ;
bool switcher=true;


bool rtcSetHardClock(DATATIME *dt);
bool rtcGetHardClock(DATATIME *dt);
void rtcUpdateLocalDT();
//--------Private functions---------
int GetYearsFromSecondsFrom2000(uint32 time);
//----------------------------------


unsigned char month_day_table[] =
    {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
uint16 month_day_tableAcc[] =
    {
    31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 375
    };
unsigned char leap_year(unsigned int test_year);
//пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ 1, пїЅпїЅпїЅпїЅ пїЅпїЅпїЅ test_year - пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ
unsigned char leap_year(unsigned int test_year)
    {
    if (((test_year % 4 == 0) && (test_year % 100)) || (test_year % 400 == 0))
	return 1;
    else
	return 0;
    }
unsigned int at91_bcd_int(unsigned char bcd)
    {
    return ((bcd & 0x0F) + (((bcd & 0xF0) >> 4) * 10));

    }

unsigned int GetDayOfWeek(DATATIME* dt)
    {
    unsigned char Mounths[]={5,1,1,4,6,2,4,0,3,5,1,3};


    int DayOfWeek = (dt->Years+(int)(dt->Years/4)+ Mounths[dt->Month-1] + dt->Data);

    if(leap_year(dt->Years) == 1 && dt->Month < 2)
	DayOfWeek= (DayOfWeek-1)%7;
    else
	DayOfWeek= (DayOfWeek)%7;
    if(DayOfWeek == 0) DayOfWeek = 7;
    return DayOfWeek;

    }
unsigned char at91_int_bcd(unsigned int value)
    {
    char tmp[2];
    tmp[1] = (value % 10) + 0;
    value /= 10;
    tmp[0] = (value % 10) + 0;
    return ((tmp[1] & 0x0F) | ((tmp[0] & 0x0F) << 4));
    }

void rtcInit(void)
    {

    rtcGetHardClock(&RTClock[0]);
    if (RTClock[0].MSec > 999)
	{
	RTClock[0].MSec = 0;
	}
    if (RTClock[0].Sec > 59)
	{
	RTClock[0].Sec = 0;
	}
    if (RTClock[0].Min > 59)
	{
	RTClock[0].Min = 0;
	}
    if (RTClock[0].Hour > 23)
	{
	RTClock[0].Hour = 0;
	}
    if (RTClock[0].Day > 7)
	{
	RTClock[0].Day = 1;
	}
    if (RTClock[0].Data > 31)
	{
	RTClock[0].Data = 1;
	}
    rtcUpdateLocalDT();
    }

bool rtcSetHardClock(DATATIME *dt)
    {
	SetTime(dt->Hour,dt->Min,dt->Sec);
	SetDate(dt->Data,dt->Month,dt->Years);
    return true;
    }

bool rtcGetHardClock(DATATIME *dt)
{
	//DateUpdate();
	CalculateTime();
	dt->Years = s_DateStructVar.u16_Year;
	dt->Data = s_DateStructVar.u8_Day;
	dt->Month = s_DateStructVar.u8_Month;

	dt->Hour = ((uint16)(s_TimeStructVar.u8_HourHigh)*10) + (uint16)(s_TimeStructVar.u8_HourLow);
	dt->Min = ((uint16)(s_TimeStructVar.u8_MinHigh)*10) + (uint16)(s_TimeStructVar.u8_MinLow);
	dt->Sec = ((uint16)(s_TimeStructVar.u8_SecHigh)*10) + (uint16)(s_TimeStructVar.u8_SecLow);

	/*
    uint32 timeSec = GetHardClock();

    dt->MSec = 0;
    dt->Sec = timeSec%60;
    dt->Min = (timeSec/60)%60;
    dt->Hour = (timeSec/(60*60))%24;
    dt->Day = ((timeSec/(60*60*24))+2)%7;//0- понедельник
    dt->Years = GetYearsFromSecondsFrom2000(timeSec);
    int16 daysCurYear = (timeSec-(dt->Years*31536000+((uint32)(dt->Years/4)+1)*86400))/86400;
    int lYear = leap_year(dt->Years);

    for(int i=0;i<12;i++)
    {
    	if(daysCurYear>month_day_table[i]+(i==1)?lYear:0)
    		daysCurYear-=month_day_table[i]+(i==1)?lYear:0;
    	else
    	{
    		dt->Month=i+1;
    		dt->Data = daysCurYear;
    	}
    }

	dt->Years+=2000;*/
    return true;
}

int GetYearsFromSecondsFrom2000(uint32 timeSec)
{
	uint16 predvYear = (uint32)(timeSec/31536000); //31536000 Seconds in leap year
	if(predvYear!=(timeSec+((uint32)(predvYear/4)+1)*86400)/31536000)//86400 Seconds in one day
		predvYear++;
	return predvYear;
}

void rtcGetDataTime(DATATIME *dt)
    {
    //rtcUpdateLocalDT();
	rtcGetHardClock(dt);
	/*
    dt->MSec = RTClock[0].MSec;
    dt->Sec = RTClock[0].Sec;
    dt->Min = RTClock[0].Min;
    dt->Hour = RTClock[0].Hour;
    dt->Day = RTClock[0].Day;
    dt->Data = RTClock[0].Data;
    dt->Month = RTClock[0].Month;
    dt->Years = RTClock[0].Years;

    dt[1].MSec = RTClock[1].MSec;
    dt[1].Sec = RTClock[1].Sec;
    dt[1].Min = RTClock[1].Min;
    dt[1].Hour = RTClock[1].Hour;
    dt[1].Day = RTClock[1].Day;
    dt[1].Data = RTClock[1].Data;
    dt[1].Month = RTClock[1].Month;
    dt[1].Years = RTClock[1].Years;*/
    /*for(i=0;i<28;i++)
     {
     dt->ram[i] = RTClock.ram[i];
     }*/
    }
void rtcGetLocalDataTime(DATATIME *dt)
    {
	rtcGetHardClock(&RTClock[0]);
    rtcUpdateLocalDT();
    dt->MSec = RTClock[1].MSec;
    dt->Sec = RTClock[1].Sec;
    dt->Min = RTClock[1].Min;
    dt->Hour = RTClock[1].Hour;
    dt->Day = RTClock[1].Day;
    dt->Data = RTClock[1].Data;
    dt->Month = RTClock[1].Month;
    dt->Years = RTClock[1].Years;
    }

void rtcUpdateLocalDT()
    {
    int shift = 0;//our shift amount
    if (RTClock[0].Month > 3 && RTClock[0].Month < 10)
	{
	shift = 1;
	}
    if (RTClock[0].Month == 3 && RTClock[0].Data > 24 && RTClock[0].Day == 7 && RTClock[0].Hour>1)
	{
	shift = 1;
	}

    if (RTClock[0].Month == 3 && RTClock[0].Data > 24 && RTClock[0].Day != 7 && (7 - RTClock[0].Day)>(31-RTClock[0].Data) )
   	{
   	shift = 1;
   	}


    if (RTClock[0].Month == 10 && ((RTClock[0].Data <= 24) || (RTClock[0].Data > 24 && RTClock[0].Day != 7 &&(7 - RTClock[0].Day)<=(31-RTClock[0].Data))))
	{
	shift = 1;
	}

    if (RTClock[0].Month == 10 && RTClock[0].Data > 24 && RTClock[0].Day == 7 && RTClock[0].Hour<1 )
   	{
   	shift = 1;
   	}

    RTClock[1].MSec = RTClock[0].MSec;
    RTClock[1].Sec = RTClock[0].Sec;
    RTClock[1].Min = RTClock[0].Min;
    RTClock[1].Hour = RTClock[0].Hour + shift;
    if (RTClock[1].Hour > 23)
	{
	RTClock[1].Hour = 0;
	}
    else
	{
	shift = 0;
	}

    RTClock[1].Day = RTClock[0].Day + shift;
    if (RTClock[1].Day > 7)
	{
	RTClock[1].Day = 1;
	}
    RTClock[1].Data = RTClock[0].Data + shift;
    if (shift && RTClock[1].Data > month_day_table[RTClock[0].Month-1])
	{
	RTClock[1].Data = 1;
	}
    else
	{
	shift = 0;
	}
    RTClock[1].Month = RTClock[0].Month + shift;
    RTClock[1].Years = RTClock[0].Years;
    /*for (i = 0; i < 28; i++)
     {
     RTClock[1].ram[i] = RTClock[0].ram[i];
     }*/

    }

void rtcSetDataTime(DATATIME *dt)
    {
    dt->Day = GetDayOfWeek(dt);
    rtcSetHardClock(dt);

    RTClock[0].MSec = dt->MSec;
    RTClock[0].Sec = dt->Sec;
    RTClock[0].Min = dt->Min;
    RTClock[0].Hour = dt->Hour;
    RTClock[0].Day = dt->Day;
    RTClock[0].Data = dt->Data;
    RTClock[0].Month = dt->Month;
    RTClock[0].Years = dt->Years;
    rtcUpdateLocalDT();
    /*for(i=0;i<28;i++)
     {
     RTClock.ram[i] = dt->ram[i];
     }*/
    }

void rtcSetLocalDataTime(DATATIME *dt)
    {
    dt->Day = GetDayOfWeek(dt);
    int shift = 0;//our shift amount

    if (dt->Month > 3 && dt->Month < 10)
	{
	shift = 1;
	}
    if (dt->Month == 3 && dt->Data > 24 && dt->Day == 7 && dt->Hour>1)
	{
	shift = 1;
	}

    if (dt->Month == 3 && dt->Data > 24 && dt->Day != 7 && (7 - dt->Day)>(31-dt->Data) )
   	{
   	shift = 1;
   	}


    if (dt->Month == 10 && ((dt->Data <= 24) || (dt->Data > 24 && dt->Day != 7 &&(7 - dt->Day)<=(31-dt->Data))))
	{
	shift = 1;
	}

    if (dt->Month == 10 && dt->Data > 24 && dt->Day == 7 && dt->Hour<1 )
   	{
   	shift = 1;
   	}

    RTClock[0].MSec = dt->MSec;
    RTClock[0].Sec = dt->Sec;
    RTClock[0].Min = dt->Min;

    if(shift == 1)
	{
	if (dt->Hour == 0)
	    {
	    RTClock[0].Hour = 23;
	    }
	else
	    {
	    RTClock[0].Hour = dt->Hour - shift;
	    shift = 0;
	    }
	}else
	    {
	    RTClock[0].Hour = dt->Hour;
	    }

    if(shift == 1)
    	{
    if (dt->Day == 1)
	{
	RTClock[0].Day = 7;
	}else
	    {
	    RTClock[0].Day = dt->Day - shift;
	    }
    	}else
    	    {
    	    RTClock[0].Day = dt->Day;
    	    }

    if(shift == 1)
        {
	if (dt->Data == 1)
	    {
	    RTClock[0].Data = month_day_table[dt->Month - 2];
	    }
	else
	    {
	    RTClock[0].Data = dt->Data - shift;
	    shift = 0;
	    }
        }else
            {
            RTClock[0].Data = dt->Data;
            }

    RTClock[0].Month = dt->Month - shift;
    RTClock[0].Years = dt->Years;
    /*for (i = 0; i < 28; i++)
     {
     RTClock[0].ram[i] = dt->ram[i];
     }*/
    rtcSetHardClock(&RTClock[0]);
    }

void rtcSync(void)
    {
    rtcSetHardClock(&RTClock[0]);
    }

void rtcIncrementSoftClockMs(void)
    {
    unsigned char day_tmp;
    RTCCorrector++;
    if (RTCCorrector == RTCCLOCK_CORRECTOR)
	{
	RTCCorrector = 0;
	rtcSync();
	}
    else
	{
	RTClock[0].MSec++;
	if (RTClock[0].MSec > 999)
	    {
	    RTClock[0].MSec = 0;
	    RTClock[0].Sec++;
	    if (RTClock[0].Sec > 59)
		{
		RTClock[0].Sec = 0;
		RTClock[0].Min++;
		if (RTClock[0].Min > 59)
		    {
		    RTClock[0].Min = 0;
		    RTClock[0].Hour++;
		    if (RTClock[0].Hour > 23)
			{

			RTClock[0].Hour = 0;
			RTClock[0].Day++;
			if (RTClock[0].Day > 7)
			    {
			    RTClock[0].Day = 1;
			    }
			RTClock[0].Data++;

			if (leap_year(RTClock[0].Years) && (RTClock[0].Month
				== 2))
			    {
			    day_tmp = 29;
			    }
			else
			    {
			    day_tmp = month_day_table[RTClock[0].Month - 1];
			    }
			if (RTClock[0].Data > day_tmp)
			    {
			    RTClock[0].Data = 1;
			    RTClock[0].Month++;
			    if (RTClock[0].Month > 12)
				{
				RTClock[0].Month = 1;
				RTClock[0].Years++;
				}
			    }
			}
		    }
		}
	    }

	}
    }

//-----------------------------------------------------------

void itoaFix(int n, char s[])
    {
    n = n % 100;

    s[0] = n / 10+48;
    s[1] = n % 10+48;
    }

void rtcGetiFixDateTime(char *dt)
    {
    char buffer[2];
    DATATIME dtC[2];

    dt[0] = 48;
    //if (iFixLocal)
	dt[1] = 49;
    //else
	//dt[1] = 48;

    //if (iFixLocal)
	rtcGetLocalDataTime(&dtC);
    //else
	//rtcGetDataTime(dtC);

    itoaFix(dtC->Years, buffer);
    dt[2] = buffer[0];
    dt[3] = buffer[1];
    itoaFix(dtC->Month, buffer);
    dt[4] = buffer[0];
    dt[5] = buffer[1];
    itoaFix(dtC->Data, buffer);
    dt[6] = buffer[0];
    dt[7] = buffer[1];
    itoaFix(dtC->Hour, buffer);
    dt[8] = buffer[0];
    dt[9] = buffer[1];
    itoaFix(dtC->Min, buffer);
    dt[10] = buffer[0];
    dt[11] = buffer[1];
    itoaFix(dtC->Sec, buffer);
    dt[12] = buffer[0];
    dt[13] = buffer[1];
    itoaFix(dtC->MSec, buffer);
    dt[14] = buffer[0];
    dt[15] = buffer[1];
    }

void rtcSetiFixDateTime(char *dt)
    {
    char buffer[3];
    buffer[2] = 0;
    DATATIME dtC;

    if (dt[1] == 49)
	iFixLocal = true;
    else
	iFixLocal = false;

    buffer[0] = dt[2];
    buffer[1] = dt[3];
    dtC.Years = atoi(buffer);

    buffer[0] = dt[4];
    buffer[1] = dt[5];
    dtC.Month = atoi(buffer);

    buffer[0] = dt[6];
    buffer[1] = dt[7];
    dtC.Data = atoi(buffer);

    buffer[0] = dt[8];
    buffer[1] = dt[9];
    dtC.Hour = atoi(buffer);

    buffer[0] = dt[10];
    buffer[1] = dt[11];
    dtC.Min = atoi(buffer);

    buffer[0] = dt[12];
    buffer[1] = dt[13];
    dtC.Sec = atoi(buffer);

    buffer[0] = dt[14];
    buffer[1] = dt[15];
    dtC.MSec = atoi(buffer);

    if(dtC.Years > 70) dtC.Years+=1900;
           else
      dtC.Years+=2000;

    if (iFixLocal)
	rtcSetLocalDataTime(&dtC);
    else
	rtcSetDataTime(&dtC);
    }

//-------------------------------------------------------------
void atcRebaseCounter()
    {
    atcCounterHZ = atcTickCounter;
    }





void GetDateStringLM(char *aBuf)
    {

    DATATIME dtl;
      char tempBuf[16];
      char overalBuff[32];
      rtcGetDataTime(&dtl);
      overalBuff[0]='\0';
      int month = dtl.Month;
      int year = dtl.Years;
      if(year>2000) year-=2000;
      if(month==1)
	  {
	  month=12;
	  if(year!=0) year--;
	  }
      else
	  {
	  month--;
	  }

      if(month<10)strcat((char *) overalBuff, "0");
      itoa(month, tempBuf);
      strcat(strcat((char *) overalBuff, (char *) tempBuf), ".");
      if(year<10) strcat((char *) overalBuff, "0");
      itoa(year, tempBuf);
      strcat(overalBuff,tempBuf);


      CopyDataBytes((uint8*) overalBuff, (uint8*) aBuf, strlen(overalBuff)+1);
    }

void GetDateStringLD(char *aBuf)
    {
    DATATIME dtl;
      char tempBuf[16];
      char overalBuff[32];
      rtcGetLocalDataTime(&dtl);
      overalBuff[0]='\0';
      int month = dtl.Month;
      int year = dtl.Years;
      int data = dtl.Data;
      if(year>2000) year-=2000;

      if(data==1)
	  {
	  data=month_day_table[(month+10)%12]+(month==1)?((uint8)leap_year(year)):(0);

	  if(month==1)
	 	  {
	 	  month=12;
	 	  if(year!=0) year--;
	 	  }
	       else
	 	  {
	 	  month--;
	 	  }
	  }
      else
	  {
	  data--;
	  }

      if(data<10)strcat((char *) overalBuff, "0");
      itoa(data, tempBuf);
      strcat(strcat((char *) overalBuff, (char *) tempBuf), ".");
      if(month<10)strcat((char *) overalBuff, "0");
      itoa(month, tempBuf);
      strcat(strcat((char *) overalBuff, (char *) tempBuf), ".");
      if(year<10) strcat((char *) overalBuff, "0");
      itoa(year, tempBuf);
      strcat((char *) overalBuff, (char *) tempBuf);


      CopyDataBytes((uint8*) overalBuff, (uint8*) aBuf, strlen(overalBuff)+1);

    }
