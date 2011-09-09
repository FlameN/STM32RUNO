/* Serial operations for SLIP */

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/netif.h"

#include "FreeRTOS.h"
#include "task.h"
#include "usart2.h"
#include "board.h"


uint8_t sio_buf[256];
int bufpoint, bufsize;
#define SIOFIFOSIZE 1024
uint8_t sio_fifo[SIOFIFOSIZE];

bool IsLiteral(char ch)
{
	if ((ch >= ' ') && (ch <= '~'))
	{
		return true;
	}
	return false;
}

void InitSioBuf()
{
	bufpoint = 0;
	bufsize = 0;
}

bool PopFromBuf(char *ch)
{
	if (bufpoint >= bufsize)
	{
		return false;
	}
	*ch = sio_buf[bufpoint];
	bufpoint++;
	return true;
}

void FillBuf()
{
	InitSioBuf();
	uint16_t time = 2000;
	uint16_t counter = 0;
	do
	{
		counter++;
		vTaskDelay(1);
	}while(ReceivedMsg.flag == false && counter < time);
	if(ReceivedMsg.flag == true)
	{
		for(int i=0;i<ReceivedMsg.size;i++)
		{
			sio_buf[i] = ReceivedMsg.buffer[i];
		}
		bufsize =ReceivedMsg.size;
	}

}

bool ReadString(void * dev, char *str, int size)
{
	char ch;
	if (bufpoint >= bufsize)
	{
		FillBuf(dev);
	}
	do
	{
		if (PopFromBuf(&ch) == false)
			return false;
	} while (!IsLiteral(ch));

	while (IsLiteral(ch))
	{
		*str = ch;
		str++;

		if (PopFromBuf(&ch) == false)
			return false;
	};
	*str = 0;
	// pop 1 more symbol 0x0D & 0x0A
	return PopFromBuf(&ch);
}

u32_t sys_jiffies()
{
	return (xTaskGetTickCount() / 10);
}
void * sio_open(int devnum)
{
	InitSioBuf();
#if LWIP_SLIP
#define SIODEV SLIP_DEV
#elif PPP_SUPPORT
#define SIODEV PPP_DEV
#endif
	uart2Init(115200);
	USART2TIMConfigure(50);
	return (void*) (0);
}

void sio_send(char c, void * dev)
{
	uart2PutChar(c);
	uart2TxTask();
}

char sio_recv(void * dev)
{
	char ch;
	if (PopFromBuf(&ch) == false)
	{
		FillBuf(dev);
		if (PopFromBuf(&ch) == false)
			return false;
	}
	return ch;
}

int sio_write(void * dev, u8_t *b, u32_t size)
{
	uart2Write((uint8_t*) b,size);
	return size;
}
int sio_tryread(void *fd, uint8_t *data, uint32_t len)
{
	return 0;
}
int sio_read(void *dev, char *b, int size)
{

	int rxsize, totalsize;
	totalsize = 0;
	do
	{
		uint16_t time = 4000;
			uint16_t counter = 0;
			do
			{
				counter++;
				vTaskDelay(1);
			}while(ReceivedMsg.flag == false && counter < time);
			if(ReceivedMsg.flag == true)
			{
				for(int i=0;i<ReceivedMsg.size;i++)
				{
					b[i] = ReceivedMsg.buffer[i];
				}
				rxsize =ReceivedMsg.size;
			}
		//rxsize = AT91F_USART_RECEIVE((AT91_USART_DEV *) dev, (uint8*) ((int) b
				//+ totalsize), size, 4000);
		if (rxsize == 0)
		{

			return totalsize;
		}
		totalsize += rxsize;
		if (totalsize >= size)
		{
			return size;
		};
		// dbgmessage("\n\r>Sio Read  part \n\r");
	} while (*((uint8_t*) ((int) b + totalsize - 1)) != 0x7E);
	// dbgmessage("\n\r>Sio Read  complite \n\r");

	return totalsize;
}

void sio_read_abort(void * dev)
{

}
