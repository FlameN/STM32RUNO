/*
 * rvnettcp.c
 *
 *  Created on: Jul 6, 2010
 *      Author: albert
 */
//======================================================================
#include"rvnettcp.h"
#include "memman.h"
#include "crc.h"

//extern void BOOTSTART( void );

// * <------------------------ RV TCP/IP ADU(1) ------------------------->
// *              <----------- RV PDU (1') ---------------->
// *  +-----------+---------------+------------------------------------------+
// *  | TID | PID | Length | UID  |Code | Data                               |
// *  +-----------+---------------+------------------------------------------+
// *  |     |     |        |      |
// * (2)   (3)   (4)      (5)    (6)
// *
// * (2)  ... RVTCP_TID          = 0 (Transaction Identifier - 2 Byte)
// * (3)  ... RVTCP_PID          = 2 (Protocol Identifier - 2 Byte)
// * (4)  ... RVTCP_LEN          = 4 (Number of bytes - 2 Byte)
// * (5)  ... RVTCP_UID          = 6 (Unit Identifier - 1 Byte)
// * (6)  ... RVTCP_FUNC         = 7 (Modbus Function Code)

#define RVTCP_TID	0
#define RVTCP_PID	2
#define RVTCP_LEN	4
#define RVTCP_UID	6
#define RVTCP_FUNC	7
#define RVTCP_DATA	8

#define MB_TCP_PROTOCOL_ID  0



RVNET_DATATYPE RVnetTcpSlaveProcess(uint8 *buf, RVNET_DATATYPE pkSize,
		uint8 device_address)
{
	uint16 len, pid;

	len = buf[RVTCP_LEN] << 8U;
	len |= buf[RVTCP_LEN + 1];
	// Len Test
	//if(len != (pkSize-RVTCP_FUNC)){return 0;}

	pid = buf[RVTCP_PID] << 8U;
	pid |= buf[RVTCP_PID + 1];
	//if(pid != MB_TCP_PROTOCOL_ID){return 0;}

	switch (buf[RVTCP_FUNC])
	{
	case 0x00:
		len = ReadDeviceID(&buf[RVTCP_DATA]);
		break;
	case 0x01:
	case 0x02:
		len = ReadNBits(&buf[RVTCP_DATA]);
		break;
	case 0x03:
	case 0x04:
		len = ReadNWords(&buf[RVTCP_DATA]);
		break;
	case 0x05:
		len = WriteBit(&buf[RVTCP_DATA]);
		break;
	case 0x10:
		pkSize = WriteNWords(&buf[RVTCP_DATA]);
		break;
	default:
		len = ErrorAddress(&buf[RVTCP_DATA]);
		break;
	}
	len +=1;// + function code
	buf[RVTCP_LEN] = (len + 1) >> 8U; //+1 - UID
	buf[RVTCP_LEN + 1] = (len + 1) & 0xFF; //+1 - UID

	return (len + RVTCP_FUNC);
}
//======================================================================
