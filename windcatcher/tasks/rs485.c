#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "usermemory.h"
//#include "adc.h"
//#include "pwmcapture.h"
#include "iec61107.h"
#include "rvnet.h"
#include "uart.h"
//#include "serial.h"

#include "stm32f10x.h"
//#include "usb_lib.h"
//#include "usb_desc.h"
//#include "hw_config.h"
//#include "usb_pwr.h"

char rs485buf[256];
char tmpbuff[128];
RVNET_DATATYPE rs485size;
IDENTIFIER id;


bool AskConunter();

void ClearBuffer(int size)
{
  for(int i=0;i<size;i++)
  {

  }
}
uint16 ConvertUIP(char* string)
    {

    return (atof(string) / 400) * 65536;
    }

double new_atof(char* s)
{
 double sign = 1;
 double val = 0;
 int d = 0;
 if (s) while (((*s<'0' || *s>'9') && *s!='-' && *s!='+' && *s!='.') && *(++s));
 else return 0.0;
 sign*=(*s == '-' && ++s) ? -1.0 : ((*s == '+' && ++s),1.0);
 while ((*s >= '0' && *s <= '9') || (*s == '.' && d < 1))
 {
   val += *s == '.' ? (++d, 0.0) : d > 0 ? ((++d, val*=10.0), *s-'0') :(val*=10.0, *s-'0');
   ++s;
 }
 double den = 1;
 if(d > 0) while(--d) den*=10.0;
 return (val*sign)/den;
}

void MakeiFixUIP()
    {
    //Voltage

    RAM.UIPparams[0] = (uint16) (new_atof(&RAM.strings[8][0]) * 65535 / 400);
    RAM.UIPparams[1] = (uint16) (new_atof(&RAM.strings[9][0]) * 65535 / 400);
    RAM.UIPparams[2] = (uint16) (new_atof(&RAM.strings[10][0]) * 65535 / 400);

    //current
    RAM.UIPparams[3] = (uint16) (new_atof(&RAM.strings[11][0]) * 65535 / 100);
    RAM.UIPparams[4] = (uint16) (new_atof(&RAM.strings[12][0]) * 65535 / 100);
    RAM.UIPparams[5] = (uint16) (new_atof(&RAM.strings[13][0]) * 65535 / 100);
    //Power
    RAM.UIPparams[6] = (uint16) (new_atof(&RAM.strings[5][0]) * 65535 / 40);
    RAM.UIPparams[7] = (uint16) (new_atof(&RAM.strings[6][0]) * 65535 / 40);
    RAM.UIPparams[8] = (uint16) (new_atof(&RAM.strings[7][0]) * 65535 / 40);

    RAM.UIPparams[9] = ((uint32)(new_atof(&RAM.strings[4][0])*4294967296/999999))&0xFFFF;
    RAM.UIPparams[10] = (((uint32)(new_atof(&RAM.strings[4][0])*4294967296/999999))>>16)&0xFFFF;

    RAM.UIPparams[11] = ((uint32)(new_atof(&RAM.strings[15][0])*4294967296/999999))&0xFFFF;
    RAM.UIPparams[12] = (((uint32)(new_atof(&RAM.strings[15][0])*4294967296/999999))>>16)&0xFFFF;

    RAM.UIPparams[13] = ((uint32)(new_atof(&RAM.strings[16][0])*4294967296/60000))&0xFFFF;
    RAM.UIPparams[14] = (((uint32)(new_atof(&RAM.strings[16][0])*4294967296/60000))>>16)&0xFFFF;

    RAM.UIPparams[15] = ((uint32)(new_atof(&RAM.strings[17][0])*65535/2000));

    }

//extern uint8_t buffer_out[VIRTUAL_COM_PORT_DATA_SIZE];

/* Private variables ---------------------------------------------------------*/
USART_InitTypeDef USART_InitStructure;



void DMA_Configuration(void);

uint32_t T1, oldT1, cntT1;
uint32_t T2, oldT2, cntT2;


/*
uint8_t SerialGetBuffer(xComPortHandle pxPort, uint8_t* aBuffer)
{
	uint8_t count = 0;
	uint8_t bufvar = 0;
	while (xSerialGetChar(pxPort,&bufvar,50) == pdTRUE)
	{
		aBuffer[count++] = bufvar;
		vTaskDelay(10);
	}
	return count;
}*/

