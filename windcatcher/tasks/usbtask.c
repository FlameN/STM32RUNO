#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
//#include "adc.h"
//#include "pwmcapture.h"
#include "rvnet.h"
#include "uart.h"
#include "crc.h"
#include "stm32f10x.h"
//#include "usb_lib.h"
//#include "usb_desc.h"
//#include "hw_config.h"
//#include "usb_pwr.h"


extern __IO uint32_t count_out;

__IO uint8_t PrevXferComplete = 1;
//extern uint8_t buffer_out[VIRTUAL_COM_PORT_DATA_SIZE];

/* Private variables ---------------------------------------------------------*/
USART_InitTypeDef USART_InitStructure;

typedef struct
{
	uint8 buf[5];
	uint16 size;
}CONTENTS;


void vUSBTask(void *pvParameters)
{
	uint16 usbsize;
	uint8 usbbuf[256];
	//uint8 usbbufsizes[100];
	CONTENTS stru[100];
	uint16 usbbufsizesCounter=0;
	bool st = false;
	/* The parameters are not used. */
	(void) pvParameters;
/*
	uint16_t bufff[20];
	bufff[0] = 0x555;
	bufff[1] = 0x444;
	bufff[2] = 0x333;
	bufff[3] = 0x222;
	bufff[4] = 0x111;

	FLASH_ErasePage(DEVICE_FLASHMEM_LOCATION);
	//FLASH_ProgramHalfWord(DEVICE_FLASHMEM_LOCATION,bufff[0]);

	//FLASH_ProgramHalfWord(DEVICE_FLASHMEM_LOCATION+0x02,bufff[1]);
	//FLASH_ProgramHalfWord(DEVICE_FLASHMEM_LOCATION+0x04,bufff[2]);
	//FLASH_ProgramHalfWord(DEVICE_FLASHMEM_LOCATION+0x06,bufff[3]);
	//FLASH_ProgramHalfWord(DEVICE_FLASHMEM_LOCATION+0x08,bufff[4]);
	WriteFlash( bufff,(void*)(DEVICE_FLASHMEM_LOCATION),  10);*/
	//vTaskDelay(15000);
	  Set_USBClock();
	  USB_Interrupts_Config();
	  USB_Init();
int heapSize = 0;
	  while (1)
	  {
			heapSize = xPortGetFreeHeapSize();
							heapSize +=0;
		  usbsize = UsbReceiveData(usbbuf,256);

		  if(usbsize > 0)
		  {

			  stru[usbbufsizesCounter].buf[0]=usbbuf[0];
			  stru[usbbufsizesCounter].buf[1]=usbbuf[1];
			  stru[usbbufsizesCounter].buf[2]=usbbuf[2];
			  stru[usbbufsizesCounter].buf[3]=usbbuf[3];
			  stru[usbbufsizesCounter].size = usbsize;
			  usbbufsizesCounter++;
			  if(usbbufsizesCounter>=100) usbbufsizesCounter = 0;
			  usbsize = RVnetSlaveProcess(usbbuf,usbsize,1);
			  if(usbsize > 0)
			  {
				  UsbSendData(usbbuf,usbsize);
			  }
		  }
		  vTaskDelay(1);
	  }
}

