// Error File
#include "error.h"

uint16 ErrorsAmount    = 1;
uint16 ErrorsMaxAmount = 1;
bool   IsErrorInitialized = false;
ErrorDescriptor *Errors = null;
ErrorHandle ErrorStatus = 0;

ErrorHandle ErrorCheckStatus()
{
  ErrorHandle tmp = ErrorStatus;
  ErrorStatus = 0;
  return tmp;
}


void ErrorInit(void *pxErrorsBuffer, uint16 ErrorTypesAmount)
{
  uint16 i = 0;
  char *eDescription = "Errors Header";
  ErrorsMaxAmount = ErrorTypesAmount;
  Errors  = (ErrorDescriptor*)pxErrorsBuffer;
  IsErrorInitialized = true;
  Errors[0].EvetsCounter = 0;
  while( i< ERROR_DESCRIPTOR_TEXT_SIZE)
    {
      Errors[0].Description[i] =  *eDescription;
      eDescription++;
      i++;
    }

}
ErrorHandle ErrorCreate(char *eDescription)
{
  uint16 i = 0;
  ErrorHandle eHandle ;
  if((ErrorsAmount != ErrorsMaxAmount) && IsErrorInitialized)
  {
    eHandle = ErrorsAmount;
    ErrorsAmount++;
    Errors[eHandle].EvetsCounter = 0;
    Errors[0].EvetsCounter++;
    while( (i<ERROR_DESCRIPTOR_TEXT_SIZE) && (*eDescription))
    {
      Errors[eHandle].Description[i] =  *eDescription;
      eDescription++;
      i++;
    }
   return eHandle;
  }
  return 0;
}
bool SYSTEM_ERROR(ErrorHandle eHandle)
{
  if( eHandle < ErrorsMaxAmount && eHandle > 0)
  {
    Errors[eHandle].EvetsCounter++;
    ErrorStatus = eHandle;
    return true;
  }
  return false;

};




