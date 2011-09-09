#ifndef IEC61107_H
#define IEC61107_H
#include "typedef.h"
// Main protocol symbols
#define SYM_ST		'/'
#define SYM_TR		'?'
#define SYM_CR		0x0D
#define SYM_LF		0x0A
#define ACK		0x06
#define NAK		0x15
#define STX		0x02
#define SOH		0x01
#define ETX		0x03
#define EOT		0x04

#define V_NORM		'0'
#define V_VTOR		'1'

#define Z_300		'0'
#define Z_600		'1'
#define Z_1200		'2'
#define Z_2400		'3'
#define Z_4800		'4'
#define Z_9600		'5'

#define Y_READ		'0'
#define Y_PROG		'1'

typedef struct
{
	char manufator[3]; // Manufactor 3 sym
	char speed; // speed '5' - 9600
	char ident[16]; // up to 16 symbols
}__attribute__((packed)) IDENTIFIER;


extern uint16 iecProcReqPacket(char *pxBuf, char *addr, uint8 limit);
extern bool iecIndProc(char *pxBuf, IDENTIFIER *idStr);
uint16 iecOptionsAckPacket(char *pxBuf, char V, char Z, char Y);
extern bool iecProcAddrAnswer(char *pxBuf,char *addrStr, uint8 limit);
extern uint16 iecProcPassPacket(char *pxBuf, char *pass, uint8 limit);
extern uint16 iecProcExitPacket(char *pxBuf);
extern uint16 iecProcVarReqPacket(char *pxBuf, char *var, uint8 limit);
extern bool   iecProcVarAnswerPacket(char *pxBuf, char *value, uint8 limit);

#endif
