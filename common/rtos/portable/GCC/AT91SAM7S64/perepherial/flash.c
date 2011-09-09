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
    //* Set number of Flash Waite sate
    //  SAM7S64 features Single Cycle Access at Up to 30 MHz
    //  if MCK = 47923200, 50 Cycles for 1 µseconde ( field MC_FMR->FMCN)
    //        AT91C_BASE_MC->MC_FMR = ((AT91C_MC_FMCN)&(50 <<16)) | AT91C_MC_FWS_1FWS ;
    AT91C_BASE_MC->MC_FMR = ((AT91C_MC_FMCN) & (72 << 16)) | AT91C_MC_FWS_1FWS;
  }
//*----------------------------------------------------------------------------
//* \fn    AT91F_Flash_Ready
//* \brief Wait the flash ready
//*----------------------------------------------------------------------------
int Flash_Ready(void)
  {
    unsigned int status;
    status = 0;

    //* Wait the end of command
    while ((status & AT91C_MC_FRDY) != AT91C_MC_FRDY)
      {
        status = AT91C_BASE_MC->MC_FSR;
      }
    return status;
  }
//*----------------------------------------------------------------------------
//* \fn    AT91F_Flash_Write
//* \brief Write in one Flash page located in AT91C_IFLASH,  size in 32 bits
//* \input Flash_Address: start at 0x0010 0000 size: in byte
//*----------------------------------------------------------------------------
int Flash_Write(unsigned int Flash_Address, unsigned int * buff, int size)
  {
    //* set the Flash controller base address
    AT91PS_MC ptMC = AT91C_BASE_MC;
    volatile unsigned int i, page, status;
    unsigned int * Flash;
    int vret = false;
    portENTER_CRITICAL();
      {

        //* init flash pointer
        Flash = (unsigned int *) Flash_Address;
        //* Get the Flash page number
        page = ((Flash_Address - (unsigned int) AT91C_IFLASH)
            / FLASH_PAGE_SIZE_BYTE);
        //* copy the new value
        for (i = 0; (i < FLASH_PAGE_SIZE_BYTE) & (size > 0); i++, Flash++, buff++, size
            -= 4)
          {
            //* copy the flash to the write buffer ensuring code generation
            *Flash = *buff;
          }
        //* Write the write page command
        ptMC->MC_FCR = AT91C_MC_CORRECT_KEY | AT91C_MC_FCMD_START_PROG
            | (AT91C_MC_PAGEN & (page << 8));
        //* Wait the end of command
        status = Flash_Ready();
        //* Check the result
        if ((status & (AT91C_MC_PROGE | AT91C_MC_LOCKE)) == 0)
          vret = true;

      }
    portEXIT_CRITICAL();

    return vret;
  }

//*----------------------------------------------------------------------------
inline void * Flash_GetPageStartAddress(void *addr)
  {
    return  (void*)( (int)addr - ((int)addr % FLASH_PAGE_SIZE_BYTE));
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
bool Flash_WriteBuffer(void *pxFlash, uint8 *buf, uint16 len)
  {
    uint16 i;
    uint8 pagebuff[FLASH_PAGE_SIZE_BYTE];
    uint32 offset = pxFlash - Flash_GetPageStartAddress(pxFlash);
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
            if(len == 0)break;
          }
        offset = 0;
        // write page
        Flash_Write(pxCurPageAddr, pagebuff, FLASH_PAGE_SIZE_BYTE);
      }

  return true;
  }

