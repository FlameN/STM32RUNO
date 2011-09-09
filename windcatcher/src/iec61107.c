#include "iec61107.h"

char iecClaculateBCC(char *pxBuf, uint16 size)
{
	char bcc = 0;
	while (size)
	{
		bcc += *pxBuf;
		pxBuf++;
		size--;
	}
	return (bcc & 0x7F);
}
uint16 iecProcReqPacket(char *pxBuf, char *addr, uint8 limit)
{
	uint16 size = 0;
	*pxBuf = '/';
	pxBuf++;
	*pxBuf = '?';
	pxBuf++;
	while (*addr)
	{
		if (limit == 0)
			return 0;
		*pxBuf = (*addr);
		pxBuf++;
		addr++;
		limit--;
		size++;
	}
	*pxBuf = '!';
	pxBuf++;
	*pxBuf = SYM_CR;
	pxBuf++;
	*pxBuf = SYM_LF;
	pxBuf++;
	return (2 + size + 3);
}
bool iecIndProc(char *pxBuf, IDENTIFIER *idStr)
{
	int i;
	if (*pxBuf != '/')
		return false;
	pxBuf++;
	for (i = 0; i < 3; i++)
	{
		idStr->manufator[i] = *pxBuf;
		pxBuf++;
	}
	idStr->speed = *pxBuf;
	pxBuf++;
	for (i = 0; i < 16; i++)
	{
		if (*pxBuf == SYM_CR)
			break;
		idStr->ident[i] = *pxBuf;
		pxBuf++;
	}
	if ((*pxBuf != SYM_CR) && (*(pxBuf + 1) != SYM_LF))
		return false;
	return true;
}
uint16 iecOptionsAckPacket(char *pxBuf, char V, char Z, char Y)
{
	*pxBuf = ACK;
	pxBuf++;
	*pxBuf = V;
	pxBuf++;
	*pxBuf = Z;
	pxBuf++;
	*pxBuf = Y;
	pxBuf++;
	*pxBuf = SYM_CR;
	pxBuf++;
	*pxBuf = SYM_LF;
	return 6;
}
bool iecConnect(char *devaddr, IDENTIFIER *idStr)
{
	//uint8 buf[32];
	//uint16 size;
	/*
	 size = iecReqPacket(buf,devaddr,10);
	 iecSysWrite(buf,size);
	 size = iecSysRead(buf,32);
	 if(iecIndProc(buf,idStr) == false ) return false;
	 size = iecOptionsAckPacket(buf,V_NORM,Z_9600,Y_PROG);
	 iecSysWrite(buf,size);
	 size = iecReqPacket(buf,devaddr,10);
	 // todo security proc;
	 */

	return true;
}
bool iecProcInputPacked(char *pxBuf, uint16 size, IDENTIFIER *idStr)
{
	switch (*pxBuf)
	{
	case SYM_ST:
		return iecProcInputPacked(pxBuf, size - 1, idStr);
	case ACK:
		break;
	case NAK:
		break;
	case STX:
		break;
	case SOH:
		break;
	}
	return true;
}
bool iecProcAddrAnswer(char *pxBuf, char *addrStr, uint8 limit)
{
	int i;
	uint8 *tmpbuff = pxBuf;
	if (*pxBuf != SOH)
		return false;
	pxBuf++;
	if (*pxBuf != 'P')
		return false;
	pxBuf++;
	if (*pxBuf != '0')
		return false;
	pxBuf++;
	if (*pxBuf != STX)
		return false;
	pxBuf++;
	if (*pxBuf != '(')
		return false;
	pxBuf++;
	i = 0;
	while ((*pxBuf != ')') && (limit > 0))
	{
		if (i >= limit)
			return false;
		*addrStr = (char) *pxBuf;
		i++;
		addrStr++;
		pxBuf++;
	}
	*addrStr = 0; // end of string
	pxBuf++;
	if (*pxBuf != ETX)
		return false;
	pxBuf++;
	uint8 bcc = iecClaculateBCC((tmpbuff + 1), 3 + 1 + i + 1 + 1);
	if (bcc != *pxBuf)
	{
		return false;
	}
	return true;
}
uint16 iecProcPassPacket(char *pxBuf, char *pass, uint8 limit)
{
	uint8 *tmpbuff = pxBuf;
	*pxBuf = SOH;
	pxBuf++;
	*pxBuf = 'P';
	pxBuf++;
	*pxBuf = '1';
	pxBuf++;
	*pxBuf = STX;
	pxBuf++;
	*pxBuf = '(';
	pxBuf++;
	int i = 0;
	while (*pass)
	{
		if (i >= limit)
			return 0;
		*pxBuf = *pass;
		pxBuf++;
		pass++;
		i++;
	}
	*pxBuf = ')';
	pxBuf++;
	*pxBuf = ETX;
	pxBuf++;
	*pxBuf = iecClaculateBCC(tmpbuff + 1, 4 + i + 1 + 1);
	return (5 + i + 2 + 1);
}
uint16 iecProcVarReqPacket(char *pxBuf, char *var, uint8 limit)
{
	uint8 *tmpbuff = pxBuf;
	*pxBuf = SOH;
	pxBuf++;
	*pxBuf = 'R';
	pxBuf++;
	*pxBuf = '1';
	pxBuf++;
	*pxBuf = STX;
	pxBuf++;
	int i = 0;
	while (*var)
	{
		if (i >= limit)
			return 0;
		*pxBuf = *var;
		pxBuf++;
		var++;
		i++;
	}

	*pxBuf = '(';
	pxBuf++;

	*pxBuf = ')';
	pxBuf++;
	*pxBuf = ETX;
	pxBuf++;
	*pxBuf = iecClaculateBCC(tmpbuff + 1, 3 + i + 2 + 1);
	return (4 + i + 2 + 2);
}

