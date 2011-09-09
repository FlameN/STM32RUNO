/*
 * translator.c
 *
 *  Created on: May 26, 2010
 *      Author: albert
 */
// It file for Main translator func
#include "typedef.h"
#include "usermemory.h"
#include "translator.h"
#include "rtclock.h"
#include "journal.h"
//#include "modules.h"
#include "crc.h"

bool bPowerOn = false;
volatile DATATIME timer;
uint8 prevLogicFlags = 0;
uint32 testSwitchRele=0;
uint16 prevErrDiscretLogic[4] =
    {
    0, 0, 0, 0
    };

void Reset()
    {
    //uint16* reset = 0x00100000;
    //(*reset) = 0;
    //  (void ())(0x00100000)();
    //AT91C_BASE_RSTC->RSTC_RCR = 0xA5000003; // Controller+Periph
    for (int i = 0; i < DEVICE_RAMMEM_WORDS; i++)
	{
	RAM.dwords[i] = 0;
	}
    }

int GetChannelBitSignal(int channelNum, uint16* ChannelData)
    {

    if (ChannelData[(int) (channelNum / 4)] & (1 << ((4* channelNum ) % 16)))
	{
	return 1;
	}
    else
	{
	return 0;
	}
    }

int GetChannelBitRepare(int channelNum, uint16* ChannelData)
    {
    int temp;
#ifdef LIDACONF
    if (ChannelData[(int) (channelNum / 4)]
	    & (1 << ((4* channelNum ) % 16 + 1)))
	{
	return 1;
	}
    else
	{
	return 0;
	}
#else
    if (ChannelData[(int) (channelNum / 4)]
	    & (1 << ((4* channelNum ) % 16 + 3)))
	{
	return 1;
	}
    else
	{
	return 0;
	}
#endif
    }

int GetChannelBitDirect(int channelNum, uint16* ChannelData)
    {
    if (ChannelData[(int) (channelNum / 4)]
	    & (1 << ((4* channelNum ) % 16 + 2)))
	{
	return 1;
	}
    else
	{
	return 0;
	}
    }

int GetChannelBitReserv(int channelNum, uint16* ChannelData)
    {
#ifdef LIDACONF
    if (ChannelData[(int) (channelNum / 4)]
	    & (1 << ((4* channelNum ) % 16 + 3)))
	{
	return 1;
	}
    else
	{
	return 0;
	}
#else
    if (ChannelData[(int) (channelNum / 4)]
	    & (1 << ((4* channelNum ) % 16 + 1)))
	return 1;
    else
	return 0;
#endif
    }

void SetChannelBitSignal(int channelNum, uint16* ChannelData, int bit)
    {
    if (bit)
	{
	ChannelData[(int) (channelNum / 4)]
		= ChannelData[(int) (channelNum / 4)] | (1 << (4* channelNum )
			% 16);
	}
    else
	{
	ChannelData[(int) (channelNum / 4)]
		= ChannelData[(int) (channelNum / 4)] & (~(1
			<< (4* channelNum ) % 16));
	}
    }

void SetChannelBitRepare(int channelNum, uint16* ChannelData, int bit)
    {
#ifdef LIDACONF
    if (bit)
	{
	ChannelData[(int) (channelNum / 4)]
	= ChannelData[(int) (channelNum / 4)] | (1 << ((4* channelNum )
			% 16 + 1));
	}
    else
	{
	ChannelData[(int) (channelNum / 4)]
	= ChannelData[(int) (channelNum / 4)] & (~(1
			<< ((4* channelNum ) % 16 + 1)));
	}
#else
    if (bit)
	{
	ChannelData[(int) (channelNum / 4)]
		= ChannelData[(int) (channelNum / 4)] | (1 << ((4* channelNum )
			% 16 + 3));
	}
    else
	{
	ChannelData[(int) (channelNum / 4)]
		= ChannelData[(int) (channelNum / 4)] & (~(1
			<< ((4* channelNum ) % 16 + 3)));
	}
#endif
    }

void SetChannelBitDirect(int channelNum, uint16* ChannelData, int bit)
    {
    if (bit)
	{
	ChannelData[(int) (channelNum / 4)]
		= ChannelData[(int) (channelNum / 4)] | (1 << ((4* channelNum )
			% 16 + 2));
	}
    else
	{
	ChannelData[(int) (channelNum / 4)]
		= ChannelData[(int) (channelNum / 4)] & (~(1
			<< ((4* channelNum ) % 16 + 2)));
	}
    }

