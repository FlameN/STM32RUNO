//*----------------------------------------------------------------------------
//*         ATMEL Microcontroller Software Support  -  ROUSSET  -
//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : Debug.h
//* Object              : Debug menu
//* Creation            : JPP   02/Sep/2004
//*----------------------------------------------------------------------------

#ifndef dbgu_h
#define dbgu_h

//* ----------------------- External Function Prototype -----------------------

#include "board.h"


extern void Dbgu_init ( long );


extern void AT91F_DBGU_TRANSMIT(unsigned char *,int);
extern void AT91F_DBGU_RECEIVE(unsigned char *,int);
extern int AT91F_DBGU_RCR();
extern int AT91F_DBGU_TCR();


#endif /* dbgu_h */
