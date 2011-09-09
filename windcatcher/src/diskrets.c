// ���� ������� ��� ��������� ���������
#include "diskrets.h"
#include "usermemory.h"

DISKRETS DiskretsWave[DISKRETS_AMOUNT];
DISKRETS DiskretsWaveOld[DISKRETS_AMOUNT];
DISKRETS DiskretsWaveFiltered[DISKRETS_AMOUNT];
unsigned short   DiskretsChangesCounter[DISKRETS_AMOUNT];
unsigned short   DiskretsChangesTimer[DISKRETS_AMOUNT];
unsigned short   DiskretsWaveStateCount[DISKRETS_AMOUNT];

//======================================================================================================
void DiskretsClear(void)
{
  int i = DISKRETS_AMOUNT;
  while(i--)
  {
   DiskretsChangesCounter[i] = 0;
   DiskretsChangesTimer[i] = 0;
   DiskretsWave[i] = 0;
   DiskretsWaveFiltered[i] = 0;
   DiskretsWaveOld[i] = 0;
   DiskretsWaveStateCount[i] = 0;
  }
}
//---------------------------------------
void DiskretSet(int num,DISKRETS vol)
{
 if( (num < DISKRETS_AMOUNT) &&
     ( (vol == DISKRET_STATE_ON) ||
       (vol == DISKRET_STATE_OFF)||
       (vol != DISKRET_STATE_MAILFUNCTION) ) )
  {
   DiskretsWave[num] = vol;
  }
}
//---------------------------------------
// ���������� �������� ������������� ��������
DISKRETS DiskretGet(int num)
{
  if (num < DISKRETS_AMOUNT)
      {
        return DiskretsWaveFiltered[num];
      }
    return DISKRET_STATE_MAILFUNCTION;
}
//-------------------------------------------------------------------------------
// ���������� ������� ����� ��������� � ������������ ����������� ��������
//-------------------------------------------------------------------------------
void DiskretsProcessWave(int i)
{
    // Noise diskrets check
    if (DiskretsChangesTimer[i] < 100)
      {
        DiskretsChangesTimer[i]++;
      }
    else
      {
        if (DiskretsChangesCounter[i] > DISKRET_NOISE)
          {
            DiskretsChangesCounter[i] = 0xFFFF;
          }
        else
          {
            DiskretsChangesCounter[i] = 0;
            DiskretsChangesTimer[i] = 0;
          };
      }
    // Set Mailf Diskrets
    if ((DiskretsWave[i] == DISKRET_STATE_MAILFUNCTION))
      {
        DiskretsWaveFiltered[i] = DISKRET_STATE_MAILFUNCTION;
        DiskretsWaveOld[i] = DiskretsWave[i];
        return;
      }

    // Diskrets wave processing
    if (DiskretsWave[i] != DiskretsWaveOld[i])
      {// ���� �������� ���������� �� �������
        DiskretsChangesCounter[i]++;
        if (DiskretsWave[i] == DISKRET_STATE_ON)
          {
            DiskretsWaveStateCount[i] = DISKRET_POSWAVE_TIME;
          }
        else
          {// ���� �������� ���������� �� ����
            DiskretsWaveStateCount[i] = DISKRET_NEGWAVE_TIME;
          }
      }
    else
      {// ���� ����� �������� �� 0
        if (DiskretsWaveStateCount[i] == 0)
          {
            DiskretsWaveFiltered[i] = DiskretsWave[i];
          }
        else
          {
            DiskretsWaveStateCount[i]--;
          }
      }
     DiskretsWaveOld[i] = DiskretsWave[i];
}
//-------------------------------------------------------------------------------
// �������� ������� ��������� ���������
//-------------------------------------------------------------------------------
void DiskretsProcess()
{
#ifndef RUNO
#if(DISKRETS_AMOUNT>4*16)
#error tmpRam too small
#endif
  uint16 tmpRam[4]= {0,0,0,0};
#ifdef DISKRETS2
 for(int i=0;i<11;i++)
 {
	   DiskretsProcessWave(i+11);
	    if(DiskretsWaveFiltered[i+11] == DISKRET_STATE_ON)
	       {
	       tmpRam[0] |= (1<<(i&0x0f));
	       } else
	        {
	        tmpRam[0] &= ~(1<<(i&0x0f));
	        }
 }
 for(int i=0;i<11;i++)
  {
 	   DiskretsProcessWave(i+33);
 	    if(DiskretsWaveFiltered[i+33] == DISKRET_STATE_ON)
 	       {
	         tmpRam[1] |= (1<<(i&0x0f));
 	       } else
 	        {
 	          tmpRam[1] &= ~(1<<(i&0x0f));
 	        }
  }
#else
 int i = DISKRETS_AMOUNT;

 while(i--)
  {
    DiskretsProcessWave(i);
    if(DiskretsWaveFiltered[i] == DISKRET_STATE_ON)
       {

         tmpRam[(i>>4)] |= (1<<(i&0x0f));

       } else
        {
          tmpRam[(i>>4)] &= ~(1<<(i&0x0f));
        }

  }
#endif
  for(int i=0;i<4;i++)
      {
      RAM.diskrets[i] = tmpRam[i];
      }
  //MemSetWords(0x0000,tmpRam,4);
#else
  uint16 tmpRam=0;

  int i = DISKRETS_AMOUNT;

   while(i--)
    {
      DiskretsProcessWave(i);
      if(DiskretsWaveFiltered[i] == DISKRET_STATE_ON)
         {
           tmpRam |= (1<<(i&0x0f));
         } else
          {
            tmpRam &= ~(1<<(i&0x0f));
          }

    }

  RAM.diskrets[0]=tmpRam;

#endif
}
