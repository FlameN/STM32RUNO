/*
 * modem.c
 *
 *  Created on: Jan 18, 2010
 *      Author: albert
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "dbg.h"
#include "modem.h"
#include "usart2.h"
#include "typedef.h"
#include "usermemory.h"

#define MODEM_SIO_BUFF_SIZE 1024

#define MODEM_ENABLE { GPIO_ResetBits(GPIOA, GPIO_Pin_1);}
#define  MODEM_DISABLE {GPIO_SetBits(GPIOA, GPIO_Pin_1); }

uint8 sio_buf[MODEM_SIO_BUFF_SIZE];

#define SIOFIFOSIZE 1024
uint8 sio_fifo[SIOFIFOSIZE];
//uint8 sio_buf[64];
volatile int bufpoint, bufsize;

#define STRBUFLEN 256

char strbuf[STRBUFLEN];
#define MODEM_PORT (&COM0)

const AT_COMMAND atset[AT_AMOUNT] =
{ "Call Ready", MA_CALLREADY, "OK", MA_OK, "SERVER OK", MA_SERVEROK,
		"NORMAL POWER DOWN", MA_NORMALPOWERDOWN, "STATE: IP STATUS",
		MA_STATEIPSTATUS, "STATE: IP CLOSE", MA_STATEIPCLOSE,
		"STATE: IP INITIAL", MA_STATEIPINITIAL, "STATE: IP GPRSACT",
		MA_STATEIPGPRSACT, "STATE: PDP DEACT", MA_STATEPDPDEACT, "> ",
		MA_CIPSENDREADY, "CONNECT", MA_CONNECT, "NO CARRIER", MA_NOCARRIER,
		"ERROR", MA_ERROR };

bool IsLiteral(char ch)
{
	if ((ch >= ' ') && (ch <= '~'))
	{
		return true;
	}
	return false;
}

void InitSioBuf()
{
	bufpoint = 0;
	bufsize = 0;
}

bool PopFromBuf(char *ch)
{
	if (bufpoint >= bufsize)
	{
		return false;
	}
	*ch = sio_buf[bufpoint];
	bufpoint++;
	return true;
}

void FillBufEx(char* abuf,int16_t* aSize)
{
	InitSioBuf();
	uint16_t time = 4000;
	uint16_t counter = 0;

	while(ReceivedMsg.flag == false && counter < time)
	{
		counter++;
		vTaskDelay(1);
	}


	if(ReceivedMsg.flag == true)
	{
		for(int i=0;i<ReceivedMsg.size;i++)
		{
			abuf[i] = ReceivedMsg.buffer[i];
		}
		*aSize = ReceivedMsg.size;
		ReceivedMsg.flag = false;
		ReceivedMsg.size = 0;
		//TIM_Cmd(TIM5, ENABLE);
	}
	//vTaskDelay(1000);
	//bufsize = uart2Read(sio_buf,256);

}

void FillBuf()
{
	InitSioBuf();
	uint16_t time = 1500;
	uint16_t counter = 0;

	while(ReceivedMsg.flag == false && counter < time)
	{
		counter++;
		vTaskDelay(1);
	}


	if(ReceivedMsg.flag == true)
	{
		for(int i=0;i<ReceivedMsg.size;i++)
		{
			sio_buf[i] = ReceivedMsg.buffer[i];
		}
		bufsize = ReceivedMsg.size;
		ReceivedMsg.size = 0;
		ReceivedMsg.flag = false;
		//TIM_Cmd(TIM5, ENABLE);
	}
	//vTaskDelay(1000);
	//bufsize = uart2Read(sio_buf,256);

}

bool ReadString(char *str, int size)
{
	char ch;
	if (bufpoint >= bufsize)
	{
		FillBuf();
	}
	do // remove non symbolic data
	{
		if (PopFromBuf(&ch) == false)
		{
			return false;
		}
	} while (!IsLiteral(ch));
	do
	{
		if (IsLiteral(ch))
		{
			*str = ch;
			str++;
		}
		else // packet end
		{
			*str = 0; //  add end of string
			return true;
		}
	} while (PopFromBuf(&ch) == true);
	return false;
}

MDEM_ANSWER GetModemAnswer(char *s)
{
	int i;
	if (*s == 0)
	{
		return MA_UNKNOWN;
	}
	for (i = 0; i < AT_AMOUNT; i++)
	{
		if (strcmp(atset[i].str, s) == 0)
		{
			return (atset[i].ma);
		}
	}
	return MA_UNKNOWN;
}

MDEM_ANSWER ReadModem()
{
	if (ReadString(strbuf, STRBUFLEN) == false)
	{
		return MA_UNKNOWN;
	}
	dbgmessage("from modem>");
	dbgmessage(strbuf);
	return GetModemAnswer((char*) strbuf);
}
/* ----------------------- Start implementation -----------------------------*/
void ModemWrite(char *str)
{
	dbgmessage("to modem>");
	dbgmessage(str);
	uint32 i = 0;
	char *tmpstr = str;
	while (*str)
	{
		str++;
		i++;
		if (i == 62)
			break;
	}
	//AT91F_USART_SEND(MODEM_PORT, (uint8*) tmpstr, i);
	uart2Write((uint8*) tmpstr, i);
}
bool WaitAnsver(MDEM_ANSWER ma)
{
	int i = 5;

	while (ReadModem() != ma)
	{
		if (i == 0)
		{
			return false;
		}
		i--;
	}
	//	dbgmessage("Ok\n");
	return true;
}