void vRs485Task(void *pvParameters)
{
	int bufsize = 512;
	uint8 buffer[bufsize];
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	uart3Init(9600/2);

	//xComPortHandle porthandle =  xSerialPortInitMinimal(9600,1024);
	buffer[0]='W';
	buffer[1]='A';
	buffer[2]='R';
	buffer[3]='E';
	buffer[4]='F';
	GPIO_ResetBits(GPIOB, GPIO_Pin_14);
int heapSize = 0;
		while (1)
			{

			heapSize = xPortGetFreeHeapSize();
			heapSize +=0;

			if (AskConunter())
			    {
			    vTaskDelay(10);

			    }
			else
			    {

			    rs485size = iecProcExitPacket(rs485buf);
			    GPIO_SetBits(GPIOB, GPIO_Pin_14);
			    vTaskDelay(1);
			    processBuffer8to7(rs485buf,rs485size);
			    uart3Write ((uint8*) rs485buf, rs485size);
			    vTaskDelay(300);
			    vTaskDelay(20);
			    }
			}

		/*
		signed char bufvar = 0;
		  while (1)
		    {
				  int cnt = uart3Read(buffer,bufsize);
				  vTaskDelay(10);
				  GPIO_SetBits(GPIOB, GPIO_Pin_14);
				  vTaskDelay(2);
				  if(cnt>0)
				  {
					 uart3Write(buffer,cnt);
					 //vTaskDelay(10);
					 //uart3Write(&buffer[1],1);
					 //uart3Write(&buffer[2],1);
				  }
				  vTaskDelay(5);
				  GPIO_ResetBits(GPIOB, GPIO_Pin_14);
				  vTaskDelay(10);
		    }
*/
}
/*
void writeBuffer(uint8_t* buffer,uint8_t  size)
{
	for(int i=0;i<size;i++)
	{
		USART_SendData (USART3, buffer++);
	}
}*/

void processBuffer7to8(char* buf,uint16 size)
{
	for(int i=0;i<size;i++)
	{
		buf[i]=buf[i]&0x7F;
	}
}

void processBuffer8to7(char* buf,uint16 size)
{
	int count=0;
	for(int i=0;i<size;i++)
	{
		count = 0;
		count += (buf[i]&0x01) ? 1 : 0;
		count += (buf[i]&0x02) ? 1 : 0;
		count += (buf[i]&0x04) ? 1 : 0;
		count += (buf[i]&0x08) ? 1 : 0;
		count += (buf[i]&0x10) ? 1 : 0;
		count += (buf[i]&0x20) ? 1 : 0;
		count += (buf[i]&0x40) ? 1 : 0;

		buf[i]=buf[i]|(count%2)<<7;
	}
}

bool SendReseive()
    {
    //  vTaskDelay(50);
	GPIO_SetBits(GPIOB, GPIO_Pin_14);
   // AT91F_PIO_SetOutput(AT91C_BASE_PIOA, LED_485);
    vTaskDelay(5);

    processBuffer8to7(rs485buf,rs485size);
    uart3Write ((uint8*) rs485buf, rs485size);
    vTaskDelay(20);
    //AT91F_PIO_ClearOutput(AT91C_BASE_PIOA, LED_485);

    GPIO_ResetBits(GPIOB, GPIO_Pin_14);
    vPortEnterCritical();
    vTaskDelay(1500);

    rs485size = uart3Read((uint8*) rs485buf, 256/*, 3000*/);
    vPortExitCritical();
    if (rs485size == 0)
	return false;
    processBuffer7to8(rs485buf,rs485size);
    vTaskDelay(50);
  //  AT91F_PIO_SetOutput(AT91C_BASE_PIOA, LED_485);
    vTaskDelay(100);
    return true;
    }
bool AskParam(char *paramName, char *value)
    {
    value[0] = 0;
    rs485size = iecProcVarReqPacket(rs485buf, paramName, 32);
    if (!SendReseive())
	return false;

    uint8 boolX = iecProcVarAnswerPacket(rs485buf, value, 128);
    if(boolX == false)
	{
    	return false;
	}

    return true;
    }

bool AskParamArg(char *paramName, char *value,char *arg)
    {
    value[0] = 0;
    rs485size = iecProcVarReqPacketArg(rs485buf, paramName, 32,arg);
    if (!SendReseive())
	return false;
    uint8 boolX = iecProcVarAnswerPacket(rs485buf, value, 128);
    if(boolX == false)
   	{
       	return false;
   	}
    return true;
    }
