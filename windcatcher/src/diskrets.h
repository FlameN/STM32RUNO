//

#ifndef _DISKRETS
#define _DISKRETS

#include "typedef.h"

#define DISKRETS_AMOUNT   11

typedef uint8 DISKRETS;

#define DISKRET_STATE_OFF            0x00
#define DISKRET_STATE_ON             0x01
#define DISKRET_STATE_MAILFUNCTION   0xFF

#define DISKRET_NOISE                90
#define DISKRET_POSWAVE_TIME          1
#define DISKRET_NEGWAVE_TIME         40

extern void DiskretsProcess(void);
extern void DiskretSet(int,DISKRETS);
extern void DiskretsClear(void);
extern DISKRETS DiskretGet(int);

#endif
