/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2013 - www.soldersplash.co.uk - C. Matthews - R. Steel


	@file     spi.c
	@author   Carl Matthews (soldersplash.co.uk)
	@date     13 June 2013

    @section LICENSE

	Software License Agreement (BSD License)

    Copyright (c) 2013, C. Matthews - R. Steel (soldersplash.co.uk)
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


	@section DESCRIPTION

	Reference http://processors.wiki.ti.com/index.php/CC3000_Host_Driver_Porting_Guide

*/

#include "string.h"
#include "SolderSplashLpc.h"

#include "hci.h"
#include "evnt_handler.h"
#include "delay.h"
#include "gpio.h"


#define _SPI_
#include "spi.h"


// ------------------------------------------------------------------------------------------------------------
/*!
    @brief SpiInit - CC3000 Module uses SPI1 on the DipCortex
*/
// ------------------------------------------------------------------------------------------------------------
void SpiInit ( void )
{
uint8_t i = 0;
volatile uint32_t data;

	// CS Output, high to de-chip select
	DEASSERT_CS();
	LPC_GPIO->DIR[SPI_CS_PORT] |= SPI_CS_PIN_MASK;

	// Enable the SPI1 Clock
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x1<<18);

	// Reset the SPI peripheral
	LPC_SYSCON->PRESETCTRL |= (0x1<<2);

	// Divide by 1 = Peripheral clock enabled
	LPC_SYSCON->SSP1CLKDIV = 1;

	// SPI CLk - IO Config
	LPC_IOCON->PIO1_20 = 2;

	// SSP CS Enable - IO Config
	// We are using it as a GPIO
	//LPC_IOCON->PIO0_2 = 1;

	// MISO Enable - IO Config
	LPC_IOCON->PIO1_21 = 2;

	// MOSI Enable - IO Config
	LPC_IOCON->PIO1_22 = 2;

	// SPI Clock Speed = Perhiperal Clock / ( CPSR * (SCR+1) )
	// Clock Prescale Register - Must be even
	LPC_SSP1->CPSR = 2;

	// DSS  = 0x7, 8-bit
	// FRF  = 0, Frame format SPI
	// CPHA = 1, Data captured on falling edge of the clock
	// CPOL = 0, Clock Low between frames, and SCR is 3 ( 4clocks - 1 )
	// SCR = 2, 36Mhz/(2+1) = 12Mhz Clock rate
	LPC_SSP1->CR0 = 0 | 0x7 | SSPCR0_SPH | 0x0200;

	// Manual CS
	LPC_GPIO->DIR[SPI_CS_PORT] |= SPI_CS_PIN_MASK;

	// Drain the fifo
	for ( i = 0; i < 8; i++ )
	{
		data = LPC_SSP1->DR;
	}

	// SSE - Enabled, Master
	LPC_SSP1->CR1 = 0 | SSPCR1_SSE;
}

