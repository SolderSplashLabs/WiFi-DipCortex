
#include "string.h"
#include "stdio.h"

#include "SolderSplashLpc.h"
#include "cc3000_headers.h"
#include "systemConfig.h"
#include "console.h"

#define _HTTP_
#include "http.h"

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief httpPut
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
	    	if(gethostbyname(hostname, strlen(hostname), (unsigned long*)&serverIp) > 0)
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
void httpPutJsonString (char *hostname, char *uri, uint32_t ip, char * jsonStr)
{
uint16_t httpCmdLen;
int32_t dataLen;
volatile long httpSocket = 0xFFFFFFFF;
uint32_t serverIp = 0;
uint32_t timeout = 100;
sockaddr tSocketAddr;
bool haveIp = false;


	// Assemble the HTTP Command into a buffer
	httpCmdLen = sprintf(&HttpCmdBuffer[0], "PUT %s HTTP/1.0\r\n", uri );
	httpCmdLen += sprintf(&HttpCmdBuffer[httpCmdLen], "Host: %s\r\nUser-Agent: WiFi-DipCortex\r\n", hostname );
	httpCmdLen += sprintf(&HttpCmdBuffer[httpCmdLen], "X-Physical-Secret: eae6285a2884d74b1fccdc43cd846856e0da299aa9003807db91057d65e1c68a\r\n");
	httpCmdLen += sprintf(&HttpCmdBuffer[httpCmdLen], "Content-Type: application/json\r\nContent-Length: %u\r\n", strlen(jsonStr));
	httpCmdLen += sprintf(&HttpCmdBuffer[httpCmdLen], "Connection: close\r\n\r\n");

	// add the data
	httpCmdLen += sprintf(&HttpCmdBuffer[httpCmdLen], "%s", jsonStr);

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
	    	if(gethostbyname(hostname, strlen(hostname), (unsigned long*)&serverIp) > 0)
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

		if(gethostbyname(HTTP_PROWL_HOSTNAME, strlen(HTTP_PROWL_HOSTNAME), (unsigned long*)&serverIp) > 0)
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
	//httpCmdLen += sprintf(&CosmCommandBuffer[httpCmdLen], "X-ApiKey: %s\r\n", SystemConfig.cosmPrivKey);
	httpCmdLen += sprintf(&HttpCmdBuffer[httpCmdLen], "Accept: text/html\r\n");
	httpCmdLen += sprintf(&HttpCmdBuffer[httpCmdLen], "Connection: close\r\n\r\n");


	httpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (httpSocket != 0xFFFFFFFF)
	{
		// We have a handle

	    tSocketAddr.sa_family = AF_INET;

	    tSocketAddr.sa_data[0] = (80 & 0xFF00) >> 8;
	    tSocketAddr.sa_data[1] = (80 & 0x00FF);


	    if(gethostbyname(hostname, strlen(hostname), (unsigned long*)&serverIp) > 0)
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
