typedef void (*gcSpiHandleRx)(void *p);
typedef void (*gcSpiHandleTx)(void);

extern unsigned char wlan_tx_buffer[];

#define WLAN_IRQ_PIN_NO		16
#define WLAN_IRQ_PIN_MASK	1<<WLAN_IRQ_PIN_NO
#define WLAN_IRQ_PORT		1


#define SPI_CS_PIN_NO						19
#define SPI_CS_PIN_MASK						1<<SPI_CS_PIN_NO
#define SPI_CS_PORT							1

#define ASSERT_CS()          				(LPC_GPIO->CLR[SPI_CS_PORT] = SPI_CS_PIN_MASK)
#define DEASSERT_CS()        				(LPC_GPIO->SET[SPI_CS_PORT] = SPI_CS_PIN_MASK)

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
