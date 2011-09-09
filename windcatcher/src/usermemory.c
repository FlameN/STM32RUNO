//********************************************************
//    ������      :
//    ������      : 1.0.
//    ����        : 29.03.2009
//    �����       : ����������� ���� ��������.
//    ��������    : ����
//    ����        :
//    ����������� :
//    ����� ���������� : IAR
//*********************************************************
//    ���������           :
//    ������� ����������  :
//    ��� ���������       : ����������
//    ������ ������       :
//*********************************************************

#include "usermemory.h"

#include "rtclock.h"
#include "stm32f10x_flash.h"
#include "i2c_ee.h"

#define __at(x)

#define EEPROM_A1A0   0
#define EEPROM_DADR   (0x50|EEPROM_A1A0)

extern size_t strlen(const char * str);
RAMMEM RAM;
int tmpPointer = 0;
int tmpLen = 0;
uint16 bigFlashBuff[0x400];

bool GetRam(uint16, uint16*, uint16);
bool SetRam(uint16, uint16*, uint16);
bool GetEeprom(uint16, uint16*, uint16);
bool SetEeprom(uint16, uint16*, uint16);
bool GetRtc(uint16, uint16*, uint16);
bool SetRtc(uint16, uint16*, uint16);
bool GetFlash(uint16, uint16*, uint16);
bool SetFlash(uint16, uint16*, uint16);

bool GetSignature(uint16, uint16*, uint16);

//==============================================================================
void MemInit()
    {

	//EEPROM INIT
	I2C_EE_Init();

	//Flash_Init();
	FLASH_Unlock();
	FLASH_SetLatency(FLASH_Latency_1);


    int i;
    MemManInit();
    for (i = 0; i < DEVICE_RAMMEM_WORDS; i++)
	{
	RAM.dwords[i] = 0;
	}

    memunit[0].startaddr = 0x0000;
    memunit[0].endaddr = DEVICE_RAMMEM_WORDS;
    memunit[0].GetMem = GetRam;
    memunit[0].SetMem = SetRam;

#if(DEVICE_RAMMEM_WORDS!=0x400)
#error Check useer memory address table !!!
#endif

    memunit[4].startaddr = 0x0400;
    memunit[4].endaddr = 0x480;
    memunit[4].GetMem = GetSignature;
    //	memunit[1].SetMem = SetRam;

    memunit[1].startaddr = 0x1000;
    memunit[1].endaddr = 0x1031;
    memunit[1].GetMem = GetRtc;
    memunit[1].SetMem = SetRtc;

    memunit[2].startaddr = 0x2000;
    memunit[2].endaddr = 0x3000;
    memunit[2].GetMem = GetEeprom;
    memunit[2].SetMem = SetEeprom;

    memunit[3].startaddr = 0x8000;
    memunit[3].endaddr = 0x10000;
    memunit[3].GetMem = GetFlash;
    memunit[3].SetMem = SetFlash;

    }

//==============================================================================
bool GetRam(uint16 addr, uint16* pxDsn, uint16 len)
    {
    atcResetCounter();
    CopyDataBytes((uint8*) &RAM.dwords[addr], (uint8*) pxDsn, len * 2);
    return true;
    }
//==============================================================================
bool SetRam(uint16 addr, uint16 *pxSrc, uint16 len)
    {
    atcResetCounter();
    CopyDataBytes((uint8*) pxSrc, (uint8*) &RAM.dwords[addr], len * 2);
    return true;
    }
//==============================================================================
bool GetSignature(uint16 addr, uint16* pxDsn, uint16 len)
    {
    char buffer[64];
    //setDeviceName
    char* DeviceName = "RUNO3";
    uint8 version[4] = {10,0,0,0};
    char* DateVersion = "12/15/2010";
    CopyDataBytes((uint8*) DeviceName, (uint8*) buffer, strlen(DeviceName)+1);
    CopyDataBytes((uint8*) version, (uint8*) buffer+0x10, 4);
    CopyDataBytes((uint8*) DateVersion, (uint8*) buffer+0x20, strlen(DateVersion)+1);


    //void *pxFlash = (void*) (SIGNATURE_LOCATION);
    //pxFlash = (void*) ((int) pxFlash + addr * 2);
    CopyDataBytes((uint8*) buffer+addr*2, (uint8*) pxDsn, len * 2);
    return true;

    }
//==============================================================================
bool GetEeprom(uint16 addr, uint16* pxDsn, uint16 len)
    {

	I2C_EE_BufferRead(pxDsn, addr*2, len*2);
	//vTaskDelay(200);
	/*
    if (AT91F_TWI_Read(EEPROM_DADR, addr * 2, (uint8*) pxDsn, len * 2) == 0)
	{
	return true;
	}*/
    return true;
    }
