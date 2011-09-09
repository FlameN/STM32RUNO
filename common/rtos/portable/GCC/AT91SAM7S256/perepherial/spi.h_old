#ifndef SPI_DEFINE
#define SPI_DEFINE

#define SPI_DLYBS		(40<<16)
#define BAUD_RATE		200000 //748800
#define SPI_BAUD_RATE		((MCK/(2*BAUD_RATE))<<8)  // Master clock / (50*2) = 16

#define SPI_INTERRUPT_LEVEL      7


typedef struct SPI_PCD_DATA_STRUCT
{
  unsigned TDH       : 8 ;
  unsigned TDL       : 8 ;
  unsigned PCS       : 4 ;
  unsigned RESERV1   : 4 ;
  unsigned LASTXFER  : 1 ;
  unsigned RESERV2   : 7 ;

}SPI_PCD_DATA;

extern void SPI_Init( void );
extern void AT91F_SPI_TRANSACTION_BLOCK (SPI_PCD_DATA *data,int size);
#endif