void SetChannelBitReserv(int channelNum, uint16* ChannelData, int bit)
    {
#ifdef LIDACONF
    if (bit)
	{
	ChannelData[(int) (channelNum / 4)]
	= ChannelData[(int) (channelNum / 4)] | (1 << ((4* channelNum )
			% 16 + 3));
	}
    else
	{
	ChannelData[(int) (channelNum / 4)]
	= ChannelData[(int) (channelNum / 4)] & (~(1
			<< ((4* channelNum ) % 16 + 3)));
	}
#else
    if (bit)
	{
	ChannelData[(int) (channelNum / 4)]
		= ChannelData[(int) (channelNum / 4)] | (1 << ((4* channelNum )
			% 16 + 1));
	}
    else
	{
	ChannelData[(int) (channelNum / 4)]
		= ChannelData[(int) (channelNum / 4)] & (~(1
			<< ((4* channelNum ) % 16 + 1)));
	}
#endif
    }

int GetDiskretFromRam(int i)
    {

    return ((RAM.diskrets[i / 11] >> (i % 11)) & 0x1);
    }
void SetRelayToRam(int i, int vol)
    {
    if (vol != 0)
	{
	RAM.relays[i / 16] |= (1 << (i % 16));
	}
    else
	{
	RAM.relays[i / 16] &= ~(1 << (i % 16));
	}
    }
int CheckDiscretMask(uint16 *mask)
    {
    int rezult = 1;
    for (int i = 0; i < 4; i++)
	{
	if ((RAM.diskrets[i] & mask[i]) != (mask[i]))
	    {
	    rezult = 0;
	    RAM.ErrorDiscretLogic[i] &= (~mask[i]);
	    RAM.ErrorDiscretLogic[i] |= ((~(RAM.diskrets[i] & mask[i]))
		    & mask[i]);
	    }
	else
	    {
	    RAM.ErrorDiscretLogic[i] &= (~mask[i]);
	    }
	}
    return rezult;
    }

int CheckDiscretMaskNegative(uint16 *mask)
    {
    int rezult = 1;
    for (int i = 0; i < 4; i++)
	{
	if ((RAM.diskrets[i] & mask[i]) == 0)
	    {
	    RAM.ErrorDiscretLogic[i] &= (~mask[i]);
	    }
	else
	    {
	    rezult = 0;
	    RAM.ErrorDiscretLogic[i] &= (~mask[i]);
	    RAM.ErrorDiscretLogic[i] |= ((RAM.diskrets[i] & mask[i]));
	    }
	}
    return rezult;
    }
void CheckCUSignal()
    {
    FLASHMEM *pxConfig = (FLASHMEM*) DEVICE_FLASHMEM_LOCATION;
    char buffer[256];
    char minbuf[32];
    for (int i = 0; i < 8; i++)
	{
	if (pxConfig->devcfg.logica.confCU[i].releNum == 0)
	    continue;
	itoa(i + 1, minbuf);
	uint8 TempReleInd = pxConfig->devcfg.logica.confCU[i].releNum - 1;
	if (GetChannelBitSignal(i, RAM.OutputCommand)
		!= ((RAM.relays[TempReleInd / 16] >> (TempReleInd % 16))
			& (0x0001)))
	    {
	    if (GetChannelBitSignal(i, RAM.OutputCommand) == 1)
		{
			strcpy(buffer,"Вкл. канала:");
			strcat(buffer,minbuf);
		//sprintf(buffer, "Вкл. канала:%s", minbuf);
		JrnlWrite(buffer);
		}
	    else
		{
	    	strcpy(buffer,"Выкл. канала:");
	    	strcat(buffer,minbuf);
		//sprintf(buffer, "Выкл. канала:%s", minbuf);
		JrnlWrite(buffer);
		}
	    }
	}
    }

