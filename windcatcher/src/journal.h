/*
 * journal.h
 *
 *  Created on: 16.06.2010
 *      Author: nbohan
 */

#ifndef JOURNAL_H_
#define JOURNAL_H_
#include "typedef.h"
#include "error.h"

extern void itoa(int n, char s[]);
extern int atoi(char *c);
extern bool JrnlClear();
extern bool JrnlWrite(char *aMsg);
extern uint16 GetJrnlLength();
extern void GetDateString(char *aBuf);
extern void GetTimeString(char *aBuf);

extern void CheckCUSignal();
extern void CheckCUDirect();
extern void CheckCUPower();
extern void CheckErrors();
extern void CheckPowerOn();

#endif /* JOURNAL_H_ */
