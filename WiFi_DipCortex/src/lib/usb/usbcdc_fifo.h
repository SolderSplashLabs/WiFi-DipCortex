#define CDC_FIFO_SIZE		1024


// function prototypes
uint16_t UsbCdcTxFifo_CopyTo ( uint8_t *target, uint16_t len );

bool UsbCdcTxFifo_GetByte ( uint8_t *data );
bool UsbCdcRxFifo_GetByte ( uint8_t *data );
uint16_t UsbCdcRxFifo_GetString ( uint8_t *data, char terminator, uint16_t maxLen  );

uint16_t UsbCdcTxFifo_AddBytes ( uint8_t *data, uint16_t len );
uint16_t UsbCdcRxFifo_AddBytes ( uint8_t *data, uint16_t len );

#ifdef _USB_CDC_FIFO_

uint16_t 	CdcFifoTxHead = 0;
uint16_t 	CdcFifoTXTail = 0;
uint8_t 	CdcFifoTx[CDC_FIFO_SIZE];
bool 		CdcFifoTxFull = false;

uint16_t 	CdcFifoRxHead = 0;
uint16_t 	CdcFifoRXTail = 0;
uint8_t 	CdcFifoRx[CDC_FIFO_SIZE];
bool 		CdcFifoRxFull = false;

#endif