void CheckCUDirect()
    {
    FLASHMEM *pxConfig = (FLASHMEM*) DEVICE_FLASHMEM_LOCATION;
    char buffer[256];
    char minbuf[32];

    for (int i = 0; i < 8; i++)
	{
	if (pxConfig->devcfg.logica.confCU[i].releNum == 0)
	    continue;
	if (!GetChannelBitReserv(i, RAM.LocalCommand))
	    continue;
	itoa((i + 1), minbuf);
	uint8 TempReleInd = pxConfig->devcfg.logica.confCU[i].releNum - 1;
	if (GetChannelBitDirect(i, RAM.OutputCommand) != GetChannelBitDirect(i,
		RAM.LocalCommand))
	    {
	    if (GetChannelBitDirect(i, RAM.LocalCommand) == 1)
		{
	    	strcpy(buffer,"Автомат. выкл. КУ:");
	    	strcat(buffer,minbuf);
		//sprintf(buffer, "Автомат. выкл. КУ:%s", minbuf);
		JrnlWrite(buffer);
		}
	    else
		{
	    	strcpy(buffer,"Автомат. вкл. КУ:");
	    	strcat(buffer,minbuf);
		//sprintf(buffer, "Автомат. вкл. КУ:%s", minbuf);
		JrnlWrite(buffer);
		}
	    }
	}
    }

void CheckErrors()
    {
    uint8 diffMask;
    if (prevLogicFlags != RAM.LogicErrorFlags)
	{
	diffMask = (prevLogicFlags) ^ (RAM.LogicErrorFlags);

	for (int i = 0; i < 8; i++)
	    {
	    if (((diffMask >> i) & 0x01) == 0x01)
	    {
		if (((RAM.LogicErrorFlags >> i) & 0x01) == 0x01)
		{
		    switch (i)
			{
		    case 0:
			JrnlWrite("Ошибка питания");
			break;
		    case 1:
			JrnlWrite("Ошибка цепей упр.");
			break;
		    case 2:
			JrnlWrite("Ошибка охраны");
			break;
		    case 3:
			JrnlWrite("Ошибка управления");
			break;
		    case 4:
			JrnlWrite("Ошибка предохр.");
			break;
		    default:
			JrnlWrite("Неизв ошибка!");
			break;
			}
		}
	    else
		{
		    switch (i)
		    {
		    case 0:
			JrnlWrite("Норма питание");
			break;
		    case 1:
			JrnlWrite("Норма цепей упр.");
			break;
		    case 2:
			JrnlWrite("Норма охраны");
			break;
		    case 3:
			JrnlWrite("Норма управления");
			break;
		    case 4:
			JrnlWrite("Норма предохр.");
			break;
		    default:
			JrnlWrite("Неизв норма");
			break;
		    }
		}
		}
	    }
	prevLogicFlags = RAM.LogicErrorFlags;
	}
    }

void CheckDiscretsErrors()
    {
    uint16 diffMask=0;
    char buffer[64];
    char conv_buf[8];
    for(int it=0;it<4;it++)
    if (prevErrDiscretLogic[it] != RAM.ErrorDiscretLogic[it])
	{
	diffMask = prevErrDiscretLogic[it] ^ RAM.ErrorDiscretLogic[it];

	for (int i = 0; i < 11; i++)
	    {
	    if ((diffMask >> i) & 0x0001 == 0x0001)
		{
		    if ((RAM.ErrorDiscretLogic[it] >> i) & 0x0001 == 0x0001)
		    {
				strcpy(buffer,"ERR Диск. М:");
				itoa(it+1,conv_buf);
				strcat(buffer,conv_buf);
				strcat(buffer," Д:");
				itoa(i+1,conv_buf);
				strcat(buffer,conv_buf);
			//sprintf(buffer, "ERR Диск. М:%i Д:%i", it+1,i+1);
			JrnlWrite(buffer);
		    }
		    else
		    {
		    	strcpy(buffer,"Ok Диск. М:");
				itoa(it+1,conv_buf);
				strcat(buffer,conv_buf);
				strcat(buffer," Д:");
				itoa(i+1,conv_buf);
				strcat(buffer,conv_buf);
			//sprintf(buffer, "Ok Диск. М:%i Д:%i", it+1,i+1);
			JrnlWrite(buffer);
		    }
	    }

	}
	prevErrDiscretLogic[it] = RAM.ErrorDiscretLogic[it];
	}
    }

void CheckPowerOn()
    {

    if (bPowerOn == false)
	{
	bPowerOn = true;

	JrnlWrite("Контроллер Вкл.");
	}
    }

