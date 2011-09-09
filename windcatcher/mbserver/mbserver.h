#ifndef _WEBSERVER
#define _WEBSERVER
#include "gprscon.h"

#define  MBSEVER_TASK_PRIORITY ( tskIDLE_PRIORITY + 2 )

/* The port on which we listen. */
#define MBS_PORT            ( 4444 )


extern void mbsStartServer(void);

#endif