bool WaitCallReady()
{
	int i = 0;
	dbgmessage("Waiting for modem.");
	while (ReadModem() != MA_CALLREADY)
	{
		vTaskDelay(200);
		if (i > 7)
		{
			return false;
		}
		i++;
	};
	return true;
}
bool WaitOk()
{
	ModemWrite("AT\r");
	return WaitAnsver(MA_OK);
}

bool WaitAtd()
{
	ModemWrite("AT&D0\r");
	return WaitAnsver(MA_OK);
}

bool WaitAte()
{
	ModemWrite("ATE0V1\r");
	return WaitAnsver(MA_OK);
}

bool WaitGprsConnect()
{
	char myIp[17];
	char strsipstart[256];
	char buf[64];
	char header[] = "+IPD";
	int packSize;
	int headerSize;
	FLASHMEM *pxConfig = (FLASHMEM*) DEVICE_FLASHMEM_LOCATION;
	int i;

	ModemWrite("AT+CIPMODE=0\r");
	if (WaitAnsver(MA_OK) == false)
		return false;
	//	ModemWrite("AT+CSQ\r");
	//	WaitAnsver(MA_OK); // TODO replace wait answer with parsing level string
	//	if (WaitAnsver(MA_OK) == false)
	//	return false;

	//	ModemWrite("AT+CREG?\r");
	//	WaitAnsver(MA_OK);// TODO replace wait answer with parsing status
	//	if (WaitAnsver(MA_OK) == false)
	//	return false;

	strcpy(buf,"AT+CGDCONT=1,\"IP\",\"");
	strcat(buf,PPP_ADPARAM);
	strcat(buf,"\"\r");
	//sprintf(buf,PPP_ADDSTR,pxConfig->devcfg.gprs.adparam);

	ModemWrite(buf);

	if (WaitAnsver(MA_OK) == false)
		return false;
	if(strlen(pxConfig->devcfg.gprs.login)>16 || strlen(pxConfig->devcfg.gprs.password)>16 || strlen(pxConfig->devcfg.gprs.adparam)>32)
	    {
			strcpy(buf,"AT+CSTT=\"");
			strcat(buf,PPP_ADPARAM);
			strcat(buf,"\",\"");
			strcat(buf,PPP_USER);
			strcat(buf,"\",\"");
			strcat(buf,PPP_PASS);
			strcat(buf,"\"\r");
	    //sprintf(buf,PPP_CSTT,PPP_ADPARAM,PPP_USER,PPP_PASS);
	    }
	else
	    {
			strcpy(buf,"AT+CSTT=\"");
			strcat(buf,pxConfig->devcfg.gprs.adparam);
			strcat(buf,"\",\"");
			strcat(buf,pxConfig->devcfg.gprs.login);
			strcat(buf,"\",\"");
			strcat(buf,pxConfig->devcfg.gprs.password);
			strcat(buf,"\"\r");
	    //sprintf(buf,PPP_CSTT,pxConfig->devcfg.gprs.adparam,pxConfig->devcfg.gprs.login,pxConfig->devcfg.gprs.password);
	    }

	ModemWrite(buf);
	if (WaitAnsver(MA_OK) == false)
		return false;

	ModemWrite("AT+CMGF=1\r");
	if (WaitAnsver(MA_OK) == false)
		return false;

	ModemWrite("AT+CIPSPRT=2\r");
	if (WaitAnsver(MA_OK) == false)
		return false;

	ModemWrite("AT+CIPHEAD=1\r");
	if (WaitAnsver(MA_OK) == false)
		return false;

	ModemWrite("AT+CIICR\r");
	if (WaitAnsver(MA_OK) == false)
		return false;

	ModemWrite("AT+CIPCSGP=1,\"CMNET\"\r");
	if (WaitAnsver(MA_OK) == false)
		return false;

	ModemWrite("AT+CLPORT=\"TCP\",\"4444\"\r");
	if (WaitAnsver(MA_OK) == false)
		return false;
	ModemWrite("AT+CIFSR\r");
	WaitAnsver(MA_UNKNOWN);

	for (i = 0; i < 5; i++)
	{
		ModemWrite("AT+CIPSERVER\r");
		if (WaitAnsver(MA_OK) == true)
			break;
	}
	if (i > 4)
		return false;

	ModemWrite("AT+CIPCCON=2\r");
	if (WaitAnsver(MA_OK) == false)
		return false;
	dbgmessage("GPRS Connected");
	return true;
}