//--------------------------------------------------------------------------
void DoProgram()
    {
    DATATIME dtl, dt;
    bool reverseDay;
    char tmp_buf[8];
    FLASHMEM *pxConfig = (FLASHMEM*) DEVICE_FLASHMEM_LOCATION;
    //reset to default


    if (pxConfig->devcfg.logica.SwitchTime > 1800
	    || pxConfig->devcfg.logica.SwitchTime == 0)
	{
    	tmp_buf[0]=60;
    	tmp_buf[1]=0;
    	//MemSetWords((uint16)(&(pxConfig->devcfg.logica.SwitchTime)),tmp_buf,1);
//	pxConfig->devcfg.logica.SwitchTime = 60;
	}

    for (int i = 0; i < 8; i++)
	if (pxConfig->devcfg.logica.confCU[i].releNum > 8)
	    {
    	tmp_buf[0]=0;
    	tmp_buf[1]=0;
    	//MemSetWords((uint16)(&(pxConfig->devcfg.logica.confCU[i].releNum)),tmp_buf,1);
//	    pxConfig->devcfg.logica.confCU[i].releNum = 0;
	    }

    if (GetJrnlLength() == 0 || GetJrnlLength() > 0x6000)
	{
    	//JrnlClear();
	}
    //    uint16 size = GetJrnlLength();
    //rtcGetLocalDataTime(&dtl);

    if (RAM.reset != 0)
	{
	RAM.reset = 0;
	Reset();
	}

    CheckPowerOn();
    /*
     if (GetModulErrors(0) > 10)
     {
     RAM.DiagnDevice[0] |= 1 << 7;
     //RAM.DiagnDevice[0] |= 1 << 6;
     }
     else
     {
     RAM.DiagnDevice[0] &= ~(1 << 7);
     }
     if (GetModulErrors(1) > 10)
     {
     RAM.DiagnDevice[0] |= 1 << 8;
     //RAM.DiagnDevice[0] |= 1 << 6;
     }
     else
     {
     RAM.DiagnDevice[0] &= ~(1 << 8);
     }
     if (GetModulErrors(2) > 10)
     {
     RAM.DiagnDevice[0] |= 1 << 9;
     //RAM.DiagnDevice[0] |= 1 << 6;
     }
     else
     {
     RAM.DiagnDevice[0] &= ~(1 << 9);
     }
     if (RAM.DiagnDevice[0] & (0x380))
     {
     RAM.DiagnDevice[0] |= 1 << 6;
     }
     else
     {
     RAM.DiagnDevice[0] &= ~(1 << 6);
     }
*/
     if (pxConfig->devcfg.logica.SwitchTime  < atcGetCounter())
     {
	 for (int i = 0; i < 8; i++)
	 {
	 uint16 tmpGrNum = pxConfig->devcfg.logica.confCU[i].grafNum;
	     if (tmpGrNum > 0 && tmpGrNum < 9)
	     {
		 SetChannelBitDirect(i, RAM.LocalCommand, 0);
	     }
	 }
	 atcResetCounter();
     }

    rtcGetDataTime(&dt);
    rtcGetLocalDataTime(&dtl);
/*
    if(timer.Years==0 ||timer.Month==0||timer.Day==0)
    {
    	rtcGetDataTime(&timer);
    }
    if(dt.Data!=timer.Data)
    {
    	rtcSync();
		rtcGetDataTime(&timer);
    }
*/
    if (CheckDiscretMask(pxConfig->devcfg.logica.MaskControl))
	{
	//RAM.ControlBlock = 0;
	RAM.LogicErrorFlags &= ~(1 << 3);
	}
    else
	{
	//RAM.ControlBlock = 1;
	RAM.LogicErrorFlags |= (1 << 3);
	}

    if (CheckDiscretMask(pxConfig->devcfg.logica.MaskPower))
	{
	//RAM.ControlBlock = 0;
	RAM.LogicErrorFlags &= ~(1);
	}
    else
	{
	//RAM.ControlBlock = 1;
	RAM.LogicErrorFlags |= (1);
	}

    //global command processing

    for (int i = 0; i < 8; i++)
	{
	if (GetChannelBitRepare(i, RAM.CommonCommand) == 1)
	    {
	    if (GetChannelBitRepare(i, RAM.OutputCommand) == 1)
		continue;
	    SetChannelBitSignal(i, RAM.LocalCommand, GetChannelBitSignal(i,
		    RAM.CommonCommand));
	    SetChannelBitDirect(i, RAM.LocalCommand, GetChannelBitDirect(i,
		    RAM.CommonCommand));
	    SetChannelBitReserv(i, RAM.LocalCommand, GetChannelBitReserv(i,
		    RAM.CommonCommand));
	    SetChannelBitRepare(i, RAM.CommonCommand, 0);//insurance to make global command only once.
	    }
	}

    //CheckCUDirect();

    //local command processing
    //local command processing
    for (int i = 0; i < 8; i++)
	{
	if (pxConfig->devcfg.logica.confCU[i].releNum == 0)
	    continue;
	//if (!GetChannelBitReserv(i, RAM.LocalCommand))
	//    continue;
	//SetChannelBitReserv(i, RAM.LocalCommand, 0);
	SetChannelBitRepare(i, RAM.OutputCommand, GetChannelBitRepare(i,
		RAM.LocalCommand));
	SetChannelBitReserv(i, RAM.OutputCommand, GetChannelBitReserv(i,
		RAM.LocalCommand));

/*
		    if(atcGetCounter() - testSwitchRele >1000)
			{
			    testSwitchRele = atcGetCounter();
			    SetChannelBitDirect(i, RAM.OutputCommand, 1);
			    SetChannelBitSignal(i, RAM.OutputCommand, !GetChannelBitSignal(i, RAM.OutputCommand));

			}
		    continue;*/
	if (GetChannelBitDirect(i, RAM.LocalCommand) == 1)
	    {
	    SetChannelBitDirect(i, RAM.OutputCommand, 1);
/*
			if(i==3)
			{
				if(atcGetCounter() - testSwitchRele >100)
				{
					testSwitchRele = atcGetCounter();
					SetChannelBitSignal(i, RAM.OutputCommand, !GetChannelBitSignal(i, RAM.OutputCommand));
				}
			}else
			{*/
				SetChannelBitSignal(i, RAM.OutputCommand, GetChannelBitSignal(i,
					RAM.LocalCommand));
			//}
	    }
	else
	    {
	    SetChannelBitDirect(i, RAM.OutputCommand, 0);
	    if (GetChannelBitRepare(i, RAM.LocalCommand) == 1)
		{
		SetChannelBitSignal(i, RAM.OutputCommand, 0);
		}

	    }

	//SetChannelBitReserv(i, RAM.LocalCommand, 0);
	}

    //grafs check
    for (int i = 0; i < 8; i++)
	{
	uint16 tmpGrNum = pxConfig->devcfg.logica.confCU[i].grafNum;

	if (GetChannelBitRepare(i, RAM.OutputCommand))
	    {
	    //RAM.ChannelSignal &= ~(1 << i);
	    continue;
	    }

	if (GetChannelBitDirect(i, RAM.OutputCommand))
	    {
	    continue;
	    }
/*
	if (tmpGrNum == 0 || tmpGrNum > 10)
	    {
	    SetChannelBitSignal(i, RAM.OutputCommand, 0);
	    continue;
	    }
*/
	if (tmpGrNum > 0 && tmpGrNum < 5)
	    {
	    tmpGrNum--;
	    reverseDay = false;
	    uint8 tmp = 0;
	    uint8
		    startHour =
			    pxConfig->devconst.Graph[tmpGrNum].Shedule[dt.Month
				    - 1][dt.Data - 1].StartHour;
	    uint8
		    startMin =
			    pxConfig->devconst.Graph[tmpGrNum].Shedule[dt.Month
				    - 1][dt.Data - 1].StartMin;
	    uint8
		    finishHour =
			    pxConfig->devconst.Graph[tmpGrNum].Shedule[dt.Month
				    - 1][dt.Data - 1].FinishHour;
	    uint8
		    finishMin =
			    pxConfig->devconst.Graph[tmpGrNum].Shedule[dt.Month
				    - 1][dt.Data - 1].FinishMin;
	    if (startHour * 60 + startMin > finishHour * 60 + finishMin)
		{
		reverseDay = true;
		tmp = startHour;
		startHour = finishHour;
		finishHour = tmp;

		tmp = startMin;
		startMin = finishMin;
		finishMin = tmp;
		}
	    //-------------- graph 1-3 without economy-------------------
	    if ((startHour * 60 + startMin <= (dt.Hour * 60 + dt.Min))
		    && (finishHour * 60 + finishMin > (dt.Hour * 60 + dt.Min)))
		{
		if (!reverseDay)
		    SetChannelBitSignal(i, RAM.OutputCommand, 0);
		else
		    SetChannelBitSignal(i, RAM.OutputCommand, 1);
		continue;
		}
	    else
		{
		if (!reverseDay)
		    SetChannelBitSignal(i, RAM.OutputCommand, 1);
		else
		    SetChannelBitSignal(i, RAM.OutputCommand, 0);
		continue;
		}

	    }

	if (tmpGrNum == 5)
	    {
	    tmpGrNum--;

	    uint8 startMonth = pxConfig->devconst.Schedule4.StartMonth;
	    uint8 startDate = pxConfig->devconst.Schedule4.StartDay;
	    uint8 finishMonth = pxConfig->devconst.Schedule4.FinishMonth;
	    uint8 finishDate = pxConfig->devconst.Schedule4.FinishDay;

	    uint8 FnMonth = (finishMonth > startMonth) ? finishMonth
		    : (finishMonth + 12);
	    uint8 CurMonth = (dt.Month * 31 + dt.Data >= startMonth * 31
		    + startDate) ? dt.Month : (dt.Month + 12);
	    //-------------- graph 4 heating-------------------
	    if ((startMonth * 31 + startDate <= CurMonth * 31 + dt.Data)
		    && (FnMonth * 31 + finishDate > CurMonth * 31 + dt.Data))
		{
		SetChannelBitSignal(i, RAM.OutputCommand, 1);
		continue;
		}
	    else
		{
		SetChannelBitSignal(i, RAM.OutputCommand, 0);
		continue;
		}
	    }
	if (tmpGrNum < 9 && tmpGrNum > 5)
	    {
	    tmpGrNum -= 6;
	    uint8
		    startHour =
			    pxConfig->devconst.Graph[tmpGrNum].Shedule[dt.Month
				    - 1][dt.Data - 1].StartHour;
	    uint8
		    startMin =
			    pxConfig->devconst.Graph[tmpGrNum].Shedule[dt.Month
				    - 1][dt.Data - 1].StartMin;
	    uint8
		    finishHour =
			    pxConfig->devconst.Graph[tmpGrNum].Shedule[dt.Month
				    - 1][dt.Data - 1].FinishHour;
	    uint8
		    finishMin =
			    pxConfig->devconst.Graph[tmpGrNum].Shedule[dt.Month
				    - 1][dt.Data - 1].FinishMin;
	    //-------------- graphs 1-3 with economy-------------------
	    if (startHour * 60 + startMin <= dt.Hour * 60 + dt.Min
		    && finishHour * 60 + finishMin > dt.Hour * 60 + dt.Min)
		{
		SetChannelBitSignal(i, RAM.OutputCommand, 0);
		continue;
		}
	    else
		{
		//date economy
		if ((pxConfig->devconst.Graph[tmpGrNum].EconomyDate.StartMonth
			* 31
			+ pxConfig->devconst.Graph[tmpGrNum].EconomyDate.StartDay
			<= dtl.Month * 31 + dtl.Data)
			&& (pxConfig->devconst.Graph[tmpGrNum].EconomyDate.FinishMonth
				* 31
				+ pxConfig->devconst.Graph[tmpGrNum]. EconomyDate.FinishDay
				>= dtl.Month * 31 + dtl.Data))
		    {
		    //time economy
		    uint8
			    startHourEk =
				    pxConfig->devconst.Graph[tmpGrNum].Shedule[dtl.Month
					    - 1][31].StartHour;
		    uint8
			    startMinEk =
				    pxConfig->devconst.Graph[tmpGrNum].Shedule[dtl.Month
					    - 1][31].StartMin;
		    uint8
			    finishHourEk =
				    pxConfig->devconst.Graph[tmpGrNum].Shedule[dtl.Month
					    - 1][31].FinishHour;
		    uint8
			    finishMinEk =
				    pxConfig->devconst.Graph[tmpGrNum].Shedule[dtl.Month
					    - 1][31].FinishMin;
		    uint8 FnEKHour =
			    (finishHourEk > startHourEk) ? finishHourEk
				    : (finishHourEk + 24);
		    uint8 CurHour = (dtl.Hour * 60 + dtl.Min >= startHourEk
			    * 60 + startMinEk) ? dtl.Hour : (dtl.Hour + 24);

		    if ((startHourEk * 60 + startMinEk <= CurHour * 60
			    + dtl.Min) && (FnEKHour * 60 + finishMinEk
			    > CurHour * 60 + dtl.Min))
			{
			SetChannelBitSignal(i, RAM.OutputCommand, 1);
			continue;
			}
		    else
			{
			SetChannelBitSignal(i, RAM.OutputCommand, 0);
			continue;
			}
		    }
		else
		    {
		    SetChannelBitSignal(i, RAM.OutputCommand, 1);
		    continue;
		    }
		}
	    }

	}

    //error and condition module for iFIX
    for (int i = 0; i < 4; i++)
	{
	RAM.ErrorAndConditionModule[i * 2] = RAM.ErrorDiscretLogic[i];
	RAM.ErrorAndConditionModule[i * 2 + 1] = RAM.diskrets[i];
	}

    CheckCUSignal();

    // send signals to rele
    uint8 tempReleMaskCounter = 0;

    for (int i = 0; i < 8; i++)
	{
	if (pxConfig->devcfg.logica.confCU[i].releNum == 0)
	    continue;
	uint8 TempReleInd = pxConfig->devcfg.logica.confCU[i].releNum - 1;
	SetRelayToRam(TempReleInd, GetChannelBitSignal(i, RAM.OutputCommand));

	tempReleMaskCounter |= (1 << TempReleInd);
	}
    //switch off unused relays
    for (int i = 0; i < 8; i++)
	{
	if ((tempReleMaskCounter & (1 << i)) == 0)
	    {
	    SetRelayToRam(i, 0);
	    }
	}

    //Get Channel Condition CU
    for (int i = 0; i < 8; i++)
	{
	/*
	 if (pxConfig->devcfg.logica.confCU[i].grafNum == 0)
	 {
	 RAM.ChannelCondition &= ~(1 << i);
	 continue;
	 }*/
	if (pxConfig->devcfg.logica.confCU[i].discNum == 0)
	    continue;
	if (GetDiskretFromRam(pxConfig->devcfg.logica.confCU[i].discNum - 1)
		== 0)
	    {
	    RAM.ChannelCondition &= (uint8) (~(1 << i));
	    }
	else
	    {
	    RAM.ChannelCondition |= (uint8) (1 << i);
	    }
	}

    //Check Channel
    for (int i = 0; i < 8; i++)
	{

	 if (pxConfig->devcfg.logica.confCU[i].discNum == 0)
	 {
	 RAM.ErrorChannel &= ~(1 << i);
	 continue;
	 }
	if (((RAM.ChannelCondition) & (1 << i)) >> i != GetChannelBitSignal(i,
		RAM.OutputCommand))
	    {
	    RAM.ErrorChannel |= (1 << i);
	    }
	else
	    {
	    RAM.ErrorChannel &= ~(1 << i);
	    }
	}

    if (RAM.ErrorChannel != 0)
	{
	RAM.LogicErrorFlags |= (1 << 1);
	}
    else
	{
	RAM.LogicErrorFlags &= ~(1 << 1);
	}

    //Check Fuse on Channels
    for (int i = 0; i < 8; i++)
	{
	/*
	 if (pxConfig->devcfg.logica.confCU[i].grafNum == 0)
	 {
	 RAM.ErrorFuseChannel &= ~(1 << i);
	 continue;
	 }*/
	if (pxConfig->devcfg.logica.confCU[i].releNum == 0)
	    continue;
	if (pxConfig->devcfg.logica.confCU[i].discNum == 0)
	    continue;
	if (GetDiskretFromRam(pxConfig->devcfg.logica.confCU[i].discNum - 1))
	    {
	    if (CheckDiscretMask(pxConfig->devcfg.logica.confCU[i].Mask))
		{
		RAM.ErrorFuseChannel &= ~(1 << i);
		}
	    else
		{
		RAM.ErrorFuseChannel |= (1 << i);
		}
	    }
	else
	    {
	    if (CheckDiscretMaskNegative(pxConfig->devcfg.logica.confCU[i].Mask))
		{
		RAM.ErrorFuseChannel &= ~(1 << i);
		}
	    else
		{
		RAM.ErrorFuseChannel |= (1 << i);
		}
	    }
	}

    if (RAM.ErrorFuseChannel != 0)
	{
	RAM.LogicErrorFlags |= (1 << 4);
	}
    else
	{
	RAM.LogicErrorFlags &= ~(1 << 4);
	}

    //check Security
    if (CheckDiscretMask(pxConfig->devcfg.logica.MaskSecurity))
	{
	RAM.LogicErrorFlags &= ~(1 << 2);
	}
    else
	{
	RAM.LogicErrorFlags |= (1 << 2);
	}

    CheckErrors();
    CheckDiscretsErrors();

    SetCRC(&RAM.LogicErrorFlags, 18);

    }
