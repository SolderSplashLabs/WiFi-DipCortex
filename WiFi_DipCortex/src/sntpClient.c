/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2012 - www.soldersplash.co.uk - C. Matthews - R. Steel

	@file     sntpClient.c
	@author   Carl Matthews (soldersplash.co.uk)
	@date     17 April 2013

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

	fetches the time from an internet server
	RFC5905 - Details the NTPv4 Protocol

*/

#include <string.h>
#include "SolderSplashLpc.h"
#include "cc3000_headers.h"
#include "systemConfig.h"

#include "wifi_app.h"
#include "timeManager.h"
#include "console.h"
#include "dnsCache.h"

#define _SNTP_
#include "sntpClient.h"

unsigned long SntpTimeStamp = 0;

unsigned long SntpRecvSocket = 0xFFFFFFFF;
uint8_t SNTP_Buffer[SNTP_MAX_DATA_LEN];
bool SNTP_Listening = false;

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief SntpInit
*/
// ------------------------------------------------------------------------------------------------------------
void SntpInit( void )
{
	SntpRecvSocket = 0xFFFFFFFF;
	SNTP_Listening = false;
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief SntpRecieve This function is called when we have data in the buffer to process
	RFC-1305 defines the format of the response packet
*/
// ------------------------------------------------------------------------------------------------------------
void SntpUpdateReceived ( void )
{
volatile unsigned long timestamp = 0;

	// Transmit Timestamp starts at byte 40
	// Quick and dirty method, take the transmit time of the packet and use that as our clock time
	// Note : This does not take into account, Internet delays or code delays at all!!
	timestamp = SNTP_Buffer[40];
	timestamp <<= 8;
	timestamp |= SNTP_Buffer[41];
	timestamp <<= 8;
	timestamp |= SNTP_Buffer[42];
	timestamp <<= 8;
	timestamp |= SNTP_Buffer[43];

	// Timestamp now contains the current no of seconds from 1900, lets convert it to the unix version of 1970
	timestamp -= UNIX_EPOCH;

	// TODO : This is a rubbish way of fixing the clock, must do better
	// instead of being always slow add a second on so that were always fast
	timestamp ++;
	Time_SetUnix(timestamp);

	SntpTimeStamp = timestamp;

	SNTP_Listening = false;
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief SntpTask - Normally idle unless waiting for a reply, if waiting it will poll for data
*/
// ------------------------------------------------------------------------------------------------------------
void SntpTask ( void )
{
volatile signed long retVal = -1;
volatile sockaddr tSocketAddr;
socklen_t tRxPacketLength;

	// if waiting for response
	if (( SNTP_Listening ) && (SntpRecvSocket != 0xffffffff ))
	{
		if ( Wifi_IsConnected() )
		{
			// TODO : this isn't accurate! but should be good enough for must purposes

			retVal = recvfrom(SntpRecvSocket, SNTP_Buffer, sizeof(SNTP_Buffer), 0, (sockaddr *)&tSocketAddr, &tRxPacketLength);

			if (retVal > 0)
			{
				// We have a message
				ConsoleInsertPrintf("SNTP : Reply recv'd");
				SntpUpdateReceived();
				closesocket(SntpRecvSocket);
				SntpRecvSocket = 0xffffffff;
			}
			else
			{
				// Nothing
				if ( Time_Uptime() > (SntpListenStarted + 4) )
				{
					ConsoleInsertPrintf("SNTP : Timed out closing socket");
					closesocket(SntpRecvSocket);
					SntpRecvSocket = 0xffffffff;
				}
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------------------
/*!
	@brief SntpBlockingRecv - a blocking receive with a timeout to get the NTP response asap

	This will still introduce an error based upon the response time over the internet
*/
// ------------------------------------------------------------------------------------------------------------
void SntpBlockingRecv ( void )
{
//timeval timeout;
//fd_set readsds;
sockaddr from;
socklen_t fromlen;
int32_t retVal = 0;
/*
	memset(&timeout, 0, sizeof(timeval));
	// A 5 second timeout
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	// Select only the active connections
	FD_ZERO(&readsds);
	FD_SET(SntpRecvSocket, &readsds);

	retVal = select(SntpRecvSocket + 1, &readsds, NULL, NULL, &timeout);

	if ((retVal != 0) && (retVal != -1))
	{
		if (FD_ISSET(SntpRecvSocket,&readsds))
		{
		*/
			retVal = recvfrom(SntpRecvSocket, SNTP_Buffer, SNTP_MAX_DATA_LEN, 0, &from, &fromlen);

			if (retVal != SNTP_MAX_DATA_LEN)
			{
				// Failed reply too small
				ConsoleInsertPrintf("SNTP : Failed reply too small");
			}
			else if ((SNTP_Buffer[0] & 0x7) != 4)
			{
				// Not a server response
				ConsoleInsertPrintf("SNTP : Failed corrupt response");
			}
			else
			{
				// Success
				SntpUpdateReceived();
			}
			/*
		}
	}
	*/

	// Were done with the socket, data should/will arrive in a single packet
	closesocket(SntpRecvSocket);
	SntpRecvSocket = 0xffffffff;
}

// ------------------------------------------------------------------------------------------------------------
/*!
	@brief SntpResolveAddress - Resolve NTP Hostname to an ip

*/
// ------------------------------------------------------------------------------------------------------------
uint32_t SntpResolveAddress ( void )
{
uint32_t serverIpAddr;
int retval = 0;

	retval = DnsCache_Query(SystemConfig.sntpServerAddress, strlen(SystemConfig.sntpServerAddress), (unsigned long *)&serverIpAddr);

	if ( retval < 0 )
	{
		return( 0 );
	}
	else
	{
		return( serverIpAddr );
	}
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief SntpUpdate - Open a listening socket and request the time from an NTP server

    @param blocking - wait for reply rather than polling for it
*/
// ------------------------------------------------------------------------------------------------------------
bool SntpUpdate ( bool blocking )
{
uint32_t serverIp = 0;
sockaddr tSocketAddr;
long optvalue_block = 0;
bool result = false;
int32_t retval = 0;

	if ( Wifi_IsConnected() )
	{
		SntpRecvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if ( SntpRecvSocket != -1 )
		{
			// the destination port
			tSocketAddr.sa_data[0] = (SNTP_PORT >> 8);
			tSocketAddr.sa_data[1] = (0x00ff & SNTP_PORT);

			// the IP address
			serverIp = SntpResolveAddress();

			if ( 0 == serverIp )
			{
				ConsoleInsertPrintf("SNTP : Failed to resolve server address");
			}
			else
			{
				tSocketAddr.sa_data[2] = 0x00;
				tSocketAddr.sa_data[3] = 0x00;
				tSocketAddr.sa_data[4] = 0x00;
				tSocketAddr.sa_data[5] = 0x00;

				// Bind to port to enable reception of data
				if (! bind(SntpRecvSocket, &tSocketAddr, sizeof(sockaddr)) )
				{
					if ( blocking )
					{

					}
					else
					{
						// Set the socket to not block
						setsockopt(SntpRecvSocket, SOL_SOCKET, SOCKOPT_RECV_NONBLOCK, &optvalue_block, 1);
					}

					// Socket is open and listening
					SNTP_Listening = true;

					// Zero the array
					memset(SNTP_Buffer, 0, SNTP_MAX_DATA_LEN);

					SNTP_Buffer[0] = SNTP_LI_NO_WARNING | SNTP_VERSION | SNTP_MODE_CLIENT;
					SNTP_Buffer[1] = 0;     // Stratum, or type of clock
					SNTP_Buffer[2] = 6;     // Polling Interval
					SNTP_Buffer[3] = 0xEC;  // Peer Clock Precision
					// 8 bytes of zero for Root Delay & Root Dispersion

					// Reference ID?
					SNTP_Buffer[12]  = 49;
					SNTP_Buffer[13]  = 0x4E;
					SNTP_Buffer[14]  = 49;
					SNTP_Buffer[15]  = 52;

					// Send the request.
					tSocketAddr.sa_data[0] = (SNTP_PORT >> 8);
					tSocketAddr.sa_data[1] = (0x00ff & SNTP_PORT);

					tSocketAddr.sa_data[2] = (char)((serverIp>>24) & 0xff);
					tSocketAddr.sa_data[3] = (char)((serverIp>>16) & 0xff);
					tSocketAddr.sa_data[4] = (char)((serverIp>>8) & 0xff);
					tSocketAddr.sa_data[5] = (char)(serverIp & 0xff);
					retval = sendto ( SntpRecvSocket, SNTP_Buffer , SNTP_MAX_DATA_LEN, 0 , &tSocketAddr , sizeof(sockaddr) );

					if ( retval != SNTP_MAX_DATA_LEN )
					{
						// Failed!
						ConsoleInsertPrintf("SNTP : Failed to send NTP Command");
					}
					else
					{
						if ( blocking )
						{
							SntpBlockingRecv();
						}
						else
						{
							// Use the task to poll for a response
							ConsoleInsertPrintf("SNTP : Waiting for reply");
							SntpListenStarted = Time_Uptime();
							result = true;
						}
					}
				}
				else
				{
					SntpRecvSocket = 0xFFFFFFFF;
					SNTP_Listening = false;
					ConsoleInsertPrintf("SNTP : Failed to bind to RX Port");
				}
			}
		}
	}

	return( result );
}