//==============================================================================
bool SetEeprom(uint16 addr, uint16 *pxSrc, uint16 len)
    {
/*
    //atcResetCounter();
    int pointer = addr * 2;
    int finPoint = addr * 2 + len * 2;
    int templen = addr * 2;
    while (pointer < finPoint)
	{
	templen = (32 - pointer % 32);
	if (finPoint < pointer + templen)
	    templen = finPoint - pointer;
	vTaskDelay(20);
	I2C_Master_BufferWrite(pxSrc,len);

	pointer = pointer + templen;

	}*/
	/*
	bool status = true;
	u16 cmp_buf[8];
	I2C_EE_BufferWrite((uint8*)pxSrc, addr*2, len*2);

	if(len == 1)
	{
		status = false;
		I2C_EE_BufferRead((uint8*)cmp_buf, addr*2, len*2);
		if(cmp_buf[0]==pxSrc[0])
		{
			status = true;
		}
	}else
	{
		I2C_EE_BufferRead((uint8*)cmp_buf, addr*2, len*2);
		if(cmp_buf[0]==pxSrc[0])
		{
			status = true;
		}
	}*/
    return true;
    }
//==============================================================================
bool GetRtc(uint16 addr, uint16* pxDsn, uint16 len)
    {
    DATATIME temprtc;

    char iFixstr[16];

    tmpPointer = addr;
    tmpLen = len;
    int tmp = 0;
    int wr = 0;

    if(tmpPointer>=0 && tmpPointer<0x08 && tmpLen>0)
    {
	rtcGetDataTime(&temprtc);
	tmp = ((tmpPointer + tmpLen)>8)?((8 - tmpPointer)):(tmpLen);

	CopyDataBytes((uint8*) ((uint8*) (&temprtc) + tmpPointer*2),(uint8*)( pxDsn) + wr*2, tmp*2);
	tmpPointer+=tmp;
	wr+=tmp;
	tmpLen -= tmp;
    }
    if(tmpPointer>=0x08 && tmpPointer<0x10 && tmpLen>0)
    {
	rtcGetLocalDataTime(&temprtc);
	tmp = ((tmpPointer + tmpLen)>0x10)?((0x10 - tmpPointer)):(tmpLen);
	CopyDataBytes((uint8*) ((uint8*) (&temprtc) + (tmpPointer-0x08)*2),(uint8*)( pxDsn) + wr*2, tmp*2);
	tmpPointer+=tmp;
	wr+=tmp;
	tmpLen -= tmp;    }
    if(tmpPointer>=0x20 && tmpPointer<0x30 && tmpLen>0)
    {
	rtcGetiFixDateTime(iFixstr);
	tmp = ((tmpPointer + tmpLen)>0x30)?((0x30 - tmpPointer)):(tmpLen);
	CopyDataBytes((uint8*) ((uint8*) (&iFixstr) + (tmpPointer-0x20)*2),(uint8*)( pxDsn) + wr*2, tmp*2);
	tmpPointer+=tmp;
	wr+=tmp;
	tmpLen -= tmp;
    }

    atcResetCounter();
    return true;
    }
//==============================================================================
bool SetRtc(uint16 addr, uint16 *pxSrc, uint16 len)
    {
    DATATIME temprtc;
        char iFixstr[16];

        if (addr >= 0x20)
    	{
	    rtcGetiFixDateTime(iFixstr);
	    CopyDataBytes((uint8*) pxSrc, (uint8*)iFixstr,len * 2);
	    rtcSetiFixDateTime(iFixstr);
    	}
        else
	    if(addr<0x08)
	    {
		rtcGetDataTime(&temprtc);
		CopyDataBytes((uint8*) pxSrc, (uint8*) ((uint8*) (&temprtc) + addr * 2),
			    ((len + addr>0x08)?(8-addr):(len)) * 2);
		rtcSetDataTime(&temprtc);
	    }
	    else
		if(addr>=0x08 && addr < 0x10)
		    {
			rtcGetLocalDataTime(&temprtc);
		    	CopyDataBytes((uint8*) pxSrc, (uint8*) ((uint8*) (&temprtc) + (addr - 0x08) * 2),
		    			    ((len + addr>0x10)?(0x10-addr):(len)) * 2);
		    	rtcSetLocalDataTime(&temprtc);
		    }

    atcResetCounter();
    return true;
    }
//==============================================================================


//==============================================================================
bool GetFlash(uint16 addr, uint16* pxDsn, uint16 len)
    {
    void *pxFlash = (void*) (DEVICE_FLASHMEM_LOCATION);
    //pxFlash = (void*) ((int) pxFlash + addr * 2);
    CopyDataBytes(((uint8*) pxFlash) + addr * 2, (uint8*) pxDsn, len * 2);
    atcResetCounter();
    return true;
    }
