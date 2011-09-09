//********************************************************
//    Проект      :
//    Версия      : 1.0.
//    Дата        : 29.03.2009
//    Автор       : Копачевский Юрий Иванович.
//    Компания    : БЭМН
//    Файл        :
//    Комментарии :
//    Среда разработки : IAR
//*********************************************************
//    Процессор           :
//    Частота процессора  :
//    Тип программы       : Приложение
//    Модель памяти       :
//*********************************************************
#include "error.h"
#include "memman.h"

ErrorHandle MemManReadAccessError;
ErrorHandle MemManWriteAccessError;


MEMUNIT memunit[MEMUNITS_NUM];
int GetDefaultHandler( uint16, uint16*, uint16) ;
int SetDefaultHandler( uint16, uint16*, uint16);

bool MemManInit()
  {
    int i = MEMUNITS_NUM;
    bool rezult = false;
    MEMUNIT *pxMemunit = &memunit[0];
    while (i--)
      {
        pxMemunit->startaddr = 0x0;
        pxMemunit->endaddr = 0x0;
        pxMemunit->GetMem = GetDefaultHandler;
        pxMemunit->SetMem = SetDefaultHandler;
        pxMemunit->xMutex = xSemaphoreCreateMutex() ;
        pxMemunit++;
      }
    rezult = true;
    MemManReadAccessError = ErrorCreate("MM_ReadAccess");
    MemManWriteAccessError = ErrorCreate("MM_WriteAccess");
    return rezult;

  }
void CopyDataBytes(uint8 *pxSRC, uint8 *pxDSN, uint16 BytesNum)
  {
    while (BytesNum--)
      {
        *(pxDSN++) = *(pxSRC++);
      }
  }
//==============================================================================
int GetDefaultHandler(uint16 addr, uint16* pxDsn, uint16 len)
  {
    return false;
  }
int SetDefaultHandler(uint16 addr, uint16* pxDsn, uint16 len)
  {
    return false;
  }

bool MemGetWords(uint16 addr, uint16* pxDsn, uint16 len)
  {
    int i = MEMUNITS_NUM;
    bool rezult = false;
    MEMUNIT *pxMemunit = &memunit[0];
    while (i--)
      {
        if ((addr >= pxMemunit->startaddr) && ((addr + len)
            < pxMemunit->endaddr))
          {
            if (pxMemunit->xMutex != NULL)
              {
                if (xSemaphoreTake(pxMemunit->xMutex, (portTickType) 2000)
                    == pdTRUE)
                  {
                    rezult = pxMemunit->GetMem(addr - pxMemunit->startaddr, pxDsn, len);
                    xSemaphoreGive(pxMemunit->xMutex);
                  }
              }
          }
        pxMemunit++;
      }
    return rezult;
  }
//==============================================================================
bool MemSetWords(uint16 addr, uint16 *pxSrc, uint16 len)
  {
    int i = MEMUNITS_NUM;
    bool rezult = false;
    MEMUNIT *pxMemunit = &memunit[0];
    while (i--)
      {
        if ((addr >= pxMemunit->startaddr) && ((addr + len)
            < pxMemunit->endaddr))
          {
            if (pxMemunit->xMutex != NULL)
              {
                if (xSemaphoreTake(pxMemunit->xMutex, (portTickType) 2000)
                    == pdTRUE)
                  {
                    rezult = pxMemunit->SetMem(addr - pxMemunit->startaddr, pxSrc, len);
                    xSemaphoreGive(pxMemunit->xMutex);
                  }
              }
          }
        pxMemunit++;
      }
    return rezult;
  }

