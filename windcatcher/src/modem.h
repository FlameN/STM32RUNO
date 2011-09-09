#ifndef MODEM_H
#define MODEM_H
#include "typedef.h"
typedef enum
{
	MA_UNKNOWN = 0,
	MA_CALLREADY,
	MA_OK,
	MA_SERVEROK,
	MA_NORMALPOWERDOWN,
	MA_STATEIPSTATUS,
	MA_STATEIPCLOSE,
	MA_STATEIPINITIAL,
	MA_STATEIPGPRSACT,
	MA_STATEPDPDEACT,
	MA_CIPSENDREADY,
	MA_CONNECT,
	MA_NOCARRIER,
	MA_ERROR

} MDEM_ANSWER;
#define AT_AMOUNT (MA_ERROR - 1)


typedef struct
{
	char *str;
	MDEM_ANSWER ma;
} AT_COMMAND;

#define PPP_USER                "bemn_7650657"
#define PPP_PASS                "7650657"
#define PPP_ADPARAM             "web1.velcom.by"
//#define PPP_USER                "bemn_7650654"
//#define PPP_PASS                "7650654"
#define PPP_ADDSTR              "AT+CGDCONT=1,\"IP\",\"%s\"\r"
#define PPP_CSTT 				"AT+CSTT=\"%s\",\"%s\",\"%s\"\r"

//#define PPP_USER                ""
//#define PPP_PASS                ""
//#define PPP_ADDSTR              "AT+CGDCONT=1,\"IP\",\"lidazhkh.velcom.by\"\r"

extern void ModemInit();
extern int ModemReceiveData(uint8 *,int);
extern void ModemSendData(uint8 *, uint8);

#endif
