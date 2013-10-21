/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2013 - www.soldersplash.co.uk - C. Matthews - R. Steel


	@file     http.c
	@author   Carl Matthews (soldersplash.co.uk)
	@date     01 Oct 2013

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

	These are quick examples of using a TCP port to post or get data from an http server.
	They are not complete but offer a starting point
*/

#include "string.h"
#include "stdio.h"

#include "SolderSplashLpc.h"
#include "cc3000_headers.h"
#include "systemConfig.h"
#include "console.h"
#include "dnsCache.h"

#define _HTTP_
#include "http.h"

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief This example http puts data to a host in csv format, like Xively
*/
// ------------------------------------------------------------------------------------------------------------
void httpPut (char *hostname, char *uri, uint32_t ip)
{
uint16_t httpCmdLen;
int32_t dataLen;
char dataBuffer[50];
volatile long httpSocket = 0xFFFFFFFF;
uint32_t serverIp = 0;
uint32_t timeout = 100;
sockaddr tSocketAddr;
bool haveIp = false;

	// TODO : add assesor to fetch actual ADC data
	dataLen = sprintf(dataBuffer, "ADC0, 55\r\nADC1, 66\r\n");

	// Assemble the HTTP Command into a buffer
	httpCmdLen = sprintf(&HttpCmdBuffer[0], "PUT %s HTTP/1.0\r\n", uri );
	httpCmdLen += sprintf(&HttpCmdBuffer[httpCmdLen], "Host: %s\r\nUser-Agent: WiFi-DipCortex\r\n", hostname );
	if (SystemConfig.apiKey[0] != 0)
	{
		httpCmdLen += sprintf(&HttpCmdBuffer[httpCmdLen], "X-ApiKey: %s\r\n", SystemConfig.apiKey);
	}
	httpCmdLen += sprintf(&HttpCmdBuffer[httpCmdLen], "Content-Type: text/csv\r\nContent-Length: %u\r\n", dataLen);
	httpCmdLen += sprintf(&HttpCmdBuffer[httpCmdLen], "Connection: close\r\n\r\n");

	// add the data
	httpCmdLen += sprintf(&HttpCmdBuffer[httpCmdLen], "%s", dataBuffer);

	httpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (httpSocket != 0xFFFFFFFF)
	{
		// We have a handle

		if( setsockopt( httpSocket, SOL_SOCKET, SOCKOPT_RECV_TIMEOUT, &timeout, sizeof( timeout ) ) != 0)
		{
			ConsoleInsertPrintf("\r\nFailed to set socket options");
		}

	    tSocketAddr.sa_family = AF_INET;

	    tSocketAddr.sa_data[0] = (80 & 0xFF00) >> 8;
	    tSocketAddr.sa_data[1] = (80 & 0x00FF);

	    if ( ip )
	    {
	    	// use it
	    	tSocketAddr.sa_data[2] = BYTE_N(ip,3);
			tSocketAddr.sa_data[3] = BYTE_N(ip,2);
			tSocketAddr.sa_data[4] = BYTE_N(ip,1);
			tSocketAddr.sa_data[5] = BYTE_N(ip,0);
			haveIp = true;
	    }
	    else
	    {
	    	if(DnsCache_Query(hostname, strlen(hostname), (unsigned long*)&serverIp) > 0)
			{
				//correcting the endianess
				tSocketAddr.sa_data[5] = BYTE_N(serverIp,0);  // First octet of destination IP
				tSocketAddr.sa_data[4] = BYTE_N(serverIp,1);   // Second Octet of destination IP
				tSocketAddr.sa_data[3] = BYTE_N(serverIp,2);  // Third Octet of destination IP
				tSocketAddr.sa_data[2] = BYTE_N(serverIp,3);  // Fourth Octet of destination IP
				haveIp = true;
			}
	    	else
			{
				ConsoleInsertPrintf("\r\nFailed to resolve : %s", hostname);
			}
	    }

		if ( haveIp )
		{
	        if( 0 == connect(httpSocket, &tSocketAddr, sizeof(tSocketAddr)) )
	        {
	        	// COnnected
	        	send(httpSocket, HttpCmdBuffer, httpCmdLen,0);

	        	dataLen = recv(httpSocket,HttpCmdBuffer,500,0) ;

	        	if (-1 == dataLen)
	        	{

	        	}
	        	else
	        	{
	        		if ( dataLen > 0 )
	        		{
	        			HttpCmdBuffer[dataLen] = 0;
	        		    ConsoleInsertPrintf(HttpCmdBuffer);
	        		}
	        	}

	            closesocket(httpSocket);
	            httpSocket = 0xFFFFFFFF;
	        }
		}
	}
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief httpPut
*/
// ------------------------------------------------------------------------------------------------------------
void httpPostProwl (char *event, char *text)
{
uint16_t httpCmdLen;
int32_t dataLen;
volatile long httpSocket = 0xFFFFFFFF;
uint32_t serverIp = 0;
uint32_t timeout = 100;
sockaddr tSocketAddr;
bool haveIp = false;

	// TODO : event and text need html encoding, currently words with spaces wont work!
	// Assemble the HTTP Command into a buffer
	httpCmdLen = sprintf(&HttpCmdBuffer[0], "POST %s?apikey=%s&application=WifiDipCortex", HTTP_PROWL_URI, HTTP_PROWL_API_KEY );
	httpCmdLen += sprintf(&HttpCmdBuffer[httpCmdLen], "&event=%s&description=%s&priority=%d HTTP/1.0\r\n", event, text, 0);
	httpCmdLen += sprintf(&HttpCmdBuffer[httpCmdLen], "Host: %s\r\nUser-Agent: WiFi-DipCortex\r\n", HTTP_PROWL_HOSTNAME );
	httpCmdLen += sprintf(&HttpCmdBuffer[httpCmdLen], "Content-Type: application/text\r\nContent-Length: %u\r\n", 0);
	httpCmdLen += sprintf(&HttpCmdBuffer[httpCmdLen], "Connection: close\r\n\r\n");

	httpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	ConsoleInsertPrintf(HttpCmdBuffer);

	if (httpSocket != 0xFFFFFFFF)
	{
		// We have a handle

		if( setsockopt( httpSocket, SOL_SOCKET, SOCKOPT_RECV_TIMEOUT, &timeout, sizeof( timeout ) ) != 0)
		{
			ConsoleInsertPrintf("\r\nFailed to set socket options");
		}

	    tSocketAddr.sa_family = AF_INET;

	    tSocketAddr.sa_data[0] = (80 & 0xFF00) >> 8;
	    tSocketAddr.sa_data[1] = (80 & 0x00FF);

		if(DnsCache_Query(HTTP_PROWL_HOSTNAME, strlen(HTTP_PROWL_HOSTNAME), (unsigned long*)&serverIp) > 0)
		{
			//correcting the endianess
			tSocketAddr.sa_data[5] = BYTE_N(serverIp,0);  // First octet of destination IP
			tSocketAddr.sa_data[4] = BYTE_N(serverIp,1);   // Second Octet of destination IP
			tSocketAddr.sa_data[3] = BYTE_N(serverIp,2);  // Third Octet of destination IP
			tSocketAddr.sa_data[2] = BYTE_N(serverIp,3);  // Fourth Octet of destination IP
			haveIp = true;
		}
		else
		{
			ConsoleInsertPrintf("\r\nFailed to resolve : %s", HTTP_PROWL_HOSTNAME);
		}

		if ( haveIp )
		{
	        if( 0 == connect(httpSocket, &tSocketAddr, sizeof(tSocketAddr)) )
	        {
	        	// COnnected
	        	send(httpSocket, HttpCmdBuffer, httpCmdLen,0);

	        	dataLen = recv(httpSocket,HttpCmdBuffer,500,0) ;

	        	if (-1 == dataLen)
	        	{

	        	}
	        	else
	        	{
	        		if ( dataLen > 0 )
	        		{
	        			HttpCmdBuffer[dataLen] = 0;
	        		    ConsoleInsertPrintf(HttpCmdBuffer);
	        		}
	        	}

	            closesocket(httpSocket);
	            httpSocket = 0xFFFFFFFF;
	        }
		}
	}
}


// ------------------------------------------------------------------------------------------------------------
/*!
    @brief httpGet
*/
// ------------------------------------------------------------------------------------------------------------
void httpGet (char *hostname, char *uri)
{
uint16_t httpCmdLen;
int32_t dataLen;
char dataBuffer[50];
volatile long httpSocket = 0xFFFFFFFF;
uint32_t serverIp = 0;
sockaddr tSocketAddr;
uint32_t timeout = 500;

	dataLen = sprintf(dataBuffer, "Test\r\n");

	// Assemble the HTTP Command into a buffer
	httpCmdLen = sprintf(&HttpCmdBuffer[0], "GET %s HTTP/1.0\r\n", uri );
	httpCmdLen += sprintf(&HttpCmdBuffer[httpCmdLen], "Host: %s\r\nUser-Agent: WiFi-DipCortex\r\n", hostname );
	httpCmdLen += sprintf(&HttpCmdBuffer[httpCmdLen], "Accept: text/html\r\n");
	httpCmdLen += sprintf(&HttpCmdBuffer[httpCmdLen], "Connection: close\r\n\r\n");


	httpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (httpSocket != 0xFFFFFFFF)
	{
		// We have a handle

	    tSocketAddr.sa_family = AF_INET;

	    tSocketAddr.sa_data[0] = (80 & 0xFF00) >> 8;
	    tSocketAddr.sa_data[1] = (80 & 0x00FF);


	    if(DnsCache_Query(hostname, strlen(hostname), (unsigned long*)&serverIp) > 0)
	    {
	    	//correcting the endianess
	        tSocketAddr.sa_data[5] = BYTE_N(serverIp,0);  // First octet of destination IP
	        tSocketAddr.sa_data[4] = BYTE_N(serverIp,1);   // Second Octet of destination IP
	        tSocketAddr.sa_data[3] = BYTE_N(serverIp,2);  // Third Octet of destination IP
	        tSocketAddr.sa_data[2] = BYTE_N(serverIp,3);  // Fourth Octet of destination IP

	        if( 0 == connect(httpSocket, &tSocketAddr, sizeof(tSocketAddr)) )
	        {
	        	// COnnected

	        	setsockopt(httpSocket, SOL_SOCKET, SOCKOPT_RECV_TIMEOUT, &timeout, 1);
	        	send(httpSocket, HttpCmdBuffer, httpCmdLen,0);

	        	dataLen = recv(httpSocket,HttpCmdBuffer,500,0) ;

	        	//while ( dataLen )
	        	//{
	        		if (-1 == dataLen)
					{
	        			//break;
					}
					else
					{
						if ( dataLen > 0 )
						{
							HttpCmdBuffer[dataLen] = 0;
							ConsoleInsertPrintf(HttpCmdBuffer);
						}
					}
	        	//}
	        }

			closesocket(httpSocket);
			httpSocket = 0xFFFFFFFF;

	    }
	    else
	    {
	    	ConsoleInsertPrintf("\r\nFailed to resolve : %s", hostname);
	    }
	}
}

