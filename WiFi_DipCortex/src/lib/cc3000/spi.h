typedef void (*gcSpiHandleRx)(void *p);
typedef void (*gcSpiHandleTx)(void);

void Spi_IrqEnable ( void );
void Spi_IrqDisable ( void );

extern unsigned char SpiTxBuffer[];


//#define SPI_IRQ_INT_EN()				( GPIOPinIntEnable( 0, 0 ) )
//#define SPI_IRQ_INT_DIS()				( GPIOPinIntDisable( 0, 0 ) )

#define WLAN_IRQ_PIN_NO					16
#define WLAN_IRQ_PIN_MASK				1<<WLAN_IRQ_PIN_NO
#define WLAN_IRQ_PORT					1
#define WLAN_IRQ()						(LPC_GPIO->PIN[WLAN_IRQ_PORT] & WLAN_IRQ_PIN_NO)

#define SPI_CS_PIN_NO					19
#define SPI_CS_PIN_MASK					1<<SPI_CS_PIN_NO
#define SPI_CS_PORT						1

#define SPI_CS_ASSERT()          		(LPC_GPIO->CLR[SPI_CS_PORT] = SPI_CS_PIN_MASK)
#define SPI_CS_DEASSERT()        		(LPC_GPIO->SET[SPI_CS_PORT] = SPI_CS_PIN_MASK)


#define WLAN_EN_PIN_NO					15
#define WLAN_EN_PIN_MASK				1<<WLAN_EN_PIN_NO
#define WLAN_EN_PORT					1
#define SPI_WLAN_EN()					(LPC_GPIO->SET[WLAN_EN_PORT] = WLAN_EN_PIN_MASK)
#define SPI_WLAN_DIS()					(LPC_GPIO->CLR[WLAN_EN_PORT] = WLAN_EN_PIN_MASK)

/* SSP Status register */
#define SSPSR_TFE       (0x1<<0)			// Transmit FIFO Empty. This bit is 1 is the Transmit FIFO is empty, 0 if not.
#define SSPSR_TNF       (0x1<<1)			// Transmit FIFO Not Full. This bit is 0 if the Tx FIFO is full, 1 if not
#define SSPSR_RNE       (0x1<<2)			// Receive FIFO Not Empty. This bit is 0 if the Receive FIFO is empty, 1 if not.
#define SSPSR_RFF       (0x1<<3)			// Receive FIFO Full. This bit is 1 if the Receive FIFO is full, 0 if not.
#define SSPSR_BSY       (0x1<<4)			// Busy. This bit is 0 if the SPI controller is idle, 1 if it is currently sending/receiving a frame and/orthe Tx FIFO is not empty.

/* SSP CR0 register */
#define SSPCR0_DSS      (0x1<<0)
#define SSPCR0_FRF      (0x1<<4)
#define SSPCR0_SPO      (0x1<<6)
#define SSPCR0_SPH      (0x1<<7)
#define SSPCR0_SCR      (0x1<<8)

/* SSP CR1 register */
#define SSPCR1_LBM      (0x1<<0)
#define SSPCR1_SSE      (0x1<<1)
#define SSPCR1_MS       (0x1<<2)
#define SSPCR1_SOD      (0x1<<3)

/* SSP Interrupt Mask Set/Clear register */
#define SSPIMSC_RORIM   (0x1<<0)
#define SSPIMSC_RTIM    (0x1<<1)
#define SSPIMSC_RXIM    (0x1<<2)
#define SSPIMSC_TXIM    (0x1<<3)

/* SSP0 Interrupt Status register */
#define SSPRIS_RORRIS   (0x1<<0)
#define SSPRIS_RTRIS    (0x1<<1)
#define SSPRIS_RXRIS    (0x1<<2)
#define SSPRIS_TXRIS    (0x1<<3)

/* SSP0 Masked Interrupt register */
#define SSPMIS_RORMIS   (0x1<<0)
#define SSPMIS_RTMIS    (0x1<<1)
#define SSPMIS_RXMIS    (0x1<<2)
#define SSPMIS_TXMIS    (0x1<<3)

/* SSP0 Interrupt clear register */
#define SSPICR_RORIC    (0x1<<0)
#define SSPICR_RTIC     (0x1<<1)

void SpiInit ( void );
void SpiClose(void);
void SpiOpen (gcSpiHandleRx pfRxHandler);
void SpiResumeSpi (void);
long SpiWrite (unsigned char *pUserBuffer, unsigned short usLength);



#define READ                    			3
#define WRITE                   			1

#define HI(value)               			(((value) & 0xFF00) >> 8)
#define LO(value)               			((value) & 0x00FF)

#define SPI_HEADER_SIZE						(5)
#define HEADERS_SIZE_EVNT       			(SPI_HEADER_SIZE + 5)

#define eSPI_STATE_POWERUP 				 	(0)
#define eSPI_STATE_INITIALIZED  		 	(1)
#define eSPI_STATE_IDLE					 	(2)
#define eSPI_STATE_WRITE_IRQ	   		 	(3)
#define eSPI_STATE_WRITE_FIRST_PORTION   	(4)
#define eSPI_STATE_WRITE_EOT			 	(5)
#define eSPI_STATE_READ_IRQ				 	(6)
#define eSPI_STATE_READ_FIRST_PORTION	 	(7)
#define eSPI_STATE_READ_EOT				 	(8)

// The magic number that resides at the end of the TX/RX buffer (1 byte after the allocated size)
// for the purpose of detection of the overrun. The location of the memory where the magic number
// resides shall never be written. In case it is written - the overrun occured and either recevie function
// or send function will stuck forever.
#define CC3000_BUFFER_MAGIC_NUMBER 			(0xDE)


#ifdef _SPI_

gcSpiHandleRx  SPI_HciRxFunc;

typedef struct
{
	//unsigned short usTxPacketLength;
	unsigned short usRxPacketLength;
	//unsigned long  ulSpiState;
	unsigned char *pTxPacket;
	//unsigned char *pRxPacket;

}tSpiInformation;

tSpiInformation sSpiInformation;



//
// Static buffer for 5 bytes of SPI HEADER
//
unsigned char tSpiReadHeader[] = {READ, 0, 0, 0, 0};

#endif
