#ifndef _USART_DEV
#define _USART_DEV
#include "typedef.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "board.h"
#include "error.h"

typedef struct
  {
    int id;
    AT91PS_USART hPort;
    ErrorHandle hError;
    xQueueHandle hRxQueue;
    xQueueHandle hTxQueue;
    AT91PS_PDC hPDC;
    int rxSize;
    int bufSize;
  } AT91_USART_DEV;

typedef enum
  {
  AT91_USART_COM0_ID = 0, AT91_USART_COM1_ID = 1, AT91_USART_COM2_ID = 2
  } AT91_USART_ID;

  extern AT91_USART_DEV COM0, COM1, COM2;
extern  void AT91F_US_START_RECEIVE(AT91PS_USART COM, uint8 *data, int size);
extern  void AT91F_USART_SEND(AT91_USART_DEV * , uint8 *, int);
extern   int AT91F_USART_RECEIVE(AT91_USART_DEV * , uint8 *, int,int);
extern  void AT91F_USART_SEND_COM0( uint8 *, int);
extern   int AT91F_USART_RECEIVE_COM0( uint8 *, int,int);

extern void AT91F_USART_OPEN(AT91_USART_ID, long, int);

#endif

