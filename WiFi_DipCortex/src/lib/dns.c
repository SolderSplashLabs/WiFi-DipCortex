/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2013 - www.soldersplash.co.uk - C. Matthews - R. Steel


	@file     dnsCache.c
	@author   Carl Matthews (soldersplash.co.uk)
	@date     03 July 2013

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

    Refer to http://tools.ietf.org/html/rfc1035 for DNS Protocol description

*/

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "cc3000_headers.h"

#include "SolderSplashLpc.h"
#include "dnsCache.h"

#define DNS_HEADER_SIZE					12
#define DNS_PORT_NO						53

// DNS Reply's can be up to 512 bytes long
uint8_t DnsDataBuffer[150];
unsigned long DnsSocket = 0xFFFFFFFF;
uint32_t DnsServerIp = 0;
uint32_t DnsListenStarted = 0;

// ------------------------------------------------------------------------------------------------------------
/*!
	@brief Dns_Task
*/
// ------------------------------------------------------------------------------------------------------------
void Dns_Task ( void )
{
volatile tNetappIpconfigRetArgs *cc3000Status;

	if (! DnsServerIp )
	{
		if ( Wifi_IsConnected() )
		{
			cc3000Status = getCC3000Info( true );
			DnsServerIp = *((uint32_t *)cc3000Status->aucDNSServer);
		}
	}
}

// ------------------------------------------------------------------------------------------------------------
/*!
	@brief Dns_ProcessServerReply
*/
// ------------------------------------------------------------------------------------------------------------
uint32_t Dns_ProcessServerReply ( int32_t replylength )
{
uint16_t answers;
uint16_t recordlen;
uint32_t IpAddress;
uint8_t i, x;

	// First 2 bytes are the Request ID
	// TODO : Check the request id is correct

	// Check error bits and response bit
	if (( DnsDataBuffer[2] & 1<<7 ) && (!( DnsDataBuffer[3] & 0x0f )))
	{
		// This is a response, no error

		// Question number
		//DnsDataBuffer[4]
		//DnsDataBuffer[5]

		// Number of answers
		answers = htons(*(uint16_t *)(&DnsDataBuffer[6]));

		// Authority RRs
		//DnsDataBuffer[8]
		//DnsDataBuffer[9]

		// Additional RRs
		//DnsDataBuffer[10]
		//DnsDataBuffer[11]

		// Next is the hostname we are trying to resolve, skipp over it and find the null
		i = 12;

		//Skip the Query Section
		while (i<replylength)
		{
			if (! DnsDataBuffer[i] )
			{
				// Found the null
				// Jump to the start of the answer section
				i = i +5;
				break;
			}
			i = i + 1;
		}

		// Loop around the answers looking for the first A Record
		for (x=0; x<answers; x++)
		{
			// Skip the compressed name bytes
			i = i + 2;

			// Get the Type
			if ( htons(*(uint16_t *)(&DnsDataBuffer[i])) == 0x0001 )
			{
				// It's an A record bingo!

				// Jump to the length, we are expecting 4!
				i = i + 8;
				recordlen = htons(*(uint16_t *)(&DnsDataBuffer[i]));

				i = i + 2;
				// Read the IP and leave
				IpAddress = htonl(*(uint32_t *)(&DnsDataBuffer[i]));
				return ( IpAddress );
			}
			else if ( htons(*(uint16_t *)(&DnsDataBuffer[i])) == 0x0005 )
			{
				// Ignore it

				// Jump to the length
				i = i + 8;
				recordlen = htons(*(uint16_t *)(&DnsDataBuffer[i]));
				i = i + 2;

				// Skip the CNAME
				i = i + recordlen;
			}

		}
	}
	else
	{

	}

	return ( 0 );
}

