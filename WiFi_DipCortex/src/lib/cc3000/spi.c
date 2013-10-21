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

typedef enum SPI_STATE
{
	SPI_STATE_POWERUP = 0,
	SPI_STATE_INITALISED,
	SPI_STATE_FIRST_WRITE,
	SPI_STATE_IDLE,
	SPI_STATE_CMD_RXD,
	SPI_WAITING_TO_WRITE,
} SPI_STATE;

SPI_STATE SpiCurrentState = SPI_STATE_POWERUP;

uint8_t SpiRxBuffer[CC3000_RX_BUFFER_SIZE];
uint8_t SpiTxBuffer[CC3000_TX_BUFFER_SIZE];

uint16_t SpiWriteLen = 0;

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Spi_IrqEnable - Enable the IRQ interrupt
*/
// ------------------------------------------------------------------------------------------------------------
void Spi_IrqEnable ( void )
{
	// IRQ as Input
	LPC_GPIO->DIR[WLAN_IRQ_PORT] &= ~(WLAN_IRQ_PIN_MASK);

	// Clear any interrupts
	LPC_GPIO_PIN_INT->RISE = 0x1<<0;
	LPC_GPIO_PIN_INT->FALL = 0x1<<0;
	LPC_GPIO_PIN_INT->IST = 0x1<<0;

	// Enable IRQ GPIO Init High and Low
	GPIOSetPinInterrupt(0, WLAN_IRQ_PORT, WLAN_IRQ_PIN_NO, 0, 0);

	GPIOPinIntEnable(CHANNEL0, 0);

	// One below the lowest
	NVIC_SetPriority(PIN_INT0_IRQn, 6);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Spi_IrqDisable - Disable the IRQ interrupt
*/
// ------------------------------------------------------------------------------------------------------------
void Spi_IrqDisable ( void )
{
	GPIOPinIntDisable(CHANNEL0, 0);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief SpiResumeSpi - Required by TI Driver
*/
// ------------------------------------------------------------------------------------------------------------
void SpiResumeSpi(void)
{
	// Return to IDLE
	SpiCurrentState = SPI_STATE_IDLE;

	// Chip Select should still be asserted blocking any incoming events, so enable the interrupt
	Spi_IrqEnable();

	// De-select the WLAN module allowing it to continue
	SPI_CS_DEASSERT();
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief SpiOpen - Required by TI Driver
*/
// ------------------------------------------------------------------------------------------------------------
void SpiOpen (gcSpiHandleRx pfRxHandler)
{
	SpiCurrentState = SPI_STATE_POWERUP;

	// Zero the buffers
	memset(SpiRxBuffer, 0, sizeof(SpiRxBuffer));
	memset(SpiTxBuffer, 0, sizeof(SpiTxBuffer));

	SPI_HciRxFunc = pfRxHandler;

	SpiWriteLen = 0;
	sSpiInformation.pTxPacket = NULL;
	sSpiInformation.usRxPacketLength = 0;

	SpiRxBuffer[CC3000_RX_BUFFER_SIZE - 1] = CC3000_BUFFER_MAGIC_NUMBER;
	SpiTxBuffer[CC3000_TX_BUFFER_SIZE - 1] = CC3000_BUFFER_MAGIC_NUMBER;

	//
	// Enable interrupt on the GPIO pin of WLAN IRQ
	//
	Spi_IrqEnable();

	DelayUs(1000);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief SpiClose - Required by the TI Driver
*/
// ------------------------------------------------------------------------------------------------------------
void SpiClose(void)
{
	//	Disable IRQ Interrupt
	Spi_IrqDisable();
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief SpiWrite - Required by the TI Driver - Sets up a message to TX by the IRQ interrupt
*/
// ------------------------------------------------------------------------------------------------------------
long SpiWrite (unsigned char *pUserBuffer, unsigned short usLength)
{
    unsigned char ucPad = 0;

	while (( SPI_WAITING_TO_WRITE == SpiCurrentState ) || ( SPI_STATE_CMD_RXD == SpiCurrentState ))
	{
		// TI Stack is blocking, so lets block if we are in this state.
	}

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

    // Update the global for the interrupt
    SpiWriteLen = usLength + (SPI_HEADER_SIZE + ucPad);

    if ( ucPad )
    {
    	pUserBuffer[SpiWriteLen-1] = 0;
    }


	// Buffer overrun protection
	if (SpiTxBuffer[CC3000_TX_BUFFER_SIZE - 1] != CC3000_BUFFER_MAGIC_NUMBER)
	{
		// TODO : Can we do anything?
		while (1);
	}

	while ( SPI_STATE_POWERUP == SpiCurrentState )
	{
		// Wait!
		if (! WLAN_IRQ() )
		{
			//NVIC_SetPendingIRQ(PIN_INT0_IRQn);
			//NVIC->STIR = PIN_INT0_IRQn;
			SpiCurrentState = SPI_STATE_INITALISED;
			break;
		}
	}

    if ( SPI_STATE_INITALISED == SpiCurrentState )
    {
    	// First message is a little different, we wait for the IRQ before Chip selecting
		SPI_CS_ASSERT();

		// It also requires a slight delay, i hate fixed delays but this is easiest for now
		DelayUs(50);

		Spi_WriteBuffer( &SpiTxBuffer[0], 4 );

		// Special delay required when first communicating with the module
		DelayUs(50);

		// Then write the rest of the data
		Spi_WriteBuffer(&SpiTxBuffer[4], SpiWriteLen-4);

		SPI_CS_DEASSERT();
		SpiWriteLen = 0;

		// Go back to idle
		SpiCurrentState = SPI_STATE_IDLE;
    }
    else
    {
    	// TODO : Check did we get IRQ'd while setting up a tx message?
    	//Spi_IrqEnable();
    	Spi_IrqDisable();
    	SPI_CS_ASSERT();
    	SpiCurrentState = SPI_WAITING_TO_WRITE;

    	while ( WLAN_IRQ() )
    	{
    		// Waiting
    	}

    	if (! WLAN_IRQ() )
		{
    		// IRQ'd
    		Spi_IrqInterrupt();
		}
    	else
    	{
    		// Timed out
    	}
    	Spi_IrqEnable();
    }

    return(0);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Spi_WriteBuffer - Writes the supplied buffer to the bus, ignoring rx'd bytes during transmission
*/
// ------------------------------------------------------------------------------------------------------------
void Spi_WriteBuffer(uint8_t *data, uint16_t size)
{
volatile uint32_t temp = 0;

	if (( data ) && ( size ))
	{
		while (size)
		{
			// wait while the FIFO is full
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
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Spi_ReadIntoBuffer - Reads from the SPI in to the supplied buffer from the bus
*/
// ------------------------------------------------------------------------------------------------------------
void Spi_ReadIntoBuffer(uint8_t *data, uint16_t size)
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

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Spi_IrqInterrupt - fire on the IRQ line falling
*/
// ------------------------------------------------------------------------------------------------------------
void Spi_IrqInterrupt ( void )
{
uint8_t msgType = 0;
uint16_t length = 0;

	// Check this interrupt was caused IRQ interrupt, leave if the line is high
	if ( WLAN_IRQ() ) return;

    //SerialPort.printf("IRQ Interrupt\r\n");

    switch ( SpiCurrentState )
    {
    	case SPI_STATE_CMD_RXD :
    		// We should not interrupt while in this state, if we do it means a command hasnt been processed and some how the interrupt has been enabled
    	break;

        case SPI_STATE_IDLE :

            // Unsolicited message from the module
            // Tell module we want want to talk to it
        	SPI_CS_ASSERT();

            // Each message is a minimum of 10 bytes
        	Spi_ReadIntoBuffer(SpiRxBuffer, 10);

            // What type of message did we get?, first byte after the header
            msgType = SpiRxBuffer[SPI_HEADER_SIZE];

            switch ( msgType )
            {
                case HCI_TYPE_EVNT :

                    length = SpiRxBuffer[SPI_HEADER_SIZE + HCI_DATA_LENGTH_OFFSET];

                    length -= 1;

                    if ((HEADERS_SIZE_EVNT + length) & 1)
                    {
                        length ++;
                    }

                    if ( length )
                    {
                    	Spi_ReadIntoBuffer(&SpiRxBuffer[10], length);
                    }

                    SpiCurrentState = SPI_STATE_CMD_RXD;
                break;

                case HCI_TYPE_DATA :
                    length = (*(uint16_t *)(&SpiRxBuffer[SPI_HEADER_SIZE + HCI_DATA_LENGTH_OFFSET]));

                    // All requests must be even length
                    if (!((HEADERS_SIZE_EVNT + length) & 1))
                    {
                        length++;
                    }

                    // Is there anything to read in and will it fit
                    if (length)
					{
						if ( length < (CC3000_RX_BUFFER_SIZE-10))
						{
							// Read the rest in
							Spi_ReadIntoBuffer(&SpiRxBuffer[10], length);
						}
						else
						{
							// It wont fit!
						}
					}

                break;

                //default :
                    //printf("Unknown packet type received\r\n");
            }

            // We have a message to process, turn off irq interupt
            // Leave CS low to stop the module sending us another event while we process this one, eliminates the race condition
            Spi_IrqDisable();

            // Call TI Stack to process...
            if ( SPI_HciRxFunc )
            {
            	SPI_HciRxFunc( &SpiRxBuffer[SPI_HEADER_SIZE] );
            }
            else
            {
            	// Can't process it if we don't have anywhere to send it
            }

            // Stack must resume SPI by calling SpiResumeSpi();
        break;

        case SPI_WAITING_TO_WRITE :
            // We can tx the command we wanted to now
            // Chip select line will already be set
        	Spi_WriteBuffer(&SpiTxBuffer[0], SpiWriteLen);

            SpiWriteLen = 0;
            SPI_CS_DEASSERT();

            SpiCurrentState = SPI_STATE_IDLE;
        break;

        case SPI_STATE_FIRST_WRITE :

            // First message is a little different, we wait for the IRQ before Chip selecting
        	SPI_CS_ASSERT();

            // It also requires a slight delay, i hate fixed delays but this is easiest for now
        	DelayUs(50);

        	Spi_WriteBuffer( &SpiTxBuffer[0], 4 );

        	// Special delay required when first communicating with the module
        	DelayUs(50);

            // Then write the rest of the data
        	Spi_WriteBuffer(&SpiTxBuffer[4], SpiWriteLen-4);

            SPI_CS_DEASSERT();
            SpiWriteLen = 0;

            // Go back to idle
            SpiCurrentState = SPI_STATE_IDLE;
        break;

        case SPI_STATE_POWERUP :
        	SpiCurrentState = SPI_STATE_INITALISED;
        break;

        case SPI_STATE_INITALISED :
        	// Module is ready, we haven't had our first write as yet though
        break;
    }
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Spi_EnableModule - Controls the modules power/Enable line
*/
// ------------------------------------------------------------------------------------------------------------
void Spi_EnableModule ( bool enable )
{
    if ( enable )
    {
        // Enable the CC3000 Module
    	SPI_WLAN_EN();
    }
    else
    {
    	SPI_WLAN_DIS();
    }
}

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
	SPI_CS_DEASSERT();
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
	//LPC_SSP1->CR0 = 0 | 0x7 | SSPCR0_SPH | 0x0300;

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


