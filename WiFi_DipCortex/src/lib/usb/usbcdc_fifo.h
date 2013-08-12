#define CDC_FIFO_SIZE		1024


// function prototypes
uint16_t UsbCdcTxFifo_CopyTo ( uint8_t *target, uint16_t len );

bool UsbCdcTxFifo_GetChar ( char *data );
bool UsbCdcRxFifo_GetChar ( char *data );
uint16_t UsbCdcRxFifo_GetString ( uint8_t *data, char terminator, uint16_t maxLen  );

uint16_t UsbCdcTxFifo_AddBytes ( char *data, uint16_t len );
uint16_t UsbCdcRxFifo_AddBytes ( char *data, uint16_t len );

void UsbCdcTxFifo_Clear ( void );

#ifdef _USB_CDC_FIFO_

uint16_t 	CdcFifoTxHead = 0;
uint16_t 	CdcFifoTXTail = 0;
__DATA(RAM3) uint8_t 	CdcFifoTx[CDC_FIFO_SIZE];
bool 		CdcFifoTxFull = false;

uint16_t 	CdcFifoRxHead = 0;
uint16_t 	CdcFifoRXTail = 0;
__DATA(RAM3) uint8_t 	CdcFifoRx[CDC_FIFO_SIZE];
bool 		CdcFifoRxFull = false;

#endif