//==============================================================================
bool SetFlash(uint16 addr, uint16 *pxSrc, uint16 len)
    {

	FLASHMEM *pxConfig = (FLASHMEM*) DEVICE_FLASHMEM_LOCATION;
	uint16 page = addr/0x400;
	uint16 pageEnd = (addr+len)/0x400;
	uint16 pageOffset = addr%0x400;
	uint16 pageEndOffset = (addr+len)%0x400;
	FLASH_Status status;

	if(page!=pageEnd)
	{
		//=================================first part
		CopyDataBytes((uint8*)((uint8*) (DEVICE_FLASHMEM_LOCATION) + 0x800*page), (uint8*) bigFlashBuff, 0x800);
		CopyDataBytes((uint8*) pxSrc, (uint8*)( &bigFlashBuff[pageOffset]), (0x400 - pageOffset)*2);

		while(FLASH_ErasePage(DEVICE_FLASHMEM_LOCATION+0x800*page)!=FLASH_COMPLETE)
		{
			status = FLASH_GetStatus();
			FLASH_ClearFlag(~FLASH_ERROR_PG);
			vTaskDelay(100);
		}

		WriteFlash( (void*)(bigFlashBuff),(void*)((uint8*) (DEVICE_FLASHMEM_LOCATION) + 0x800*page),  0x800);

		//=================================Second part
		CopyDataBytes((uint8*)((uint8*) (DEVICE_FLASHMEM_LOCATION) + 0x800*pageEnd), (uint8*) bigFlashBuff, 0x800);
		CopyDataBytes(((uint8*) pxSrc) + (len-pageEndOffset)*2, (uint8*)( &bigFlashBuff[0]), pageEndOffset*2);

		while(FLASH_ErasePage(DEVICE_FLASHMEM_LOCATION+0x800*pageEnd)!=FLASH_COMPLETE)
		{
			status = FLASH_GetStatus();
			FLASH_ClearFlag(~FLASH_ERROR_PG);
			vTaskDelay(100);
		}

		WriteFlash( (void*)(bigFlashBuff),(void*)((uint8*) (DEVICE_FLASHMEM_LOCATION) + 0x800*pageEnd),  0x800);
	}else
	{
		CopyDataBytes((uint8*)((uint8*) (DEVICE_FLASHMEM_LOCATION) + 0x800*page), (uint8*) bigFlashBuff, 0x800);
		CopyDataBytes((uint8*) pxSrc, (uint8*)( &bigFlashBuff[pageOffset]), len*2);

		while(FLASH_ErasePage(DEVICE_FLASHMEM_LOCATION+0x800*page)!=FLASH_COMPLETE)
		{
			status = FLASH_GetStatus();
			FLASH_ClearFlag(~FLASH_ERROR_PG);
			vTaskDelay(10);
		}

		WriteFlash( (void*)(bigFlashBuff),(void*)((uint8*) (DEVICE_FLASHMEM_LOCATION) + 0x800*page),  0x800);
	}
	//FLASH_ProgramHalfWord(DEVICE_FLASHMEM_LOCATION+addr*2,pxSrc[0]);

//----------------------------------
	/*
	if(addr<0x200)
	{
	CopyDataBytes((uint8*) pxConfig, (uint8*) bigFlashBuff, 0x400);
	CopyDataBytes((uint8*) pxSrc, (uint8*)( &bigFlashBuff[addr]), len*2);

	FLASH_ErasePage(DEVICE_FLASHMEM_LOCATION);

	WriteFlash( bigFlashBuff,(void*)(DEVICE_FLASHMEM_LOCATION),  0x400);
	//FLASH_ProgramHalfWord(DEVICE_FLASHMEM_LOCATION+addr*2,pxSrc[0]);
	}else
		if(addr<0x400)
		{
		CopyDataBytes((uint8*) (DEVICE_FLASHMEM_LOCATION+0x400), (uint8*) bigFlashBuff, 0x400);
		CopyDataBytes((uint8*) pxSrc, (uint8*)( &bigFlashBuff[addr-0x200]), len*2);

		FLASH_ErasePage(DEVICE_FLASHMEM_LOCATION+0x800);

		WriteFlash( bigFlashBuff,(void*)(DEVICE_FLASHMEM_LOCATION+0x400),  0x400);
		//FLASH_ProgramHalfWord(DEVICE_FLASHMEM_LOCATION+addr*2,pxSrc[0]);
		}*/
	atcResetCounter();
	return true;
    }

