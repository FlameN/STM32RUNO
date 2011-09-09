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
#include "rvnet.h"
#include "memman.h"
#include "crc.h"

//extern uint8 ebuf[256];

RVNET_DATATYPE ReadDeviceID(uint8 *);
RVNET_DATATYPE ReadNWords(uint8 *);
RVNET_DATATYPE WriteNWords(uint8 *);
RVNET_DATATYPE ErrorAddress(uint8 *);
RVNET_DATATYPE ReadNBits(uint8 *);
RVNET_DATATYPE WriteBit(uint8 *);
void SwapBuffer(uint8 *buf, uint16 len);
inline uint16 SwapWord(uint16 Word)
  {
    return (Word >> 8) | (Word << 8);
  }

inline void SwapBuffer(unsigned char *buf, uint16 len)
  {
    unsigned char tint;

    while (len--)
      {
        tint = *buf;
        *buf = *(buf + 1);
        *(buf + 1) = tint;
        buf += 2;
      }
  }
//======================================================================
//extern void BOOTSTART( void );

RVNET_DATATYPE RVnetSlaveProcess(uint8 *px_buf, RVNET_DATATYPE pkSize,
    uint8 device_address)
  {
    volatile uint8 *pxPack = px_buf;
    if (*pxPack == device_address)
      {
        if (CheckCRC(pxPack, pkSize))
          {
            pxPack++;
            switch (*pxPack)
              {
            case 0x00:
              pxPack++;
              pkSize = ReadDeviceID(pxPack);
              break;
            case 0x01:
            case 0x02:
              pxPack++;
              pkSize = ReadNBits(pxPack);
              break;
            case 0x03:
            case 0x04:
              pxPack++;
              pkSize = ReadNWords(pxPack);
              break;
            case 0x05:
              pxPack++;
              pkSize = WriteBit(pxPack);
              break;
            case 0x10:
              pxPack++;
              pkSize = WriteNWords(pxPack);
              break;
            default:
              pxPack++;
              pkSize = ErrorAddress(pxPack);
              break;

              }
          }
        else
          return 0;
      }
    else
      return 0;
    pkSize += 2;
    SetCRC(px_buf, pkSize);
    pkSize += 2;
    return pkSize;
  }
//======================================================================
RVNET_DATATYPE ReadDeviceID(unsigned char *pxPack)
  {
    // ���������� � ����������
    *pxPack = 0x00;
    pxPack++;
    *pxPack = 0x01;
    pxPack++;
    *pxPack = 0x01;
    pxPack++;
    *pxPack = 0x01;
    pxPack++;
    return 4;
  }
//======================================================================
RVNET_DATATYPE ReadNWords(uint8 *pxPack)
  {
   volatile uint16 base, wnum;
    // ���������� ��������� ������ � ����������� ����
    base = *(pxPack) << 8;
    base += *(pxPack + 1);
    wnum = *(pxPack + 2) << 8;
    wnum += *(pxPack + 3);
    if (wnum > 123)
      {
        return ErrorAddress(pxPack);
      }
    *pxPack = (unsigned char) (wnum * 2);
    if (MemGetWords(base, (uint16*)(pxPack + 1), wnum) == 0)
      {
        return ErrorAddress(pxPack);
      }
    SwapBuffer(pxPack + 1, wnum);
    return (RVNET_DATATYPE)(wnum * 2 + 1);
  }
//======================================================================
RVNET_DATATYPE WriteNWords(unsigned char *pxPack)
  {
    volatile uint16 base, wnum;
    // ���������� ��������� ������ � ����������� ����
    base = *(pxPack) << 8;
    base += *(pxPack + 1);
    wnum = *(pxPack + 2) << 8;
    wnum += *(pxPack + 3);
    if (wnum > 123)
      {
        return ErrorAddress(pxPack);
      }
    SwapBuffer(pxPack + 5, wnum);
    if (MemSetWords(base, pxPack + 5, wnum) == 0)
      {
        return ErrorAddress(pxPack);
      }
    return 4;
  }
//======================================================================
RVNET_DATATYPE ReadNBits(unsigned char *pxPack)
  {
    volatile  uint16 base, wnum;
    // ���������� ��������� ������ � ����������� ����
    base = *(pxPack) << 8;
    base += *(pxPack + 1);
    wnum = *(pxPack + 2) << 8;
    wnum += *(pxPack + 3);
    *pxPack = ((wnum/8) & 0xff);
    base /= 16;
    wnum /= 16;
    if (wnum > 123)
      {
        return ErrorAddress(pxPack);
      }
    // ��������� ������
    // E��� �� ���� RAM    -------------------------------------------------

    if (MemGetWords(base,  pxPack + 1, wnum) == 0)
      {
        return ErrorAddress(pxPack);
      }
    return (RVNET_DATATYPE)(wnum * 2 + 1);
  }
//======================================================================
RVNET_DATATYPE WriteBit(unsigned char *pxPack)
  {
    volatile uint16 base, wnum;
    uint16 temp_word;
    uint16 bit_shift;
    // ���������� ��������� ������ � ����������� ����
    base = *(pxPack) << 8;
    base += *(pxPack + 1);
    wnum = *(pxPack + 2) << 8;
    wnum += *(pxPack + 3);
    bit_shift = base % 16;
    base /= 16;
    //if(wnum >123){return ErrorAddress(pxPack);}
    // wnum.word /= 16;
    if (MemGetWords(base, &temp_word, 1) == 0)
      {
        return ErrorAddress(pxPack);
      }
    if ((wnum & 0xFF00) == 0)
      temp_word &= ~(1 << bit_shift);
    else
      temp_word |= (1 << bit_shift);
    if (MemSetWords(base, &temp_word, 1) == 0)
      {
        return ErrorAddress(pxPack);
      }
    return 4;
  }
//======================================================================

RVNET_DATATYPE ErrorAddress(unsigned char *pxPack)
  {
    *(pxPack - 1) |= 0x80;
    return 0;
  }

