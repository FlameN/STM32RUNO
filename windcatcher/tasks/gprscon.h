#ifndef _GPRSCONTASK
#define _GPRSCONTASK

#define vGprsConTask_STACK_SIZE		800
//#define vGprsConTask_PRIORITY           ( tskIDLE_PRIORITY + 6 )
/*
typedef enum
  {
  CONNECTING, CONNECTED, DISCONNECT
  } ePPPThreadControl;
*/


extern void vGprsConTask(void *pvParameters )  __attribute__((naked));
//extern ePPPThreadControl GetPPPThreadControl(void);
//extern bool TCPPool( void );


#endif