char* ParseAnswer(char *buf, char *dsn)
    {
    while ( *buf != '(')
	buf++;
    buf++;
    while (*buf && *buf != ')')
	{
	*dsn = *buf;
	dsn++;
	buf++;
	}
    *dsn = 0;
    return buf;
    }
bool AskConunter()
    {

    char *pxBuf;
    char tmpbuffX[16];
    // Find Device
    rs485size = iecProcReqPacket(rs485buf, "", 10);
    if (!SendReseive())
	return false;
    uint8 boolX = iecIndProc(rs485buf, &id);
    if (false==boolX)
    {
    	return false;
    }

    // Password & security
    rs485size = iecOptionsAckPacket(rs485buf, '0', '5', '1');
    if (!SendReseive())
	return false;
    boolX = iecProcAddrAnswer(rs485buf, &RAM.strings[0][0], 128);
    if (false==boolX)
	{
		return false;
	}
    //#define IEC_PASS "777777"
    char* pass=((FLASHMEM*) DEVICE_FLASHMEM_LOCATION)->devcfg.logica.reserv;
    if (strlen((char*)pass)>0 && strlen((char*)pass)<15)
	{
	rs485size = iecProcPassPacket(rs485buf, (char*)(((FLASHMEM*) DEVICE_FLASHMEM_LOCATION)->devcfg.logica.reserv), 16);
	if(!SendReseive())return false;
	if(rs485buf[0] != ACK)return false;
	}

    if (!AskParam("DATE_", tmpbuff))
	return false;
    ParseAnswer(tmpbuff, &RAM.strings[1][0]);

    if (!AskParam("TIME_", tmpbuff))
	return false;
    ParseAnswer(tmpbuff, &RAM.strings[2][0]);

    if (!AskParam("SNUMB", tmpbuff))
	return false;
    ParseAnswer(tmpbuff, &RAM.strings[3][0]);

    if (!AskParam("ET0PE", tmpbuff))
	return false;
    ParseAnswer(tmpbuff, &RAM.strings[4][0]);

    if (!AskParam("POWPP", tmpbuff))
	return false;
    pxBuf = tmpbuff;
    pxBuf = ParseAnswer(pxBuf, &RAM.strings[5][0]);
    pxBuf = ParseAnswer(pxBuf, &RAM.strings[6][0]);
    pxBuf = ParseAnswer(pxBuf, &RAM.strings[7][0]);

    if (!AskParam("VOLTA", tmpbuff))
	return false;
    pxBuf = tmpbuff;
    pxBuf = ParseAnswer(pxBuf, &RAM.strings[8][0]);
    pxBuf = ParseAnswer(pxBuf, &RAM.strings[9][0]);
    pxBuf = ParseAnswer(pxBuf, &RAM.strings[10][0]);

    if (!AskParam("CURRE", tmpbuff))
	return false;
    pxBuf = tmpbuff;
    pxBuf = ParseAnswer(pxBuf, &RAM.strings[11][0]);
    pxBuf = ParseAnswer(pxBuf, &RAM.strings[12][0]);
    pxBuf = ParseAnswer(pxBuf, &RAM.strings[13][0]);

    if (!AskParam("FREQU", tmpbuff))
	return false;
    pxBuf = tmpbuff;
    pxBuf = ParseAnswer(pxBuf, &RAM.strings[14][0]);

    if (!AskParam("ET0PE", tmpbuff))
   	return false;
    ParseAnswer(tmpbuff, &RAM.strings[15][0]);



    GetDateStringLM(tmpbuffX);
    if (!AskParamArg("ENMPE", tmpbuff,tmpbuffX))
         	return false;
    ParseAnswer(tmpbuff, &RAM.strings[16][0]);

    GetDateStringLD(tmpbuffX);
    if (!AskParamArg("ENDPE", tmpbuff,tmpbuffX))
          	return false;
    ParseAnswer(tmpbuff, &RAM.strings[17][0]);

       MakeiFixUIP();

    rs485size = iecProcExitPacket(rs485buf);
    GPIO_SetBits(GPIOB, GPIO_Pin_14);
    //AT91F_PIO_SetOutput(AT91C_BASE_PIOA, USART0_RTS);
    uart3Write((uint8*) rs485buf, rs485size);
    vTaskDelay(300);
    return true;
    }
