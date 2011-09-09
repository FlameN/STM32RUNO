// RV_NET h
#include "typedef.h"

#ifndef RVNET_DATATYPE
typedef unsigned char RVNET_DATATYPE;
#endif


struct F3REQHEADER
{
  uint16  base;
  uint16  wnum;
};

struct F3ANSHEADER
{
  uint16  base;
  uint16  wnum;
};


extern RVNET_DATATYPE RVnetSlaveProcess(uint8 *px_buf,RVNET_DATATYPE,unsigned char device_address);
//======================================================================
extern RVNET_DATATYPE ReadDeviceID(unsigned char *pxPack);
extern RVNET_DATATYPE ReadNWords(uint8 *pxPack);
extern RVNET_DATATYPE WriteNWords(unsigned char *pxPack);
extern RVNET_DATATYPE ReadNBits(unsigned char *pxPack);
extern RVNET_DATATYPE WriteBit(unsigned char *pxPack);
extern RVNET_DATATYPE ErrorAddress(unsigned char *pxPack);