// ------------------------------------------------------------------------------------------------------------
/*!
	@brief Dns_AwaitReply
*/
// ------------------------------------------------------------------------------------------------------------
uint32_t Dns_AwaitReply ( void )
{
volatile signed long retVal = -1;
volatile sockaddr tSocketAddr;
socklen_t tRxPacketLength;

	DnsListenStarted = Time_Uptime();

	while (1)
	{
		retVal = recvfrom(DnsSocket, DnsDataBuffer, sizeof(DnsDataBuffer), 0, (sockaddr *)&tSocketAddr, &tRxPacketLength);

		if (retVal > 0)
		{
			// We have a message
			ConsoleInsertPrintf("DNS : Reply recv'd");

			closesocket(DnsSocket);
			DnsSocket = 0xffffffff;

			return( Dns_ProcessServerReply(retVal) );

			break;
		}
		else
		{
			// Nothing
			if ( Time_Uptime() > (DnsListenStarted + 4) )
			{
				ConsoleInsertPrintf("DNS : Timed out closing socket");
				closesocket(DnsSocket);
				DnsSocket = 0xffffffff;

				return ( 0 );
				break;
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------------------
/*!
	@brief Dns_GetHostByName
*/
// ------------------------------------------------------------------------------------------------------------
int32_t Dns_GetHostByName ( char * strHostname, uint16_t hostnameLen, unsigned long * ipAddress )
{
sockaddr tSocketAddr;
uint32_t bufferPos = 0;
int32_t result = -1;
long optvalue_block = 0;
int8_t i = 0;
uint8_t count = 0;

	if ( Wifi_IsConnected() )
	{
		// Get a socket id
		if ( 0xffffffff == DnsSocket )
		{
			DnsSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		}

		if ( DnsSocket != -1 )
		{
			// Any port
			tSocketAddr.sa_data[0] = 0;
			tSocketAddr.sa_data[1] = 0;

			// Any IP
			tSocketAddr.sa_data[2] = 0x00;
			tSocketAddr.sa_data[3] = 0x00;
			tSocketAddr.sa_data[4] = 0x00;
			tSocketAddr.sa_data[5] = 0x00;

			// Open the UDP Port
			if (! bind(DnsSocket, &tSocketAddr, sizeof(sockaddr)) )
			{
				// Set the socket to not block
				setsockopt(DnsSocket, SOL_SOCKET, SOCKOPT_RECV_NONBLOCK, &optvalue_block, 1);

				bufferPos = Time_Uptime();

				// Use the uptime and request id
				DnsDataBuffer[0] = bufferPos >> 8;
				DnsDataBuffer[1] = bufferPos;

				// Standard Query, message not truncated, query recursively non authenticated data unacceptable
				DnsDataBuffer[2] = 0x01;
				DnsDataBuffer[3] = 0x00;

				// No of questions, just the one
				DnsDataBuffer[4] = 0;
				DnsDataBuffer[5] = 1;

				// No of answers is always 0
				DnsDataBuffer[6] = 0;
				DnsDataBuffer[7] = 0;

				// Authority is always 0
				DnsDataBuffer[8] = 0;
				DnsDataBuffer[9] = 0;

				// Additional always 0
				DnsDataBuffer[10] = 0;
				DnsDataBuffer[11] = 0;

				// Next is the hostname we would like to resolve..
				// The hostname is split into sections by dots, each section is prefixed with the length
				// dots are not sent

				count = 0;
				// Note : i is signed
				for (i=hostnameLen-1; i>-1; i--)
				{
					if (strHostname[i] != '.')
					{
						DnsDataBuffer[13+i] = strHostname[i];
						count ++;
					}
					else
					{
						DnsDataBuffer[13+i] = count;
						count = 0;
					}
				}

				// first byte is the first length of first part of the hostname
				DnsDataBuffer[12] = count;

				bufferPos = 13 + hostnameLen;

				// Null Terminate the string
				DnsDataBuffer[bufferPos++] = 0;

				// A Record
				DnsDataBuffer[bufferPos++] = 0;
				DnsDataBuffer[bufferPos++] = 1;

				// Class INET
				DnsDataBuffer[bufferPos++] = 0;
				DnsDataBuffer[bufferPos++] = 1;

				tSocketAddr.sa_family = AF_INET;

				tSocketAddr.sa_data[0] = (DNS_PORT_NO >> 8);
				tSocketAddr.sa_data[1] = (0x00ff & DNS_PORT_NO);

				// the destination IP address
				tSocketAddr.sa_data[2] = BYTE_N(DnsServerIp,3);
				tSocketAddr.sa_data[3] = BYTE_N(DnsServerIp,2);
				tSocketAddr.sa_data[4] = BYTE_N(DnsServerIp,1);
				tSocketAddr.sa_data[5] = BYTE_N(DnsServerIp,0);

				if ( bufferPos != sendto ( DnsSocket, DnsDataBuffer , bufferPos , 0 , &tSocketAddr , sizeof(sockaddr) ) )
				{
					// failed to send
					result = -1;
				}
				else
				{
					*ipAddress = Dns_AwaitReply();
					result = 1;
				}
			}

		}
		else
		{
			DnsSocket = 0xffffffff;
		}
	}

	return ( result );
}


