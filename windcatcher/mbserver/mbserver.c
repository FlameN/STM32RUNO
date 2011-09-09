#include <stdlib.h>
#include <string.h>

#include "typedef.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "ppp.h"

#include "mbserver.h"
#include "rvnettcp.h"

void mbsThread(void *)__attribute__((naked));
int cxPacketFail;
#define MBSERVER_BUFFSIZE 256
unsigned char mbserverbuf[MBSERVER_BUFFSIZE];
/*------------------------------------------------------------*/

/*
 * Process an incoming connection on port 80.
 *
 * This simply checks to see if the incoming data contains a GET request, and
 * if so sends back a single dynamically created page.  The connection is then
 * closed.  A more complete implementation could create a task for each
 * connection.
 */
static int mbsProcessConnection(struct netconn *pxNetCon);

void mbsStartServer(void)
{
	xTaskCreate (mbsThread, ( const signed portCHAR * const ) "mbsServer",
			500,
			NULL,
			MBSEVER_TASK_PRIORITY,
			( xTaskHandle * ) NULL);
}
void mbsStopServer()
{

}
void mbsThread(void *pvParameters)
{
	struct netconn *pxMBSListener, *pxNewConnection;
	cxPacketFail = 0;
	//dbgmessage("RVnet TCP thread started \r\n");
	pxMBSListener = netconn_new(NETCONN_TCP);
	netconn_bind(pxMBSListener, NULL, MBS_PORT);
	netconn_listen(pxMBSListener);
	pxMBSListener->recv_timeout = 2000;
	//dbgmessage("RVnet TCP init... complite \r\n");
	while (1)
	{
		if (GetPPPThreadControl() != CONNECTED)
		{
			//dbgmessage("Rvnet TCP Server waiting ppp connection !!! ... \r\n");
		}
		while (GetPPPThreadControl() != CONNECTED)
		{
			vTaskDelay(10);
		}
		pxNewConnection = netconn_accept(pxMBSListener);
		if (pxNewConnection != NULL)
		{
			//dbgmessage(">MBS NETCON Connected ... \n");
			while ((GetPPPThreadControl() == CONNECTED) && (pxNewConnection
					!= NULL) && (mbsProcessConnection(pxNewConnection) != 0))
			{
				vTaskDelay(5);
			}
			netconn_close(pxNewConnection);
			while (netconn_delete(pxNewConnection) != ERR_OK)
			{
				vTaskDelay(10);
			}
			//dbgmessage(">MBS NETCON Delete ... \n");
		}
		vTaskDelay(5);
	}
}

static int mbsProcessConnection(struct netconn *pxNetCon)
{
	struct netbuf *pxRxBuffer;
	portCHAR *pcRxString;
	unsigned portSHORT usLength;

	int i;
	/* We expect to immediately get data. */
	//dbgmessage("Netconn Receive ... \n");
	pxNetCon->recv_timeout = 4*60*1000;// 4 min and close

	pxRxBuffer = netconn_recv(pxNetCon);
	//dbgmessage("Netconn Data Received ... \n");
	//dbgStackFreeSize("\n\rRvnet TCP Server Task Stack");
	if (pxRxBuffer != NULL)
	{


		/* Where is the data? */
		//dbgmessage("Process Received Data... \n");

		netbuf_data(pxRxBuffer, (void *) &pcRxString, &usLength);
		//trace(">Rvnet TCP Received  %d bytes  \r\n", (int) usLength);

		if (usLength != 0)
		{
			//LED_MODE_GREEN(1);
			i = 0;
			while (i < usLength)
			{
				mbserverbuf[i] = pcRxString[i];
				i++;
				if (i >= MBSERVER_BUFFSIZE)
				{
					usLength = i;
					break;
				}
			}
			//trace("\n\r");
			for (int j = 0; j < usLength; j++)
			{
				//trace("0x%02X ,", mbserverbuf[j]);
			}
			//trace("\n\r");
			//LED_MODE_RED(1);
			i = RVnetTcpSlaveProcess(mbserverbuf, usLength, 1);
			if (i != 0)
			{
				netconn_write(pxNetCon, mbserverbuf, i, NETCONN_COPY);
				//trace("RVnet TCP Sended  %d bytes  \r\n", i);
				//trace("\n\r");
				//for (int j = 0; j < i; j++)
				//{
					//trace("0x%02X ,", mbserverbuf[j]);
				//}
				//trace("\n\r");

			}
			else
			{
				cxPacketFail++;
				//trace("!!!   RVnet TCP Bad Frame received : %d \n",
//						cxPacketFail);
			}

		}
		netbuf_delete(pxRxBuffer);

		//LED_MODE_GREEN(0);
		//LED_MODE_RED(0);
		return 1;
	}
	else
	{
		//dbgmessage(">MBS NETCON CONNECTION CLOSE ... \r\n");
		return 0;
	}
	return 0;
}
