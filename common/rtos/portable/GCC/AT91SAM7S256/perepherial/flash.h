#ifndef FLASH_H
#define FLASH_H
#include "typedef.h"
/*-------------------------------*/
/* Flash Status Field Definition */
/*-------------------------------*/
//#define FLASHFUNC __attribute__ ((long_call,section(".ramsection")))
#define FLASHFUNC __attribute__ ((section (".fastrun")))


#define 	AT91C_MC_FSR_MVM 		((unsigned int) 0xFF << 8)		// (MC) Status Register GPNVMx: General-purpose NVM Bit Status
#define 	AT91C_MC_FSR_LOCK 		((unsigned int) 0xFFFF << 16)	// (MC) Status Register LOCKSx: Lock Region x Lock Status

#define	 ERASE_VALUE 		0xFFFFFFFF

/*-----------------------*/
/* Flash size Definition */
/*-----------------------*/
/*256 kbytes of Internal High-speed Flash, organized in 1024 Pages of 256 Bytes (AT91SAM7S256)*/

#define  FLASH_PAGE_SIZE_BYTE	256
//#define  FLASH_PAGE_SIZE_LONG	32

//#define  FLASH_LOCK_BITS_SECTOR	16
//#define  FLASH_SECTOR_PAGE		32
//#define  FLASH_LOCK_BITS		16    /* 16 lock bits, each protecting 16 sectors of 32 pages*/
//#define  FLASH_BASE_ADDRESS		0x00100000
/*------------------------------*/
/* External function Definition */
/*------------------------------*/
/* Flash function */
extern void Flash_Init(void);
extern bool Flash_WriteBuffer(uint8 *pxFlash, uint8 *buf, uint16 len);
extern bool Flash_Write(uint16 page, unsigned int *pagebuff)FLASHFUNC;
#endif /* FLASH_H */

