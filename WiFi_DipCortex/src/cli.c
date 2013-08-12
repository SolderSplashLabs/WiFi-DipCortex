/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2013 - www.soldersplash.co.uk - C. Matthews - R. Steel


	@file     SolderSplashLpc.c
	@author   Carl Matthews (soldersplash.co.uk)
	@date     01 May 2013

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

#include <string.h>
#include <stdio.h>
#include <time.h>

#include "SolderSplashLpc.h"
#include "systemConfig.h"
#include "timeManager.h"
#include "console.h"

#include "cc3000_headers.h"

#include "SolderSplashUdp.h"
#include "http.h"
#include "inet.h"
#include "sntpClient.h"
#include "wifi_app.h"

#define _CLI_
#include "cli.h"
// ------------------------------------------------------------------------------------------------------------
/*!
    @brief CLI_Init -
*/
// ------------------------------------------------------------------------------------------------------------
void CLI_Init ( void )
{
	Console_Init( ( CONSOLE_CMDS_STRUCT *)&ConsoleCommands );
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Cli_LogHciEvent - Debug accessor used to show the TI stack status
*/
// ------------------------------------------------------------------------------------------------------------
void Cli_LogHciEvent ( uint16_t hciNo )
{
	if (( hciNo > HCI_CMND_SOCKET_BASE) && ( hciNo <= HCI_CMND_MDNS_ADVERTISE))
	{
		ConsoleInsertDebugPrintf("HCI Event Received : 0x%04X - %s", hciNo, HCI_EVENT_STR[hciNo-HCI_CMND_SOCKET]);
	}
	else if ((hciNo > HCI_CMND_NETAPP_BASE) && ( hciNo <= HCI_NETAPP_SET_TIMERS))
	{
		ConsoleInsertDebugPrintf("HCI Event Received : 0x%04X - %s", hciNo, HCI_NETAPP_STR[hciNo-HCI_NETAPP_DHCP]);
	}
	else if (hciNo < HCI_CMND_WLAN_CONFIGURE_PATCH+1)
	{
		ConsoleInsertDebugPrintf("HCI Event Received : 0x%04X - %s", hciNo, HCI_MISC_STR[hciNo]);
	}
	else
	{
		ConsoleInsertDebugPrintf("HCI Event Received : 0x%04X", hciNo);
	}
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief CLI_Ipconfig
*/
// ------------------------------------------------------------------------------------------------------------
int CLI_Ipconfig (int argc, char **argv)
{
volatile tNetappIpconfigRetArgs *cc3000Status;
uint8_t *tmpPtr;
uint8_t *ip;

	ConsolePrintf("\r\n");

	if ( argc > 4 )
	{
		SystemConfig.ulStaticIP = inet_addr(&argv[1][0]);
		ip = (uint8_t *)&SystemConfig.ulStaticIP;
		ConsoleInsertPrintf("\r\nIP : (%d.%d.%d.%d)", ip[0], ip[1], ip[2], ip[3] );

		SystemConfig.ulGatewayIP = inet_addr(&argv[2][0]);
		ip = (uint8_t *)&SystemConfig.ulGatewayIP;
		ConsoleInsertPrintf("\r\nGateway : (%d.%d.%d.%d)", ip[0], ip[1], ip[2], ip[3] );

		SystemConfig.ulSubnetMask = inet_addr(&argv[3][0]);
		ip = (uint8_t *)&SystemConfig.ulSubnetMask;
		ConsoleInsertPrintf("\r\nSubnet : (%d.%d.%d.%d)", ip[0], ip[1], ip[2], ip[3] );

		SystemConfig.ulDnsServer = inet_addr(&argv[4][0]);
		ip = (uint8_t *)&SystemConfig.ulDnsServer;
		ConsoleInsertPrintf("\r\nDNS : (%d.%d.%d.%d)", ip[0], ip[1], ip[2], ip[3] );

		SystemConfig.flags.StaticIp = 1;
		wlan_stop();
		Wifi_AppInit(0);
		//result = 1;
	}
	else
	{
		cc3000Status = getCC3000Info( true );

		if ( Wifi_IsConnected() )
		{
			ConsoleInsertPrintf("Connected to: %s", (char *)&cc3000Status->uaSSID[0]);
		}
		else
		{
			ConsoleInsertPrintf("Not Connected");
		}

		tmpPtr = (uint8_t *)&cc3000Status->uaMacAddr;
		ConsoleInsertPrintf("MAC : %02X-%02X-%02X-%02X-%02X-%02X" , tmpPtr[5], tmpPtr[4], tmpPtr[3], tmpPtr[2], tmpPtr[1], tmpPtr[0]);

		tmpPtr = (uint8_t *)&cc3000Status->aucIP;
		ConsoleInsertPrintf("IP : %d.%d.%d.%d", tmpPtr[3], tmpPtr[2], tmpPtr[1], tmpPtr[0] );

		tmpPtr = (uint8_t *)&cc3000Status->aucDefaultGateway;
		ConsoleInsertPrintf("Gateway : %d.%d.%d.%d", tmpPtr[3], tmpPtr[2], tmpPtr[1], tmpPtr[0] );

		tmpPtr = (uint8_t *)&cc3000Status->aucDNSServer;
		ConsoleInsertPrintf("DNS : %d.%d.%d.%d", tmpPtr[3], tmpPtr[2], tmpPtr[1], tmpPtr[0] );

		tmpPtr = (uint8_t *)&cc3000Status->aucSubnetMask;
		ConsoleInsertPrintf("Subnet : %d.%d.%d.%d", tmpPtr[3], tmpPtr[2], tmpPtr[1], tmpPtr[0] );
	}
	return(1);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief CLI_Help
*/
// ------------------------------------------------------------------------------------------------------------
int CLI_Help (int argc, char **argv)
{
uint8_t i = 0;

	ConsolePrintf("\r\n");

	i = 0;
	while (ConsoleCommands[i].Command)
	{
		ConsolePrintf("%15s \t: %s\r\n", ConsoleCommands[i].Command, ConsoleCommands[i].help);
		i++;
	}

	return(1);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief CLI_WlanConnect
*/
// ------------------------------------------------------------------------------------------------------------
int CLI_WlanConnect (int argc, char **argv)
{
uint8_t result = 0;

	if ( argc > 3 )
	{
		// ssid, encyption enum, passkey
		// encrypted
		wlan_connect(atoi(argv[2]), argv[1], strlen(argv[1]), NULL, (unsigned char *)argv[3], strlen(argv[3]));
		ConsoleInsertPrintf("\r\nConnecting to : %s with key : %s\r\n", argv[1], argv[3]);
		result = 1;
	}
	else if ( argc > 1 )
	{
		// No encryption
		wlan_connect(WLAN_SEC_WPA2, argv[1], strlen(argv[1]), NULL, NULL, 0);
		ConsoleInsertPrintf("\r\nConnecting to : %s\r\n", argv[1]);

		result = 1;
	}
	else
	{
		wlan_stop();
		Wifi_AppInit(0);
		result = 1;
	}

	return(result);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief CLI_WlanDisconnect
*/
// ------------------------------------------------------------------------------------------------------------
int CLI_WlanDisconnect (int argc, char **argv)
{
	wlan_ioctl_set_connection_policy(0, 0, 0);
	wlan_disconnect();
	return(1);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief CLI_WlanStatus
*/
// ------------------------------------------------------------------------------------------------------------
int CLI_WlanStatus (int argc, char **argv)
{
uint8_t buffer[2];

	ConsolePrintf("\r\n");
	ConsoleInsertPrintf("\r\nAsking for current Wifi status");
	ConsoleInsertPrintf("Wifi Status Returned : %d", wlan_ioctl_statusget());

	if (! nvmem_read_sp_version( (unsigned char*)&buffer ) )
	{
		ConsoleInsertPrintf("Version : %u.%u", buffer[0], buffer[1]);
	}
	else
	{
		ConsoleInsertPrintf("Read nvmem failed!");
	}

	return(1);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief CLI_Ipconfig
*/
// ------------------------------------------------------------------------------------------------------------
int CLI_Ping (int argc, char **argv)
{
uint8_t *ip;
volatile unsigned long serverIpAddr = 0;

	ConsolePrintf("\r\n");
	if ( Wifi_IsConnected() )
	{
		if ( argc > 2 )
		{
			serverIpAddr = inet_addr(&argv[2][0]);
			ip = (uint8_t *)&serverIpAddr;
			ConsoleInsertPrintf("\r\nPinging : (%d.%d.%d.%d)", ip[0], ip[1], ip[2], ip[3] );

			Wifi_SendPing( serverIpAddr, 3, 32, 500 );
		}
		else
		{
			// Resolve the IP address
			if ( gethostbyname(argv[1], strlen(argv[1]), (unsigned long *)&serverIpAddr) )
			{
				ip = (uint8_t *)&serverIpAddr;
				ConsoleInsertPrintf("\r\nPinging : %s (%d.%d.%d.%d)", argv[1], ip[3], ip[2], ip[1], ip[0] );

				// Ping with suggested defaults
				serverIpAddr = htonl(serverIpAddr);
				Wifi_SendPing( serverIpAddr, 3, 32, 500 );
			}
			else
			{
				ConsoleInsertPrintf("\r\nCan not resolve IP address");
			}
		}
	}
	else
	{
		ConsoleInsertPrintf("\r\nWiFi not connected");
	}

	return(1);
}

volatile long ulSocket;

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief CLI_Udp
*/
// ------------------------------------------------------------------------------------------------------------
int CLI_Udp (int argc, char **argv)
{
uint8_t result = 0;

	ConsolePrintf("\r\n");
	if ( Wifi_IsConnected() )
	{
		if ( argc > 1 )
		{
			if ( 'c' == argv[1][0])
			{
				SSUDP_Close();
				ConsolePrintf("\r\nUDP Closed");
				result = 1;
			}
			else if ( 'l' == argv[1][0])
			{
				ConsolePrintf("\r\nUDP Listening");
				SSUDP_Listen();
				result = 1;
			}
		}
	}
	else
	{
		ConsoleInsertPrintf("\r\nWiFi not connected");
		result = 1;
	}

	return(result);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief CLI_CC3000_ReadParams
*/
// ------------------------------------------------------------------------------------------------------------
int CLI_CC3000_ReadParams (int argc, char **argv)
{
	ConsolePrintf("\r\n");
	if ( ReadParameters() )
	{
		ConsoleInsertPrintf("Successfully read parameters\r\n");
	}
	else
	{
		ConsoleInsertPrintf("Failed to read parameters");
	}

	return(1);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief CLI_CC3000_IoCtl
*/
// ------------------------------------------------------------------------------------------------------------
int CLI_CC3000_IoCtl (int argc, char **argv)
{
uint8_t tempByte = 0xff;
uint8_t result = 0;
int32_t tmpLong = 0;

	if ( argc > 4 )
	{
		if (( 'p' == argv[1][0] ) && ( 'y' == argv[1][5] ))
		{
			// policy
			//wlan_ioctl_set_connection_policy(DISABLE, DISABLE, DISABLE);
		}
	}
	else if ( argc > 2 )
	{
		ConsolePrintf("\r\n");
		if (( 'i' == argv[1][0] ) && ( 't' == argv[1][3] ))
		{
			// init 0/1
			tempByte = atoi(argv[2]);
			ConsoleInsertPrintf("Re-init : %u", tempByte);
			if ( 1 == tempByte )
			{
				ConsoleInsertPrintf("Re-init : 1");
				wlan_stop();
				Wifi_AppInit(tempByte);
				result = 1;
			}
			else if ( 0 == tempByte )
			{
				ConsoleInsertPrintf("Re-init : 0");
				wlan_stop();
				Wifi_AppInit(tempByte);
				result = 1;
			}

		}
		else  if (( 'm' == argv[1][0] ) && ( 'd' == argv[1][1] ))
		{
			// mdns
			// init 0/1
			tempByte = atoi(argv[2]);

			if ( 1 == tempByte )
			{
				ConsolePrintf("Enabling mDNS");
				mdnsAdvertiser(1,SystemConfig.deviceName,strlen(SystemConfig.deviceName));
				result = 1;
			}
			else if ( 0 == tempByte )
			{
				ConsolePrintf("Disabling mDNS");
				mdnsAdvertiser(0,SystemConfig.deviceName,strlen(SystemConfig.deviceName));
				result = 1;
			}
		}
		else  if (( 's' == argv[1][0] ) && ( 's' == argv[1][5] ))
		{
			tmpLong = atoi(argv[2]);

			Wifi_StartScan(tmpLong);
			result = 1;
		}
	}
	else if ( argc > 1 )
	{
		ConsolePrintf("\r\n");
		if (( 'd' == argv[1][0] ) && ( 'e' == argv[1][1] ))
		{
			// delete
			ConsoleInsertPrintf("Deleting all profiles");
			wlan_ioctl_del_profile(255);
			result = 1;
		}
		else if (( 's' == argv[1][0] ) && ( 'c' == argv[1][1] ))
		{
			// scan
			ConsoleInsertPrintf("Retrieving scan results");
			Wifi_GetScanResults();
			result = 1;
		}
		else if (( 's' == argv[1][0] ) && ( 'm' == argv[1][1] ))
		{
			// smart
			ConsoleInsertPrintf("Starting smart config");
			StartSmartConfig();
			result = 1;
		}
	}

	//inet_addr

	return(result);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief CLI_CC3000_Patch
*/
// ------------------------------------------------------------------------------------------------------------
int CLI_CC3000_Patch (int argc, char **argv)
{
	//CC3000_ApplyPatch();
	return(1);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief CLI_Resolve - Resolve host to ip
*/
// ------------------------------------------------------------------------------------------------------------
int CLI_Resolve (int argc, char **argv)
{
unsigned long serverIpAddr;
uint8_t result = 0;
uint8_t *ip;

	ConsolePrintf("\r\n");
	if ( Wifi_IsConnected() )
	{
		if ( argc > 1 )
		{
			ConsoleInsertPrintf("DNS Lookup : %s ... ", argv[1]);
			if ( gethostbyname(argv[1], strlen(argv[1]), &serverIpAddr) )
			{
				ip = (uint8_t *)&serverIpAddr;
				ConsoleInsertPrintf("Resolved : %s to : %d.%d.%d.%d", argv[1], ip[3], ip[2], ip[1], ip[0] );
			}
			result = 1;
		}
	}
	else
	{
		ConsoleInsertPrintf("\r\nWiFi not connected");
		result = 1;
	}
	return(result);
}

const char LelylanOnStr[] = "{ \"properties\": [{ \"id\": \"<status>\", \"value\": \"on\", \"pending\": false }] }";
const char LelylanOffStr[] = "{ \"properties\": [{ \"id\": \"<status>\", \"value\": \"off\", \"pending\": false }] }";

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief CLI_Http
*/
// ------------------------------------------------------------------------------------------------------------
int CLI_Http (int argc, char **argv)
{
uint8_t result = 0;

	ConsolePrintf("\r\n");
	if ( argc > 2 )
	{
		if (( 'p' == argv[1][0] ) && ( 'l' == argv[1][3] ))
		{
			httpPutJsonString(argv[2], argv[3], 0, (char *)&LelylanOnStr[0]);
			result = 1;
		}
		else if (( 'p' == argv[1][0] ) && ( 'u' == argv[1][1] ))
		{
			// put
			ConsolePrintf("\r\n");
			httpPut(argv[2], argv[3], 0);
			result = 1;
		}
		else if (( 'g' == argv[1][0] ) && ( 'e' == argv[1][1] ))
		{
			// get
			ConsolePrintf("\r\n");
			httpGet(argv[2], argv[3]);
			result = 1;
		}
		else if (( 'a' == argv[1][0] ) && ( 'p' == argv[1][1] ))
		{
			strncpy( SystemConfig.apiKey, argv[2], sizeof(SystemConfig.apiKey));
			ConsoleInsertPrintf("API Key Set");
			result = 1;
		}

		if ( argc > 3 )
		{
			if (( 'p' == argv[1][0] ) && ( 'r' == argv[1][1] ))
			{
				ConsoleInsertPrintf("Posting to prowl");
				httpPostProwl(argv[2], argv[3]);
				result = 1;
			}
		}
	}

	return(result);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief CLI_9Dof
*/
// ------------------------------------------------------------------------------------------------------------
int CLI_9Dof (int argc, char **argv)
{
uint8_t result = 1;

	//LSM330_Task();
	return(result);
}


// ------------------------------------------------------------------------------------------------------------
/*!
    @brief CLI_Time
*/
// ------------------------------------------------------------------------------------------------------------
int CLI_Time (int argc, char **argv)
{
struct tm *currentTime;
uint32_t tmpTime = 0;
uint16_t days = 0;
uint8_t hours = 0;
uint8_t minutes = 0;

	if ( argc == 2 )
	{
		//update
		if (( 'u' == argv[1][0] ) && ( 'd' == argv[1][2] ))
		{
			SntpUpdate( false );
		}
		//uptime
		else if (( 'u' == argv[1][0] ) && ( 'p' == argv[1][1] ))
		{
			tmpTime = Time_Uptime();

			days = tmpTime / SECONDS_IN_AN_DAY;
			tmpTime -= days * SECONDS_IN_AN_DAY;

			hours = tmpTime / SECONDS_IN_AN_HOUR;
			tmpTime -= hours * SECONDS_IN_AN_HOUR;

			minutes = tmpTime / SECONDS_IN_AN_MIN;
			tmpTime -= minutes * SECONDS_IN_AN_MIN;
			ConsolePrintf("\r\n");
			ConsoleInsertPrintf("%d day/s %02d:%02d:%02d", days, hours, minutes, tmpTime);
		}
	}
	else if ( argc == 1 )
	{
		ConsolePrintf("\r\n");
		tmpTime = Time_StampNow(SystemConfig.timeOffset);
		currentTime = localtime(&tmpTime);
		ConsoleInsertPrintf("%s - Offset : %d minutes", asctime (currentTime), SystemConfig.timeOffset);
		//ConsoleInsertPrintf("%02d-%02d-%02d %02d:%02d:%02d - Offset : %d minutes\n", currentTime->tm_year, (currentTime->tm_mon+1), currentTime->tm_mday, currentTime->tm_hour, currentTime->tm_min, currentTime->tm_sec, SystemConfig.timeOffset);
	}

	return(1);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief CLI_Time
*/
// ------------------------------------------------------------------------------------------------------------
int CLI_Gpio (int argc, char **argv)
{
	ConsolePrintf("\r\n");
	ConsoleInsertPrintf("Port 0 : 0x%08x\r\nPort 1 : 0x%08x", LPC_GPIO->PIN[0], LPC_GPIO->PIN[1] );

	return(1);
}
