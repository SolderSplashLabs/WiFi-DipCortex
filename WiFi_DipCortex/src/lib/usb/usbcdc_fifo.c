/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2013 - www.soldersplash.co.uk - C. Matthews - R. Steel


	@file     main.c
	@author   Carl Matthews (soldersplash.co.uk)
	@date     19 May 2013

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

*/

#include "SolderSplashLpc.h"
#include "usbcdc.h"

#define _USB_CDC_FIFO_
#include "usbcdc_fifo.h"

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief UsbCdcFifo_Task
*/
// ------------------------------------------------------------------------------------------------------------
void UsbCdcFifo_Task ( void )
{
	// if there's data in the fifo, try sending it
	if ( CdcFifoTXTail != CdcFifoTxHead )
	{
		UsbCdc_TriggerTx();
	}
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief UsbCdcFifo_Clear -
*/
// ------------------------------------------------------------------------------------------------------------
void UsbCdcTxFifo_Clear ( void )
{
	CdcFifoTxHead = 0;
	CdcFifoTXTail = 0;
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief UsbCdcTxFifo_CopyTo - Copy fifo data out and into a buffer
*/
// ------------------------------------------------------------------------------------------------------------
uint16_t UsbCdcTxFifo_CopyTo ( uint8_t *target, uint16_t len )
{
uint16_t result = 0;

	while (( CdcFifoTXTail != CdcFifoTxHead ) && ( result < len ))
	{
		target[result] = CdcFifoTx[CdcFifoTXTail];
		CdcFifoTXTail = (CdcFifoTXTail + 1) % CDC_FIFO_SIZE;

		result ++;
	}

	return ( result );
}


// ------------------------------------------------------------------------------------------------------------
/*!
    @brief UsbCdcFifo_GetByte -
*/
// ------------------------------------------------------------------------------------------------------------
bool UsbCdcTxFifo_GetChar ( char *data )
{
	if ( CdcFifoTXTail != CdcFifoTxHead )
	{
		*data = CdcFifoTx[CdcFifoTXTail];

		// Advance the tail and wrap it around
		CdcFifoTXTail = (CdcFifoTXTail + 1) % CDC_FIFO_SIZE;

		return ( true );
	}
	return ( false );
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief UsbCdcFifo_GetByte -
*/
// ------------------------------------------------------------------------------------------------------------
bool UsbCdcRxFifo_GetChar ( char *data )
{
	if ( CdcFifoRXTail != CdcFifoRxHead )
	{
		*data = CdcFifoRx[CdcFifoRXTail];

		// Advance the tail and wrap it around
		CdcFifoRXTail = (CdcFifoRXTail + 1) % CDC_FIFO_SIZE;

		return ( true );
	}
	return ( false );
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief UsbCdcRxFifo_GetString - get a buffer full up to the terminator char or the maxlen
*/
// ------------------------------------------------------------------------------------------------------------
uint16_t UsbCdcRxFifo_GetString ( uint8_t *data, char terminator, uint16_t maxLen  )
{
uint16_t result = 0;

	while (( CdcFifoTXTail != CdcFifoTxHead ) && ( result < maxLen ))
	{
		data[result] = CdcFifoTx[CdcFifoTXTail];
		CdcFifoTXTail = (CdcFifoTXTail + 1) % CDC_FIFO_SIZE;

		result ++;
		if ( terminator == data[result-1] ) break;
	}

	return ( result );
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief UsbCdcFifo_AddBytes - Add data to the fifo
    TODO : Handle the fifo filling up
*/
// ------------------------------------------------------------------------------------------------------------
uint16_t UsbCdcTxFifo_AddBytes ( char *data, uint16_t len )
{
uint16_t i = 0;
uint16_t result = 0;

	if (data != NULL)
	{
		// Loop around the supplied data and populate our cdc fifo
		for ( i=0; i<len; i++ )
		{
			// Grab the byte from wherever and put in FIFO
			CdcFifoTx[CdcFifoTxHead] = data[i];

			// Move the head forward and wrap around;
			CdcFifoTxHead = (CdcFifoTxHead + 1) % CDC_FIFO_SIZE;

			if (CdcFifoTxHead == CdcFifoTXTail)
			{
				// fifo is full, wait until we can add some more
				//while (CdcFifoTxHead == CdcFifoTXTail)
				{
					// TODO : ADD Timeout
				}
			}
			else
			{

				result = true;
			}
		}
	}

	return ( result );
}


// ------------------------------------------------------------------------------------------------------------
/*!
    @brief UsbCdcRxFifo_AddBytes - Add data to the fifo
    TODO : Handle the fifo filling up
*/
// ------------------------------------------------------------------------------------------------------------
uint16_t UsbCdcRxFifo_AddBytes ( char *data, uint16_t len )
{
	uint16_t i = 0;
	uint16_t result = 0;

	if (data != NULL)
	{
		// Loop around the supplied data and populate our cdc fifo
		for ( i=0; i<len; i++ )
		{
			// Grab the byte from wherever and put in FIFO
			CdcFifoRx[CdcFifoRxHead] = data[i];

			// Move the head forward and wrap around;
			CdcFifoRxHead = (CdcFifoRxHead + 1) % CDC_FIFO_SIZE;

			if (CdcFifoRxHead == CdcFifoRXTail)
			{
				// I canna take any more captain
				CdcFifoRxFull = TRUE;
			}
			else
			{
				CdcFifoRxFull = false;
				result = true;
			}
		}
	}

	return ( result );
}

