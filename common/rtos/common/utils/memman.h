//********************************************************
//    Project      :
//    Version      :
//    Date         :
//    Author       : Kopachevsky Yuri
//    Company      : BEMN
//    Discription  :
//    Tools        : GNUARM compiller
//*********************************************************

#ifndef   MEMMAN_H
#define   MEMMAN_H

#include "typedef.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "usermemory.h"

typedef struct
  {
    uint32 startaddr;
    uint32 endaddr;
    xSemaphoreHandle xMutex;
    bool (*GetMem)( uint16, uint16*, uint16);
    bool (*SetMem)( uint16, uint16*, uint16);
  } MEMUNIT;

#ifndef MEMUNITS_NUM
#error "MEMUNITS_NUM must be defined"
#endif

extern MEMUNIT memunit[MEMUNITS_NUM];
extern bool MemGetWords( uint16, uint16 *, uint16);
extern bool MemSetWords( uint16, uint16 *, uint16);
extern bool MemManInit();

#endif