//*****************************************************************************
//
//! Write to bus, dump returned data
//!
//!  \param  buffer
//!
//!  \return none
//!
//!  \brief  ...
//
//*****************************************************************************
void SpiWriteDataSynchronous(unsigned char *data, unsigned short size)
{
volatile uint32_t temp = 0;

	while (size)
    {
		// wait while the fifo is full
		while ((LPC_SSP1->SR & SSPSR_TNF) != SSPSR_TNF);

        LPC_SSP1->DR = *data;

		size --;
		data++;

		// Empty any data RX'd we don't want it, or for it to fill our fifo
		temp = LPC_SSP1->DR;
    }

	// Wait until it's all left the TX FIFO
	while (! LPC_SSP1->SR & SSPSR_TFE );
	{
		// dummy read
		temp = LPC_SSP1->DR;
	}

	// Remove all data from the RX FIFO
	while ((LPC_SSP1->SR & (SSPSR_BSY | SSPSR_RNE)))
	{
		temp = LPC_SSP1->DR;
	}
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief SpiOpen
*/
// ------------------------------------------------------------------------------------------------------------
void SpiOpen (gcSpiHandleRx pfRxHandler)
{

	sSpiInformation.ulSpiState = eSPI_STATE_POWERUP;

	memset(spi_buffer, 0, sizeof(spi_buffer));
	memset(wlan_tx_buffer, 0, sizeof(spi_buffer));

	sSpiInformation.SPIRxHandler = pfRxHandler;
	sSpiInformation.usTxPacketLength = 0;
	sSpiInformation.pTxPacket = NULL;
	sSpiInformation.pRxPacket = (unsigned char *)spi_buffer;
	sSpiInformation.usRxPacketLength = 0;

	spi_buffer[CC3000_RX_BUFFER_SIZE - 1] = CC3000_BUFFER_MAGIC_NUMBER;
	wlan_tx_buffer[CC3000_TX_BUFFER_SIZE - 1] = CC3000_BUFFER_MAGIC_NUMBER;

	//
	// Enable interrupt on the GPIO pin of WLAN IRQ
	//
	tSLInformation.WlanInterruptEnable();

}

//*****************************************************************************
//
//! This function enter point for write flow
//!
//!  \param  buffer
//!
//!  \return none
//!
//!  \brief  ...
//
//*****************************************************************************
long SpiFirstWrite(unsigned char *ucBuf, unsigned short usLength)
{
    //
    // workaround for first transaction
    //
    ASSERT_CS();

    DelayUs(100);

    // SPI writes first 4 bytes of data
    SpiWriteDataSynchronous(ucBuf, 4);

    DelayUs(100);

    SpiWriteDataSynchronous(ucBuf + 4, usLength - 4);

    // From this point on - operate in a regular way
    sSpiInformation.ulSpiState = eSPI_STATE_IDLE;

    while (! LPC_SSP1->SR & SSPSR_TFE );
    while ( LPC_SSP1->SR & SSPSR_BSY );

    DEASSERT_CS();

    return(0);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief SpiWrite
*/
// ------------------------------------------------------------------------------------------------------------
long SpiWrite (unsigned char *pUserBuffer, unsigned short usLength)
{
    unsigned char ucPad = 0;

	// Figure out the total length of the packet in order to figure out if there is padding or not
    if(!(usLength & 0x0001))
    {
        ucPad++;
    }

    pUserBuffer[0] = WRITE;
    pUserBuffer[1] = HI(usLength + ucPad);
    pUserBuffer[2] = LO(usLength + ucPad);
    pUserBuffer[3] = 0;
    pUserBuffer[4] = 0;

    usLength += (SPI_HEADER_SIZE + ucPad);

	// The magic number that resides at the end of the TX/RX buffer (1 byte after the allocated size)
	// for the purpose of overrun detection. If the magic number is overwritten - buffer overrun
	// occurred - and we will be stuck here forever!
	if (wlan_tx_buffer[CC3000_TX_BUFFER_SIZE - 1] != CC3000_BUFFER_MAGIC_NUMBER)
	{
		// TODO : Can we do anything?
		while (1);
	}

	if (sSpiInformation.ulSpiState == eSPI_STATE_POWERUP)
	{
		while (sSpiInformation.ulSpiState != eSPI_STATE_INITIALIZED);
	}

	if (sSpiInformation.ulSpiState == eSPI_STATE_INITIALIZED)
	{
		//
		// This is time for first TX/RX transactions over SPI: the IRQ is down - so need to send read buffer size command
		//
		SpiFirstWrite(pUserBuffer, usLength);
	}
	else
	{
		//
		// We need to prevent here race that can occur in case two back to back packets are sent to the
		// device, so the state will move to IDLE and once again to not IDLE due to IRQ
		//
		tSLInformation.WlanInterruptDisable();

		while (sSpiInformation.ulSpiState != eSPI_STATE_IDLE)
		{
			;
		}


		sSpiInformation.ulSpiState = eSPI_STATE_WRITE_IRQ;
		sSpiInformation.pTxPacket = pUserBuffer;
		sSpiInformation.usTxPacketLength = usLength;

		//
		// Assert the CS line and wait till SSI IRQ line is active and then initialize write operation
		//
		ASSERT_CS();

		//
		// Re-enable IRQ - if it was not disabled - this is not a problem...
		//
		tSLInformation.WlanInterruptEnable();

		//
		// check for a missing interrupt between the CS assertion and enabling back the interrupts
		//
		if (tSLInformation.ReadWlanInterruptPin() == 0)
		{
            SpiWriteDataSynchronous(sSpiInformation.pTxPacket, sSpiInformation.usTxPacketLength);

			sSpiInformation.ulSpiState = eSPI_STATE_IDLE;

			DEASSERT_CS();
		}
	}


	//
	// Due to the fact that we are currently implementing a blocking situation
	// here we will wait till end of transaction
	//

	while (eSPI_STATE_IDLE != sSpiInformation.ulSpiState);

    return(0);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief SpiClose
*/
// ------------------------------------------------------------------------------------------------------------
void SpiClose(void)
{
	if (sSpiInformation.pRxPacket)
	{
		sSpiInformation.pRxPacket = 0;
	}

	//	Disable IRQ Interrupt
    tSLInformation.WlanInterruptDisable();
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief SpiResumeSpi
*/
// ------------------------------------------------------------------------------------------------------------
void SpiResumeSpi(void)
{
	GPIOPinIntEnable( 0, 0 );
}


//*****************************************************************************
//
//! This function enter point for write flow
//!
//!  \param  SpiPauseSpi
//!
//!  \return none
//!
//!  \brief  The function triggers a user provided callback for
//
//*****************************************************************************

void SpiPauseSpi(void)
{
	  GPIOPinIntDisable( 0, 0 );
}


//*****************************************************************************
//
//! Read data from the bus
//!
//!  \param  buffer
//!
//!  \return none
//!
//!  \brief  ...
//
//*****************************************************************************
void
SpiReadDataSynchronous(unsigned char *data, unsigned short size)
{
	long i = 0;
    unsigned char *data_to_send = tSpiReadHeader;

	for (i = 0; i < size; i ++)
    {
		while ( LPC_SSP1->SR & SSPSR_BSY );

		//Dummy write to trigger the clock
    	LPC_SSP1->DR = data_to_send[0];

    	while ( LPC_SSP1->SR & SSPSR_BSY );
    	while (! (LPC_SSP1->SR & SSPSR_TFE));

		data[i] = LPC_SSP1->DR;
    }
}

//*****************************************************************************
//
//! This function enter point for read flow: first we read minimal 5 SPI header bytes and 5 Event
//!	Data bytes
//!
//!  \param  buffer
//!
//!  \return none
//!
//!  \brief  ...
//
//*****************************************************************************
void SpiReadHeader(void)
{
	SpiReadDataSynchronous(sSpiInformation.pRxPacket, 10);
}

//*****************************************************************************
//
//! This function enter point for write flow
//!
//!  \param  SpiTriggerRxProcessing
//!
//!  \return none
//!
//!  \brief  The function triggers a user provided callback for
//
//*****************************************************************************
void SpiTriggerRxProcessing(void)
{
	// Trigger Rx processing
	SpiPauseSpi();
	DEASSERT_CS();

	// The magic number that resides at the end of the TX/RX buffer (1 byte after the allocated size)
	// for the purpose of detection of the overrun. If the magic number is overriten - buffer overrun
	// occurred - and we will stuck here forever!
	if (sSpiInformation.pRxPacket[CC3000_RX_BUFFER_SIZE - 1] != CC3000_BUFFER_MAGIC_NUMBER)
	{
		while (1);
	}

	sSpiInformation.ulSpiState = eSPI_STATE_IDLE;
	sSpiInformation.SPIRxHandler(sSpiInformation.pRxPacket + SPI_HEADER_SIZE);
}

//*****************************************************************************
//
//! This function processes received SPI Header and in accordance with it - continues reading
//!	the packet
//!
//!  \param  None
//!
//!  \return None
//!
//!  \brief  ...
//
//*****************************************************************************
long SpiReadDataCont(void)
{
    long data_to_recv;
	unsigned char *evnt_buff, type;

    // determine what type of packet we have
    evnt_buff =  sSpiInformation.pRxPacket;
    data_to_recv = 0;
	STREAM_TO_UINT8((char *)(evnt_buff + SPI_HEADER_SIZE), HCI_PACKET_TYPE_OFFSET, type);

    switch(type)
    {
        case HCI_TYPE_DATA:
        {
			//
			// We need to read the rest of data..
			//
			STREAM_TO_UINT16((char *)(evnt_buff + SPI_HEADER_SIZE), HCI_DATA_LENGTH_OFFSET, data_to_recv);
			if (!((HEADERS_SIZE_EVNT + data_to_recv) & 1))
			{
    	        data_to_recv++;
			}

			if (data_to_recv)
			{
            	SpiReadDataSynchronous(evnt_buff + 10, data_to_recv);
			}
            break;
        }
        case HCI_TYPE_EVNT:
        {
			//
			// Calculate the rest length of the data
			//
            STREAM_TO_UINT8((char *)(evnt_buff + SPI_HEADER_SIZE), HCI_EVENT_LENGTH_OFFSET, data_to_recv);
			data_to_recv -= 1;

			//
			// Add padding byte if needed
			//
			if ((HEADERS_SIZE_EVNT + data_to_recv) & 1)
			{

	            data_to_recv++;
			}

			if (data_to_recv)
			{
            	SpiReadDataSynchronous(evnt_buff + 10, data_to_recv);
			}

			sSpiInformation.ulSpiState = eSPI_STATE_READ_EOT;
            break;
        }
    }

    return (0);
}


//*****************************************************************************
//
//! This function enter point for write flow
//!
//!  \param  SSIContReadOperation
//!
//!  \return none
//!
//!  \brief  The function triggers a user provided callback for
//
//*****************************************************************************

void SSIContReadOperation(void)
{
	//
	// The header was read - continue with  the payload read
	//
	if (!SpiReadDataCont())
	{
		//
		// All the data was read - finalize handling by switching to the task
		//	and calling from task Event Handler
		//
		SpiTriggerRxProcessing();
	}
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Wlan_IrqIntterupt
*/
// ------------------------------------------------------------------------------------------------------------
void Wlan_IrqIntterupt ( void )
{

	if (sSpiInformation.ulSpiState == eSPI_STATE_POWERUP)
	{
		/* This means IRQ line was low call a callback of HCI Layer to inform on event */
		sSpiInformation.ulSpiState = eSPI_STATE_INITIALIZED;
	}
	else if (sSpiInformation.ulSpiState == eSPI_STATE_IDLE)
	{
		sSpiInformation.ulSpiState = eSPI_STATE_READ_IRQ;

		/* IRQ line goes down - start reception */
		ASSERT_CS();

		//
		// Wait for TX/RX Compete which will come as DMA interrupt
		//
		SpiReadHeader();

		sSpiInformation.ulSpiState = eSPI_STATE_READ_EOT;

		SSIContReadOperation();
	}
	else if (sSpiInformation.ulSpiState == eSPI_STATE_WRITE_IRQ)
	{
		SpiWriteDataSynchronous(sSpiInformation.pTxPacket, sSpiInformation.usTxPacketLength);

		sSpiInformation.ulSpiState = eSPI_STATE_IDLE;

		DEASSERT_CS();
	}
}


