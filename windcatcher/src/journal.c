/*
 * journal.c
 *
 *  Created on: June 16, 2010
 *      Author: Nick Bokhan
 *
 */

#include "journal.h"
#include "usermemory.h"
#include "rtclock.h"
#include "typedef.h"
#include "snprintf.h"
#define JRNLSIZE 0x17*170

extern size_t strlen(const char * str);
extern char * strcat(char * destination, const char * source);

uint16 jrnlLength;

void reverse(char *str)
    {
    char c;
    int size=strlen(str)/2;
    for(int i=0;i<size;i++)
	{
	c=str[i];
	str[i]=str[strlen(str)-i-1];
	str[strlen(str)-i-1]=c;
	}
    }

/* itoa:  конвертируем n в символы в s */
void itoa(int n, char s[])
    {
    int i, sign;

    if ((sign = n) < 0) /* записываем знак */
	n = -n; /* делаем n положительным числом */
    i = 0;
    do
	{ /* генерируем цифры в обратном порядке */
	s[i++] = n % 10 + '0'; /* берем следующую цифру */
	}
    while ((n /= 10) > 0); /* удаляем */
    if (sign < 0)
	s[i++] = '-';
    s[i] = '\0';
    reverse(s);
    }

int atoi(char *c) {
      int res = 0;
      while (*c >= '0' && *c <= '9')
      {
        res = res * 10 + *c++ - '0';
      }
      return res;
    }

void GetTimeString(char *aBuf)
    {
	DATATIME dtl;

	rtcGetDataTime(&dtl);
	char conv_buf[8];
	itoa(dtl.Hour,conv_buf);
	strcpy(aBuf,conv_buf);
	strcat(aBuf,":");
	itoa(dtl.Min,conv_buf);
	strcat(aBuf,conv_buf);
	strcat(aBuf,":");
	itoa(dtl.Sec,conv_buf);
	strcat(aBuf,conv_buf);
//	sprintf(aBuf,"%02i:%02i:%02i",dtl.Hour,dtl.Min,dtl.Sec);
    }

void GetDateString(char *aBuf)
    {
	DATATIME dtl;
	rtcGetDataTime(&dtl);
	char conv_buf[8];
	itoa(dtl.Data,conv_buf);
	strcpy(aBuf,conv_buf);
	strcat(aBuf,"/");
	itoa(dtl.Month,conv_buf);
	strcat(aBuf,conv_buf);
	strcat(aBuf,"/");
	itoa(dtl.Years,conv_buf);
	strcat(aBuf,conv_buf);
//	sprintf(aBuf,"%02i/%02i/%04i",dtl.Data,dtl.Month,dtl.Years);
    }

int JrnlClear()
    {
    jrnlLength = 1;
    uint32 buf[0x100]={0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,
	    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,
	    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,
	    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,
	    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,
	    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,
	    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,
	    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,
	    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0
    };

    for(int i=0;i<5;i++)
	{
//	    MemSetWords(0x2000+0x200*i, (uint16*)buf, 0x200);
	//    vTaskDelay(1000);
	}
    return MemSetWords(0x2000, &jrnlLength, 1);
    }

int JrnlWrite(char *aMsg)
    {

    uint16 testlen,wrLen;
    char buffer[64];
    char Date[16];
    char Time[16];

    GetDateString(Date);
    GetTimeString(Time);

	char conv_buf[8];
	strcpy(buffer,"<");
	strcat(buffer,Date);
	strcat(buffer,"><");
	strcat(buffer,Time);
	strcat(buffer,"><");
	strcat(buffer,aMsg);
	strcat(buffer,">");
    //sprintf(buffer,sizeof(buffer),"<%s><%s><%-22s>",Date,Time,aMsg);

    vTaskDelay(20);
    jrnlLength=GetJrnlLength();

    testlen =(strlen(buffer));

    if (jrnlLength > JRNLSIZE)
	{
	    JrnlClear();
	    return false;
	}
    vTaskDelay(20);
    MemSetWords((uint16)(0x2000 + jrnlLength), (uint16*) (buffer), 0x17);
    vTaskDelay(20);

    jrnlLength += 0x17;
    vTaskDelay(20);
    if (jrnlLength > JRNLSIZE)
   	{
	    jrnlLength=1;
   	}
    if(MemSetWords(0x2000, &jrnlLength, 1))
	{
	return true;
	}else
	    {
	    return false;
	    }
    }

uint16 GetJrnlLength()
    {
    //uint16 journaLength;
    //MemGetWords(0x2000, &jrnlLength, 1);
    vTaskDelay(20);
    //return jrnlLength;
    return 1;
    }