uint16 iecProcVarReqPacketArg(char *pxBuf, char *var, uint8 limit,char *param)
{
	uint8 *tmpbuff = pxBuf;
	*pxBuf = SOH;
	pxBuf++;
	*pxBuf = 'R';
	pxBuf++;
	*pxBuf = '1';
	pxBuf++;
	*pxBuf = STX;
	pxBuf++;
	int i = 0;
	while (*var)
	{
		if (i >= limit)
			return 0;
		*pxBuf = *var;
		pxBuf++;
		var++;
		i++;
	}

	*pxBuf = '(';
	pxBuf++;
	int j = 0;
	while (*param)
	{
		if (j >= limit)
			return 0;
		*pxBuf = *param;
		pxBuf++;
		param++;
		j++;
	}
	*pxBuf = ')';
	pxBuf++;
	*pxBuf = ETX;
	pxBuf++;
	*pxBuf = iecClaculateBCC(tmpbuff + 1, 3 + i + j + 2 + 1);
	return (4 + i +j + 2 + 2);
}

bool iecProcVarAnswerPacket(char *pxBuf, char *value, uint8 limit)
{
	int i = 0;
	uint8 *tmpbuff = pxBuf;
	if (*pxBuf != STX)
		return false;
	pxBuf++;
	while (*pxBuf != ETX)
	{
		if (i >= limit)
			return false;
		*value = (char) *pxBuf;
		i++;
		value++;
		pxBuf++;
	};

	pxBuf++;
	uint8 bcc = iecClaculateBCC((tmpbuff + 1), i + 1);
	if (bcc != *pxBuf)
	{
		return false;
	}
	return true;
}

uint16 iecProcExitPacket(char *pxBuf)
{
	*pxBuf = 0x01;
	pxBuf++;
	*pxBuf = 0x42;
	pxBuf++;
	*pxBuf = 0x30;
	pxBuf++;
	*pxBuf = 0x03;
	pxBuf++;
	*pxBuf = 0x75;
	return 5;
}
