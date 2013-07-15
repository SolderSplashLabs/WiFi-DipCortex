#include ".\cc3000\cc3000_common.h"
#include ".\cc3000\wlan.h"
#include ".\cc3000\evnt_handler.h"
#include ".\cc3000\nvmem.h"
#include ".\cc3000\socket.h"
#include ".\cc3000\netapp.h"
#include ".\cc3000\spi.h"

#include "SolderSplashLpc.h"

char CosmCommandBuffer[500];

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

	dataLen = sprintf(dataBuffer, "Test\r\n");

	// Assemble the HTTP Command into a buffer
	httpCmdLen = sprintf(&CosmCommandBuffer[0], "PUT %s HTTP/1.1\r\n", uri );
	httpCmdLen += sprintf(&CosmCommandBuffer[httpCmdLen], "Host: %s\r\nUser-Agent: SplashBase\r\n", hostname );
	//httpCmdLen += sprintf(&CosmCommandBuffer[httpCmdLen], "X-ApiKey: %s\r\n", SystemConfig.cosmPrivKey);
	httpCmdLen += sprintf(&CosmCommandBuffer[httpCmdLen], "Content-Type: text/csv\r\nContent-Length: %u\r\n", dataLen);
	httpCmdLen += sprintf(&CosmCommandBuffer[httpCmdLen], "Connection: close\r\n\r\n");

	// add the data
	httpCmdLen += sprintf(&CosmCommandBuffer[httpCmdLen], "%s", dataBuffer);

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
	    	if(gethostbyname(hostname, strlen(hostname), &serverIp) > 0)
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
	        	send(httpSocket, CosmCommandBuffer, httpCmdLen,0);

	        	dataLen = recv(httpSocket,CosmCommandBuffer,500,0) ;

	        	if (-1 == dataLen)
	        	{

	        	}
	        	else
	        	{
	        		if ( dataLen > 0 )
	        		{
	        			CosmCommandBuffer[dataLen] = 0;
	        		    ConsoleInsertPrintf(CosmCommandBuffer);
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
	httpCmdLen = sprintf(&CosmCommandBuffer[0], "GET %s HTTP/1.1\r\n", uri );
	httpCmdLen += sprintf(&CosmCommandBuffer[httpCmdLen], "Host: %s\r\nUser-Agent: SplashBase\r\n", hostname );
	//httpCmdLen += sprintf(&CosmCommandBuffer[httpCmdLen], "X-ApiKey: %s\r\n", SystemConfig.cosmPrivKey);
	httpCmdLen += sprintf(&CosmCommandBuffer[httpCmdLen], "Accept: text/html\r\n");
	httpCmdLen += sprintf(&CosmCommandBuffer[httpCmdLen], "Connection: close\r\n\r\n");


	httpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (httpSocket != 0xFFFFFFFF)
	{
		// We have a handle

	    tSocketAddr.sa_family = AF_INET;

	    tSocketAddr.sa_data[0] = (80 & 0xFF00) >> 8;
	    tSocketAddr.sa_data[1] = (80 & 0x00FF);


	    if(gethostbyname(hostname, strlen(hostname), &serverIp) > 0)
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
	        	send(httpSocket, CosmCommandBuffer, httpCmdLen,0);

	        	dataLen = recv(httpSocket,CosmCommandBuffer,500,0) ;


	        	if (-1 == dataLen)
	        	{

	        	}
	        	else
	        	{
	        		if ( dataLen > 0 )
	        		{
	        			CosmCommandBuffer[dataLen] = 0;
	        		    ConsoleInsertPrintf(CosmCommandBuffer);
	        		}
	        	}

	            closesocket(httpSocket);
	            httpSocket = 0xFFFFFFFF;
	        }
	    }
	    else
	    {
	    	ConsoleInsertPrintf("\r\nFailed to resolve : %s", hostname);
	    }
	}
}
