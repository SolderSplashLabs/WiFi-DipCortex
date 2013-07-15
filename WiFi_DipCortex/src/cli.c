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

#include "SolderSplashLpc.h"
#include "console.h"

#include ".\cc3000\wlan.h"
#include ".\cc3000\evnt_handler.h"
#include ".\cc3000\nvmem.h"
#include ".\cc3000\socket.h"
#include ".\cc3000\netapp.h"
#include ".\cc3000\spi.h"

#include "SolderSplashUdp.h"

#define _CLI_
#include "cli.h"

char device_name[] = "WifiDip-Cortex";

//*****************************************************************************
//
//!  \brief   Prints an IP Address to UART, Little Endian Format
//!
//!  \param  ip is a pointer to a 4 byte array with IP octets
//!
//!  \return none
//!
//
//*****************************************************************************
void printIpAddr(char * ip)
{
    char str[20];
    memset(str,0,sizeof(str));
    itoa(ip[3],str,10);
    // Send First octet
    ConsolePrintf(str);

    ConsolePrintf(".");
    memset(str,0,sizeof(str));
    itoa(ip[2],str,10);
    // Send Second octet
    ConsolePrintf(str);
    ConsolePrintf(".");
    memset(str,0,sizeof(str));
    itoa(ip[1],str,10);
    // Send Third octet
    ConsolePrintf(str);
    ConsolePrintf(".");
    memset(str,0,sizeof(str));
    itoa(ip[0],str,10);
    // Send Fourth octet
    ConsolePrintf(str);
}

//*****************************************************************************
//
//!  \brief   Prints the MAC Address to UART, Little Endian Format
//!
//!  \param  mac is a pointer to a 6 byte array with the MAC address
//!
//!  \return none
//!
//
//*****************************************************************************
void printMACAddr(char * mac)
{
    char str[25];
    memset(str,0,sizeof(str));

    itoa(mac[5],str,16);
    // Send First octet
    ConsolePrintf(str);

    ConsolePrintf(":");
    memset(str,0,sizeof(str));
    itoa(mac[4],str,16);
    // Send Second octet
    ConsolePrintf(str);
    ConsolePrintf(":");
    memset(str,0,sizeof(str));
    itoa(mac[3],str,16);
    // Send Third octet
    ConsolePrintf(str);
    ConsolePrintf(":");
    memset(str,0,sizeof(str));
    itoa(mac[2],str,16);
    // Send Fourth octet
    ConsolePrintf(str);
    ConsolePrintf(":");
    memset(str,0,sizeof(str));
    itoa(mac[1],str,16);
    // Send Fourth octet
    ConsolePrintf(str);
    ConsolePrintf(":");
    memset(str,0,sizeof(str));
    itoa(mac[0],str,16);
    // Send Fourth octet
    ConsolePrintf(str);
}


// ------------------------------------------------------------------------------------------------------------
/*!
    @brief CLI_Init -
*/
// ------------------------------------------------------------------------------------------------------------
void CLI_Init ( void )
{
	Console_Init( &ConsoleCommands );
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
		ConsoleInsertPrintf("HCI Event Received : 0x%04X - %s", hciNo, HCI_EVENT_STR[hciNo-HCI_CMND_SOCKET]);
	}
	else if ((hciNo > HCI_CMND_NETAPP_BASE) && ( hciNo <= HCI_NETAPP_SET_TIMERS))
	{
		ConsoleInsertPrintf("HCI Event Received : 0x%04X - %s", hciNo, HCI_NETAPP_STR[hciNo-HCI_NETAPP_DHCP]);
	}
	else
	{
		ConsoleInsertPrintf("HCI Event Received : 0x%04X", hciNo);
	}

}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief CLI_Ipconfig
*/
// ------------------------------------------------------------------------------------------------------------
int CLI_Ipconfig (int argc, char **argv)
{
tNetappIpconfigRetArgs *cc3000Status;

	ConsolePrintf("\r\n");
	cc3000Status = getCC3000Info();

	ConsolePrintf("Connected to: ");
	ConsolePrintf((char *)cc3000Status->uaSSID);
	ConsolePrintf("\r\n");

	ConsolePrintf("MAC : ");
	printMACAddr((char *)cc3000Status->uaMacAddr);
	ConsolePrintf("\r\n");

	ConsolePrintf("IP : ");
	printIpAddr((char *)cc3000Status->aucIP);
	ConsolePrintf("\r\n");

	ConsolePrintf("Gateway : ");
	printIpAddr((char *)cc3000Status->aucDefaultGateway);
	ConsolePrintf("\r\n");

	ConsolePrintf("DNS : ");
	printIpAddr((char *)cc3000Status->aucDNSServer);
	ConsolePrintf("\r\n");

	ConsolePrintf("Subnet : ");
	printIpAddr((char *)cc3000Status->aucSubnetMask);


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
		wlan_connect(atoi(argv[2]), argv[1], strlen(argv[1]), NULL, argv[3], strlen(argv[3]));
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

	ConsoleInsertPrintf("\r\nAsking for current Wifi status");
	ConsoleInsertPrintf("Wifi Status Returned : %d", wlan_ioctl_statusget());

	if (! nvmem_read_sp_version( &buffer ) )
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
	//netapp_ping_send(unsigned long *ip, unsigned long ulPingAttempts, unsigned long ulPingSize, unsigned long ulPingTimeout);

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

	return(result);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief CLI_CC3000_ReadParams
*/
// ------------------------------------------------------------------------------------------------------------
int CLI_CC3000_ReadParams (int argc, char **argv)
{
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
unsigned char scanResults[50];

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
				mdnsAdvertiser(1,device_name,strlen(device_name));
				result = 1;
			}
			else if ( 0 == tempByte )
			{
				ConsolePrintf("Disabling mDNS");
				mdnsAdvertiser(0,device_name,strlen(device_name));
				result = 1;
			}
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
			//wlan_ioctl_set_scan_params
			ConsoleInsertPrintf("Retrieving scan results");
			wlan_ioctl_get_scan_results(0, scanResults);
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
volatile unsigned long serverIpAddr;
uint8_t result = 0;

	if ( argc > 1 )
	{
		ConsolePrintf("\r\nDNS Lookup : %s ... ", argv[1]);
		if ( gethostbyname(argv[1], strlen(argv[1]), &serverIpAddr) )
		{
			ConsolePrintf("\r\nResolved : %s to : ", argv[1]);
			printIpAddr(&serverIpAddr);
		}
		result = 1;
	}
	return(result);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief CLI_Http
*/
// ------------------------------------------------------------------------------------------------------------
int CLI_Http (int argc, char **argv)
{
uint8_t result = 0;

	if ( argc > 2 )
	{
		if (( 'p' == argv[1][0] ) && ( 'u' == argv[1][1] ))
		{
			ConsolePrintf("\r\n");
			httpPut(argv[2], argv[3]);
			result = 1;
		}
		else if (( 'g' == argv[1][0] ) && ( 'e' == argv[1][1] ))
		{
			ConsolePrintf("\r\n");
			httpGet(argv[2], argv[3]);
			result = 1;
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

