//*----------------------------------------------------------------------------
//*      ATMEL Microcontroller Software Support  -  ROUSSET  -
//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : lib_twi.h
//* Object
//* 1.1 31/Jan/05 JPP   : Clean For basic:
//*----------------------------------------------------------------------------

// Global declarations


extern void AT91F_TWI_Init(void);
extern int AT91F_TWI_Write (unsigned int,unsigned int, unsigned  char *, int);
extern int AT91F_TWI_Read  (unsigned int,unsigned int, unsigned  char *, int );
extern int AT91F_TWI_WriteByteAddress (unsigned int,unsigned int, unsigned  char *, int);
extern int AT91F_TWI_ReadByteAddress  (unsigned int,unsigned int, unsigned  char *, int );

