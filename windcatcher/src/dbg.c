#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "typedef.h"

#define ADDRETURN 1;
#include "pppdebug.h"
//#include "dbgupdc.h"
char strbuffer[128];
//xSemaphoreHandle xdbgMutex;

void dbgInit()
    {

    //xdbgMutex = xSemaphoreCreateMutex();
    //Dbgu_init(230400);
    }

void dbgmessage(char *str)
    {/*
    char outbuffer[256];
    char *buf;
    int i;
   if (xdbgMutex != NULL)
	{
	if (xSemaphoreTake(xdbgMutex, (portTickType) 100) == pdTRUE)
	    {
	    i=0;
	    buf = outbuffer;
	    while (*str)
		{
		if (i > 255)
		    return;
		*buf = *str;
#ifdef ADDRETURN
		if ((*buf) == '\n')
		    {
		    *buf = '\r';
		    buf++;
		    i++;
		    if (i > 255)
			return;
		    *buf = '\n';
		    }
#endif
		buf++;
		str++;
		i++;
		}

	    //AT91F_DBGU_TRANSMIT((unsigned char *) outbuffer, i);
	    //while (AT91F_DBGU_TCR() != 0)
		//{
		//vTaskDelay(1);
		//};
	    }
	xSemaphoreGive(xdbgMutex);
	}
*/
    }
void dbgPrintArray(int bstart, int bend, unsigned char buff[])
    {/*
    char *pxstrbuff = strbuffer;
    int len = bend - bstart;
    int i;
    int tmp;
    //vsprintf(pxstrbuff, "\n Array %04d bytes:", &len);
    dbgmessage(strbuffer);

    for (i = bstart; i < bend; i++)
	{
	tmp = buff[i];
	if(i*5>120)break;
	snprintf(pxstrbuff + i * 5, 120 , "0x%02X,", tmp);

	}
    dbgmessage(strbuffer);
*/
   }

void strace(const char *tstr, __const char *__restrict format, ...)
    {/*
    char *pxstrbuff = strbuffer;
//    va_list ap;
  //  va_start(ap, format);
  //  vsnprintf(pxstrbuff, 200, format, ap);
  //  va_end(ap);
  //  dbgmessage(strbuffer);
    dbgmessage(format);*/
    }

void trace(__const char *__restrict format, ...)
    {/*
    char *pxstrbuff = strbuffer;
    va_list ap;
    va_start(ap, format);
    vsnprintf(pxstrbuff, 120, format, ap);
    va_end(ap);
    dbgmessage(strbuffer);*/
    }

void ppp_trace(int level, __const char *__restrict format, ...)
    {/*
    char *pxstrbuff = strbuffer;
     va_list ap;
    va_start(ap, format);
    vsnprintf(pxstrbuff, 120, format, ap);
    va_end(ap);
    dbgmessage(strbuffer);
   // dbgmessage(format);
    if (level != 6)
	{
	dbgmessage("\r");
	} //LOG_DETAIL*/
    }

void lvip_trace(int level, __const char *__restrict format, ...)
    {/*
    char *pxstrbuff = strbuffer;
    va_list ap;
    va_start(ap, format);
    vsnprintf(pxstrbuff, 120, format, ap);
    va_end(ap);
    dbgmessage(strbuffer);
  //  dbgmessage(format);
    if (level != 6)
	{
	dbgmessage("\r");
	} //LOG_DETAIL*/
    }
void dbgStackFreeSize(char *str)
    {
	/*
	unsigned portBASE_TYPE uxHighWaterMark;
      uxHighWaterMark =  uxTaskGetStackHighWaterMark(NULL);
      uxHighWaterMark += 0;

      dbgmessage(str);
      trace(" - Stack Rem: %d \n\r",uxHighWaterMark);*/
    }
void dbgHeapFreeSize()
    {
    int heapSize = 0;
    heapSize = xPortGetFreeHeapSize();
    heapSize += 0;
    //trace("\n\rHeap free  %d. \r\n", heapSize);
    }