void ModemInit()
{
	//AT91F_USART_OPEN(AT91_USART_COM0_ID, 115200, AT91C_US_ASYNC_MODE);

	uart2Init(115200);
	USART2TIMConfigure(50);


	//------------------------------------------------------------

	while (1)
	{
		dbgmessage("!!!!!!!!!!!");
		dbgHeapFreeSize();
		uint16_t counter=0;
		//LED_MODE_GREEN(0);
		//LED_MODE_RED(1);


		dbgmessage("Init Modem.\r\n");
		dbgmessage("Modem ... ");
		MODEM_ENABLE;
		dbgmessage("Enabled ....  ");
		vTaskDelay(3000);
		MODEM_DISABLE;
		dbgmessage("Disable ....  ");
		vTaskDelay(10000);
		MODEM_ENABLE;
		vTaskDelay(1000);
		WaitCallReady();

		//LED_MODE_RED(0);
		//if (!WaitCallReady())
			//continue;

		while (!WaitOk() && counter++ < 11)
		{
			vTaskDelay(100);
		}
		if(counter >= 10)continue;

		//LED_MODE_RED(1);
		//LED_MODE_GREEN(1);
		vTaskDelay(50);
		if (!WaitAtd())
			continue;
		vTaskDelay(30);

		//LED_MODE_GREEN(1);
		if (!WaitAte())
			continue;
		vTaskDelay(50);
		//LED_MODE_GREEN(0);
		if (!WaitOk())
			continue;
		vTaskDelay(50);
		//LED_MODE_GREEN(1);

		if (!WaitGprsConnect())
			continue;
		vTaskDelay(50);
		//LED_MODE_GREEN(0);
		return;
	}
}
int ModemReceiveData(uint8 *buf, int maxsize)
{
	//LED_MODE_GREEN(1);
	dbgmessage("modem receive data:");
	char debugbuf[64];
	char header[] = "+IPD";
	int packSize;
	int headerSize;
	int i;
	//bufsize = AT91F_USART_RECEIVE(MODEM_PORT, (uint8*) sio_buf, SIOFIFOSIZE,
			//4000);

	FillBuf();
	if(bufsize == 0) return 0;

	for (i = 0; i < 4; i++)
	{
		if (sio_buf[i] != header[i])
		{
			return 0;
		}
	};

	dbgmessage(sio_buf);
	char* cmpbuf = "+IPD";
	char* strbufsize[16];
	int cmpPoint = 0;
	int strbufsizePoint = 0;
	//sscanf((char*) sio_buf, "+IPD%d:", &packSize);

	headerSize = 0;
	while (sio_buf[headerSize] != ':')
	{
		if(cmpPoint<4)
		{
			if(cmpbuf[cmpPoint] == sio_buf[headerSize])
			{
				cmpPoint++;
			}
		}else
		{
			strbufsize[strbufsizePoint++] = sio_buf[headerSize];
		}
		headerSize++;
	}
	strbufsize[strbufsizePoint]=0;
	//packSize = atoi(strbufsize);
	//char* c = &strbufsize[0];
	int it = 0;
	int res = 0;
	      while (strbufsize[it] >= '0' && strbufsize[it] <= '9')
	      {
	        res = res * 10 + strbufsize[it] - '0';
	        it++;
	      }
	packSize = res;

	headerSize++;
	while (bufsize < (packSize + headerSize))
	{

		int tmpsize;
		/*tmpsize = AT91F_USART_RECEIVE(MODEM_PORT, (uint8*) (&sio_buf[bufsize]),
				SIOFIFOSIZE - bufsize, 5000);*/
//********************************************************
	/*
		uint16_t time = 5000;
		uint16_t counter = 0;
		do
		{
			counter++;
			vTaskDelay(1);
		}while(ReceivedMsg.flag == false && counter < time);
		if(ReceivedMsg.flag == true)
		{
			for(int i=0;i<ReceivedMsg.size;i++)
			{
				sio_buf[i] = ReceivedMsg.buffer[i];
			}
			tmpsize =ReceivedMsg.size;
		}
*/
		FillBufEx((uint8*) (&sio_buf[bufsize]),tmpsize);
//********************************************************


		if (tmpsize == 0)
		{
		    //sprintf(debugbuf,"tmpsize:%i",tmpsize);
		    //dbgmessage(debugbuf);
			break;
		}
		bufsize += tmpsize;
	}
	i = 0;

	bufsize-=headerSize;

	//LED_MODE_GREEN(0);//led off

	//sprintf(debugbuf,"bufsize:%i",bufsize);
		//dbgmessage(debugbuf);
	while (bufsize)
	{
		if (i >= maxsize)
		{
			return 0;
		}
		*buf = sio_buf[i + headerSize];
		//sprintf(debugbuf,"msg[%i]:%i|%c",i,sio_buf[i + headerSize],(char)(sio_buf[i + headerSize]));
		//dbgmessage(debugbuf);
		buf++;
		i++;
		bufsize--;
	}



	return packSize;
}
void ModemSendData(uint8 *buf, uint8 size)
{
    //dbgmessage("modem send data:");
    char* tempbuf[6];
	//LED_MODE_GREEN(1);
	//LED_MODE_RED(1);
    //dbgmessage(buf);
	char strCipsend[20];
	strcpy(strCipsend,"AT+CIPSEND=");
	itoa(size,tempbuf,10);
	strcat(strCipsend,tempbuf);
	strcat(strCipsend,"\r");
	//sprintf(strCipsend, "AT+CIPSEND=%d\r", size);
	ModemWrite(strCipsend);
	//vTaskDelay(10);
	//AT91F_USART_SEND(MODEM_PORT, buf, size);
	uart2Write( buf, size);
//		WaitAnsver(MA_UNKNOWN); // listen here send ok or error ... no matter//
	//LED_MODE_GREEN(0);
	//LED_MODE_RED(0);
}

