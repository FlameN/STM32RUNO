//
#ifndef _RELEY
#define _RELEY
#include "typedef.h"
#define RELAYS_AMOUNT   3

typedef uint8 RELAY;

#define RELAY_STATE_OFF  0x00
#define RELAY_STATE_ON   0x01
#define RELAY_STATE_SEAL 0x02

extern void ReleysReset(void);
extern void RelaySet(uint16,RELAY);
extern RELAY RelayGet(uint16);
extern void RelaysProcess(void);
#endif
