/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2012 - www.soldersplash.co.uk - C. Matthews - R. Steel

	@file     systemConfig.c
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

	A polling UDP Server example, listens for UDP messages and replies back to them. In the this example the messages can be broadcast
	this lets us locate devices on a network.

	Using a similar protocol to the SplashBase, which has Android and PC's applications

*/


#include "string.h"

#include "SolderSplashLpc.h"

#include "cc3000_headers.h"
#include "lpc\pwmControl.h"

#include "wifi_app.h"
#include "systemConfig.h"
#include "console.h"

#include "udpServer.h"

char UdpMessageBuffer[118];
uint8_t SSUDP_RX_BUFFER[256];

unsigned long SS_RecvUdpSocket = 0xFFFFFFFF;
bool SSUDP_Listening = false;

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Close the UDP Server
*/
// ------------------------------------------------------------------------------------------------------------
bool UdpServer_Close ( void )
{
	if ( SS_RecvUdpSocket != 0xFFFFFFFF )
	{
		if ( Wifi_IsConnected() )
		{
			closesocket(SS_RecvUdpSocket);
		}
		SS_RecvUdpSocket = 0xFFFFFFFF;
	}
	return ( true );
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief
*/
// ------------------------------------------------------------------------------------------------------------
void UdpServer_Init ( void )
{
	SS_RecvUdpSocket = 0xFFFFFFFF;
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief
*/
// ------------------------------------------------------------------------------------------------------------
bool UdpServer_Listen ( void )
{
sockaddr tSocketAddr;
bool result = false;
long optvalue_block = 0;

	if ( SS_RecvUdpSocket != 0xFFFFFFFF )
	{
		closesocket(SS_RecvUdpSocket);
		SS_RecvUdpSocket = 0xFFFFFFFF;
	}

	SS_RecvUdpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if ( SS_RecvUdpSocket != 0xFFFFFFFF )
	{
		// the destination port
		tSocketAddr.sa_data[0] = (SSC_UDP_PORT_RX >> 8);
		tSocketAddr.sa_data[1] = (0x00ff & SSC_UDP_PORT_RX);

		// the IP address
		tSocketAddr.sa_data[2] = 0x00;
		tSocketAddr.sa_data[3] = 0x00;
		tSocketAddr.sa_data[4] = 0x00;
		tSocketAddr.sa_data[5] = 0x00;

		// Bind to port to enable reception of data
		if (! bind(SS_RecvUdpSocket, &tSocketAddr, sizeof(sockaddr)) )
		{
			setsockopt(SS_RecvUdpSocket, SOL_SOCKET, SOCKOPT_RECV_NONBLOCK, &optvalue_block, 1);

			result = true;
			SSUDP_Listening = true;
		}
		else
		{
			closesocket(SS_RecvUdpSocket);
			SS_RecvUdpSocket = 0xFFFFFFFF;
			ConsoleInsertPrintf("UDP : Failed to bind to RX Port");
		}
	}


	return ( result );
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief We are polling the module for data, the socket is set up to be non blocking
*/
// ------------------------------------------------------------------------------------------------------------
void UdpServer_Task ( void )
{
volatile signed long retVal = -1;
static signed long lastRetVal = 77777;
volatile sockaddr tSocketAddr;
socklen_t tRxPacketLength;
timeval timeout;

	if ( Wifi_IsConnected() )
	{
		memset(&timeout, 0, sizeof(timeval));
		timeout.tv_sec = 0;
		timeout.tv_usec = 5;//50*100;

		if (( SS_RecvUdpSocket != 0xFFFFFFFF ) && (SSUDP_Listening))
		{
			if ( Wifi_IsConnected() )
			{
				retVal = recvfrom(SS_RecvUdpSocket, SSUDP_RX_BUFFER, sizeof(SSUDP_RX_BUFFER), 0, (sockaddr *)&tSocketAddr, &tRxPacketLength);

				if (retVal > 0)
				{
					// We have a message
					ConsoleInsertPrintf("UDP : Message recv'd");
					UdpServer_ProcessMessage((uint32_t *)&tSocketAddr.sa_data[2], SSUDP_RX_BUFFER, sizeof(SSUDP_RX_BUFFER));
				}
				else
				{
					if( retVal != lastRetVal)
					{
						lastRetVal = retVal;
						ConsoleInsertPrintf("UDP : Data Check return val : %i", lastRetVal);
					}
				}
			}
		}
	}
	else
	{
		UdpServer_Close();
	}

}


//*****************************************************************************
//
// SSC_UpdatePwmDuty - Processes a PWM Duty message
//
//*****************************************************************************
void SSUDP_UpdatePwmDuty( uint8_t *buffer )
{
volatile float scaledFreqFloat;
uint8_t bitFlags = buffer[1];
uint8_t mask = 0;
uint8_t i = 0;
uint16_t *pwmDuty;

	if ( bitFlags & BIT7 )
	{
		scaledFreqFloat = Pwm_GetFreq();
	}

	for (i=0; i<7; i++)
	{
		mask = (0x01 << i);
		if ( bitFlags & mask )
		{
			// PWM duty for this channel will be Updated,
			// 2 is the offset in the message of the first PWM value
			pwmDuty = (uint16_t *)&buffer[2+(i*2)];

			if ( bitFlags & BIT7 )
			{
				*pwmDuty = scaledFreqFloat * (float)((float)*pwmDuty / (float)100 );
			}

			Pwm_SetDuty( mask, *pwmDuty );
		}
	}
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Decode the received message and action it
*/
// ------------------------------------------------------------------------------------------------------------
void UdpServer_ProcessMessage ( uint32_t *addr,  uint8_t *buffer, uint16_t lend )
{
bool replyWithStatus = false;
uint32_t freq = 0;
uint32_t freqChunk = 0;

	switch( buffer[0] )
	{
		case SSC_PING :
			replyWithStatus = true;
		break;

		case SSC_PWM_COLOUR_MODE :
			freq = Pwm_GetFreq();
			freqChunk = freq >> 8;
			Pwm_SetDuty( PWM_MASK_RED, freq - (freqChunk * buffer[2]));
			Pwm_SetDuty( PWM_MASK_GREEN, freq - (freqChunk * buffer[3]));
			Pwm_SetDuty( PWM_MASK_BLUE, freq - (freqChunk * buffer[4]));
		break;

		case SSC_SET_UNIT_NAME :
			SysConfig_SetName((char *)&buffer[2], buffer[1]);
		break;


		case SSC_SET_CONFIG :
			SystemConfig.flags = (*((SYSTEM_CONFIG_FLAGS *)&buffer[1]));

			if (SystemConfig.flags.StaticIp)
			{
				// Using a static IP
				SystemConfig.ulStaticIP = ( *((uint32_t *)&buffer[4]) );
				SystemConfig.ulSubnetMask = ( *((uint32_t *)&buffer[8]) );
				SystemConfig.ulGatewayIP = ( *((uint32_t *)&buffer[12]) );
				SystemConfig.ulDnsServer = ( *((uint32_t *)&buffer[16]) );
			}
			else
			{
				// Dynamic
				SystemConfig.ulStaticIP = 0;
				SystemConfig.ulSubnetMask = 0;
				SystemConfig.ulGatewayIP = 0;
			}

			SystemConfig.timeOffset = *((uint16_t *)&buffer[51]);

			// Set the SNTP Address
			SysConfig_SetNtp((char *)&buffer[20], 31);

			UdpServer_Close();

			// Restart the connection
			wlan_stop();
			Wifi_AppInit(0);

/*
			if ( (*(ui8 *)&SystemConfig.flags) != pucData[1] )
			{

			}

*/
		break;

		case SSC_SAVE_CONFIG :
			SysConfig_Save();
		break;

		case SSC_RESET :

			if (('k' == buffer[1]) && ('i' == buffer[2]) && ('c' == buffer[3]) && ('k' == buffer[4]))
			{
				// We have been told to reboot, pull the trigger
				NVIC_SystemReset();
			}
			else if (('r' == buffer[1]) && ('e' == buffer[2]) && ('f' == buffer[3]) && ('l' == buffer[4]))
			{
				// Can't trigger USB boot loader mode with out the button press
				//UpdateFirmwareReq();
			}
			else if (('d' == buffer[1]) && ('e' == buffer[2]) && ('f' == buffer[3]) && ('a' == buffer[4]))
			{
				SysConfig_FactoryDefault();

				SysConfig_Save();

				// Reboot
				NVIC_SystemReset();
			}
		break;

		default :

		break;
	}

	if ( replyWithStatus )
	{
		SSUDP_PingReply(addr);
	}
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief SSUDP_PingReply
*/
// ------------------------------------------------------------------------------------------------------------
void SSUDP_PingReply( uint32_t *addr )
{
sockaddr tSocketAddr;
unsigned long ulDataLength = 118;
//volatile long tmpSocket;
uint8_t temp = 0;

tNetappIpconfigRetArgs *cc3000Status;

	if (( SS_RecvUdpSocket != 0xFFFFFFFF ) && (SSUDP_Listening))
	{
		// Get Cached version
		cc3000Status = getCC3000Info( true );

		tSocketAddr.sa_family = AF_INET;

		// the destination port
		tSocketAddr.sa_data[0] = (SSC_UDP_PORT_TX >> 8);
		tSocketAddr.sa_data[1] = (0x00ff & SSC_UDP_PORT_TX);

		// the destination IP address, already in network order
		tSocketAddr.sa_data[2] = BYTE_N(*addr,0);
		tSocketAddr.sa_data[3] = BYTE_N(*addr,1);
		tSocketAddr.sa_data[4] = BYTE_N(*addr,2);
		tSocketAddr.sa_data[5] = BYTE_N(*addr,3);

		// Message Header
		UdpMessageBuffer[SSC_POS_STARTBYTE] = 0xE1;

		// Our IP
		UdpMessageBuffer[SSC_POS_IP] = cc3000Status->aucIP[0];
		UdpMessageBuffer[SSC_POS_IP + 1] = cc3000Status->aucIP[1];
		UdpMessageBuffer[SSC_POS_IP + 2] = cc3000Status->aucIP[2];
		UdpMessageBuffer[SSC_POS_IP + 3] = cc3000Status->aucIP[3];

		// MAC address already in the buffer
		UdpMessageBuffer[SSC_POS_MAC] = cc3000Status->uaMacAddr[5];
		UdpMessageBuffer[SSC_POS_MAC + 1] = cc3000Status->uaMacAddr[4];
		UdpMessageBuffer[SSC_POS_MAC + 2] = cc3000Status->uaMacAddr[3];
		UdpMessageBuffer[SSC_POS_MAC + 3] = cc3000Status->uaMacAddr[2];
		UdpMessageBuffer[SSC_POS_MAC + 4] = cc3000Status->uaMacAddr[1];
		UdpMessageBuffer[SSC_POS_MAC + 5] = cc3000Status->uaMacAddr[0];

		// SW Rev - TODO : Move to Defines
		UdpMessageBuffer[SSC_POS_SWREV] = 1;
		UdpMessageBuffer[SSC_POS_SWREV+1] = 0;

		memcpy(&UdpMessageBuffer[SSC_POS_UNITNAME], SystemConfig.deviceName, 23);

		UdpMessageBuffer[SSC_POS_CONFIG_BITS] = 0;

		// ADCs
		UdpMessageBuffer[SSC_POS_ADC0] = 1;
		UdpMessageBuffer[SSC_POS_ADC0+1] = 2;

		UdpMessageBuffer[SSC_POS_ADC1] = 1;
		UdpMessageBuffer[SSC_POS_ADC1+1] = 3;

		UdpMessageBuffer[SSC_POS_ADC2] = 1;
		UdpMessageBuffer[SSC_POS_ADC2+1] = 4;


		// Current Relay State
		UdpMessageBuffer[SSC_POS_RELAYSTATE] = 0;

		temp = 1;
		UdpMessageBuffer[SSC_POS_PWM0DUTY] = temp;
		UdpMessageBuffer[SSC_POS_PWM0DUTY + 1] = temp >> 8;

		UdpMessageBuffer[SSC_POS_PWM1DUTY] = temp;
		UdpMessageBuffer[SSC_POS_PWM1DUTY + 1] = temp >> 8;

		UdpMessageBuffer[SSC_POS_PWM2DUTY] = temp;
		UdpMessageBuffer[SSC_POS_PWM2DUTY + 1] = temp >> 8;

		UdpMessageBuffer[SSC_POS_PWMFREQ] = temp;
		UdpMessageBuffer[SSC_POS_PWMFREQ + 1] = temp >> 8;

		// Get RGB Value
		UdpMessageBuffer[SSC_POS_COLOURMODE] = 0;
		UdpMessageBuffer[SSC_POS_REDDUTY] = 1;
		UdpMessageBuffer[SSC_POS_GREENDUTY] = 2;
		UdpMessageBuffer[SSC_POS_BLUEDUTY] = 3;
		UdpMessageBuffer[SSC_POS_STEPSIZE] = 1;

		UdpMessageBuffer[SSC_POS_STEPCNT] = 1;
		UdpMessageBuffer[SSC_POS_STEPCNT + 1] = 255;

		// Relays names not relivent for wifi dip

		UdpMessageBuffer[SSC_POS_RELAY1NAME] = 'R';
		UdpMessageBuffer[SSC_POS_RELAY1NAME+1] = '1';
		UdpMessageBuffer[SSC_POS_RELAY1NAME+2] = 0;

		UdpMessageBuffer[SSC_POS_RELAY2NAME] = 'R';
		UdpMessageBuffer[SSC_POS_RELAY2NAME+1] = '2';
		UdpMessageBuffer[SSC_POS_RELAY2NAME+2] = 0;

		UdpMessageBuffer[SSC_POS_RELAY3NAME] = 'R';
		UdpMessageBuffer[SSC_POS_RELAY3NAME+1] = '3';
		UdpMessageBuffer[SSC_POS_RELAY3NAME+2] = 0;

		UdpMessageBuffer[SSC_POS_RELAY4NAME] = 'R';
		UdpMessageBuffer[SSC_POS_RELAY4NAME+1] = '4';
		UdpMessageBuffer[SSC_POS_RELAY4NAME+2] = 0;

		if ( ulDataLength != sendto ( SS_RecvUdpSocket, UdpMessageBuffer , ulDataLength , 0 , &tSocketAddr , sizeof(sockaddr) ) )
		{
			// Failed to send
			ConsoleInsertPrintf("UDP : Failed to send");
		}
	}
}

