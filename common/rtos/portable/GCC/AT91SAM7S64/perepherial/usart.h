#include "typedef.h"



extern void Usart0_init ( long );
extern void Usart1_init ( long );
extern void Usart2_init ( long );

extern AT91PS_USART COM0;
extern AT91PS_USART COM1;
extern AT91PS_USART COM2;

extern void AT91F_US0_TRANSMITT_BLOCK (uint8 *,int);
extern int  AT91F_US0_RECEIVE_BLOCK   (uint8 *,int);
extern void AT91F_US1_TRANSMITT_BLOCK (uint8 *,int);
extern int  AT91F_US1_RECEIVE_BLOCK   (uint8 *,int);
extern void AT91F_US2_TRANSMITT_BLOCK (uint8 *,int);
extern int  AT91F_US2_RECEIVE_BLOCK   (uint8 *,int);













