// Include Standard files
#include "board.h"
#include "flash.h"
#include "FreeRTOS.h"

int Flash_Ready(void) FLASHFUNC

//*----------------------------------------------------------------------------
//* \fn    AT91F_Flash_Init
//* \brief Flash init
//*----------------------------------------------------------------------------
void Flash_Init(void)
    {
    *AT91C_MC_FMR = ((AT91C_MC_FMCN) & (72 << 16)) | AT91C_MC_FWS_1FWS;
    }

bool Flash_Write(uint16 page, uint8 *pagebuff)
    {
    uint8 *flash;
    unsigned int i;
    portENTER_CRITICAL();
	{
	AT91C_BASE_MC->MC_FCR = (AT91C_MC_CORRECT_KEY | AT91C_MC_FCMD_UNLOCK
		|
		(AT91C_MC_PAGEN & (page << 8)));
	while ((AT91C_BASE_MC->MC_FSR & AT91C_MC_EOL) != AT91C_MC_EOL)
	    ;

	flash = (uint8 *)( 0x00100000 + page * FLASH_PAGE_SIZE_BYTE);

	for (i = 0; i < FLASH_PAGE_SIZE_BYTE; i++)
	    {
	    flash[i] = *pagebuff;
	    pagebuff++;
	    }
	AT91C_BASE_MC->MC_FCR = (AT91C_MC_CORRECT_KEY
		| AT91C_MC_FCMD_START_PROG | (AT91C_MC_PAGEN & (page << 8)));
	while ((AT91C_BASE_MC->MC_FSR & AT91C_MC_EOP) != AT91C_MC_EOP)
	    ;
	}
    portEXIT_CRITICAL();
    return true;
    }

//*----------------------------------------------------------------------------
inline void * Flash_GetPageStartAddress(void *addr)
    {
    return (void*) ((int) addr - ((int) addr % FLASH_PAGE_SIZE_BYTE));
    }

void * Flash_InitPageBuffer(uint8 *pBuff, void *addr)
    {
    uint8 *pxData;
    // Initialize temp buffer
    pxData = Flash_GetPageStartAddress(addr);
    for (int i = 0; i < FLASH_PAGE_SIZE_BYTE; i++)
	{
	*pBuff = *pxData;
	pxData++;
	pBuff++;
	}
    return Flash_GetPageStartAddress(addr);
    }

//*----------------------------------------------------------------------------
bool Flash_WriteBuffer(uint8 *pxFlash, uint8 *buf, uint16 len)
    {
    uint16 i;
    uint8 pagebuff[FLASH_PAGE_SIZE_BYTE];
    uint32 offset = (int) pxFlash - (int) Flash_GetPageStartAddress(pxFlash);
    void *pxCurPageAddr;
    while (len)
	{
	pxCurPageAddr = Flash_InitPageBuffer(pagebuff, pxFlash);
	// Copy to pbuf
	for (i = offset; i < FLASH_PAGE_SIZE_BYTE; i++)
	    {
	    pagebuff[i] = *buf;
	    buf++;
	    pxFlash++;
	    len--;
	    if (len == 0)
		break;
	    }
	offset = 0;
	// write page
	Flash_Write(((int) (pxCurPageAddr) / FLASH_PAGE_SIZE_BYTE), pagebuff);
	}

    return true;
    }

