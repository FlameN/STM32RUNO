#include <stdlib.h>
#include <stdarg.h>

#include "typedef.h"
#include "FreeRTOS.h"
#include "task.h"
#include "usart2.h"
#include "gprscon.h"
#include "dbg.h"
#include "rvnettcp.h"

#include "board.h"
#include "modem.h"


#define GPRS_IDLE_TIMEOUT_MS       ( 60*4) // 4 min idle and restart
#define GPRS_BUF_SIZE      		   ( 256)


uint32 GprsIdleMSec;
uint32 GprsConnectionsAttempt;

int gprssize;

uint8 gprsbuf[GPRS_BUF_SIZE];

void GprsIdleIncMSec()
{
	GprsIdleMSec++;
}

void vGprsConTask(void *pvParameters)
{
	(void) pvParameters;

	GprsConnectionsAttempt = 0;
	volatile int heapSize = 0;
	while (1)
	{
		trace("Gprs Connection attempt %d", GprsConnectionsAttempt);
		GprsConnectionsAttempt++;
		ModemInit();
		GprsIdleMSec = 0;

		while (1)
		{
			gprssize = ModemReceiveData(gprsbuf,GPRS_BUF_SIZE);

			heapSize = xPortGetFreeHeapSize();
			dbgStackFreeSize("");
			heapSize +=0;
			if (gprssize != 0)
			{
				//gprssize = RVnetSlaveProcess(gprsbuf, gprssize, 1);
				gprssize = RVnetTcpSlaveProcess(gprsbuf, gprssize, 1);
				heapSize = xPortGetFreeHeapSize();
				heapSize +=0;
				if (gprssize != 0)
				{
					ModemSendData(gprsbuf, gprssize);
					heapSize = xPortGetFreeHeapSize();
					heapSize +=0;
					portENTER_CRITICAL();
					{
						GprsIdleMSec = 0;
					}
					portEXIT_CRITICAL();
				}
			}
			if (GprsIdleMSec > GPRS_IDLE_TIMEOUT_MS)
			{
				dbgmessage("GPRS IDLE timeout. Restart connection now.");
				break; // try to Init modem again
			}
			vTaskDelay(1);
			// TODO if timeout break to ModemInit
		}

		vTaskDelay(1000);
	}
}

