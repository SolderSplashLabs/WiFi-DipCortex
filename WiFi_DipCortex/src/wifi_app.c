/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2013 - www.soldersplash.co.uk - C. Matthews - R. Steel


	@file     wifi_app.c
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

*/

#include "SolderSplashLpc.h"
#include "main.h"
#include "cc3000_headers.h"
#include "usb\usbcdc_fifo.h"
#include "lpc\gpio.h"
#include "console.h"
#include "delay.h"
#include "cc3000\spi.h"
#include "SystemConfig.h"
#include "buttonCon.h"

#include "SolderSplashUdp.h"

#define _WIFI_APP_
#include "wifi_app.h"

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Wifi_Task
*/
// ------------------------------------------------------------------------------------------------------------
void Wifi_Task ( void )
{
volatile tNetappIpconfigRetArgs *cc3000Status;
uint8_t pressed;

	Buttons_GetPressed(&pressed);

	if (Wifi_IsConnected())
	{
		if (! IpConfRequested )
		{
			cc3000Status = getCC3000Info( false );
			SntpUpdate( false );
			SSUDP_Listen();

			IpConfRequested = true;
		}

		if (( pressed & BUTTON1 ) || ( pressed & BUTTON2 ))
		{
			httpPostProwl("Button Press", "Someone pressed the button!");
		}
	}

	Buttons_GetHeld(&pressed);

	if (( pressed & BUTTON3 ) && ( pressed & BUTTON2 ))
	{
		// NVIC_SystemReset();
	}

	Buttons_ActionPressed();
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Wifi_AppInit
*/
// ------------------------------------------------------------------------------------------------------------
void Wifi_AppInit ( uint8_t initType )
{
unsigned long ipAddr = 0;

	// SPI Init
	SpiInit();

	LPC_GPIO->DIR[WLAN_IRQ_PORT] &= ~(WLAN_IRQ_PIN_MASK);

	// EN as output, high to enable
	LPC_GPIO->DIR[WLAN_EN_PORT] |= WLAN_EN_PIN_MASK;
	LPC_GPIO->CLR[WLAN_EN_PORT] = WLAN_EN_PIN_MASK;

	// WLAN On API Implementation
	wlan_init( CC3000_UsynchCallback, sendWLFWPatch, sendDriverPatch, sendBootLoaderPatch, ReadWlanInterruptPin, Spi_IrqEnable, Spi_IrqDisable, WriteWlanPin);

	// Trigger a WLAN device, 0 = no patches, 1 = patches availible
	wlan_start(initType);

	wlan_smart_config_set_prefix((char*)aucCC3000_prefix);

	if ( SystemConfig.flags.StaticIp )
	{
		netapp_dhcp(&SystemConfig.ulStaticIP, &SystemConfig.ulSubnetMask, &SystemConfig.ulGatewayIP, &SystemConfig.ulGatewayIP );
	}
	else
	{
		netapp_dhcp(&ipAddr, &ipAddr, &ipAddr, &ipAddr);
	}

	// Fast connect, use profiles
	wlan_ioctl_set_connection_policy(0, 1, 1);

	DelayUs(1000);

	//wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|HCI_EVNT_WLAN_UNSOL_INIT|HCI_EVNT_WLAN_ASYNC_PING_REPORT);
	wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|HCI_EVNT_WLAN_UNSOL_INIT);

	// Setup the timeout values
	netapp_timeout_values( (unsigned long *)&WIFI_DHCP_TIMEOUT, (unsigned long *)&WIFI_ARP_TIMEOUT, (unsigned long *)&WIFI_KEEPALIVE_TIMEOUT, (unsigned long *)&WIFI_INACTIVITY_TIMEOUT );

	// Init soldersplash UDP coms
	SSUDP_Init();
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Wifi_IsConnected - Is WiFi Connected?
*/
// ------------------------------------------------------------------------------------------------------------
bool Wifi_IsConnected ( void )
{
	if ( ulCC3000Connected && ulCC3000DHCP )
	{
		return ( true );
	}
	else
	{
		return ( false );
	}
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief PIN_INT0_IRQHandler - IRQ Intterupt on change
*/
// ------------------------------------------------------------------------------------------------------------
void PIN_INT0_IRQHandler ( void )
{
	if ( LPC_GPIO_PIN_INT->IST & (0x1<<0) )
	{
		LPC_GPIO_PIN_INT->FALL = 0x1<<0;
		LPC_GPIO_PIN_INT->RISE = 0x1<<0;
		LPC_GPIO_PIN_INT->IST = 0x1<<0;
		// Call CC3000 Interrupt
		Spi_IrqInterrupt();
	}
}

// ------------------------------------------------------------------------------------------------------------
/*!
	CC3000_UsynchCallback

    @brief The function handles asynchronous events that come from CC3000 device
*/
// ------------------------------------------------------------------------------------------------------------
void CC3000_UsynchCallback(long lEventType, char * data, unsigned char length)
{
	switch (lEventType)
	{
		case HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE :
			ulSmartConfigFinished = 1;
			ConsoleInsertPrintf("Callback : Simple Config Done");
		break;

		case HCI_EVNT_WLAN_UNSOL_CONNECT :
			ulCC3000Connected = 1;

			Led( true );
			ConsoleInsertPrintf("Callback : Connected");
		break;

		case HCI_EVNT_WLAN_UNSOL_DISCONNECT :
			ulCC3000Connected = 0;
			ulCC3000DHCP      = 0;
			ulCC3000DHCP_configured = 0;
			IpConfDataCached = false;
			IpConfRequested = false;

			Led( false );

			ConsoleInsertPrintf("Callback : Disconnected");
		break;

		case HCI_EVNT_WLAN_UNSOL_DHCP :

			// Note : WiFi connection can not be used until we receive this DHCP event even if we are using a static IP
			ulCC3000DHCP = 1;

			ConsoleInsertPrintf("Callback : DHCP - IP Address : %i.%i.%i.%i", data[3], data[2], data[1], data[0]);
		break;

		case HCI_EVENT_CC3000_CAN_SHUT_DOWN :
			OkToDoShutDown = 1;
			ConsoleInsertPrintf("Callback : CC3000 TX Complete");
		break;

		case HCI_EVNT_WLAN_ASYNC_PING_REPORT :
			ConsoleInsertPrintf("Callback : Recvd Ping!");
			WifiPingReport = data;

			ConsoleInsertPrintf("Ping Results : Min ( %d ) Max ( %d ) Average ( %d )", WifiPingReport->min_round_time, WifiPingReport->max_round_time, WifiPingReport->avg_round_time);
		break;

		case HCI_EVNT_BSD_TCP_CLOSE_WAIT :
			ConsoleInsertPrintf("Callback : Socket no %u closed", data[0]);
		break;

		case HCI_EVNT_WLAN_UNSOL_INIT :
			//Notification that the CC3000 device finished the initialization process
			ConsoleInsertPrintf("Callback : Init 0x%04X", lEventType);
		break;

		case HCI_EVNT_WLAN_TX_COMPLETE :
			ConsoleInsertPrintf("Callback : TX Complete 0x%04X", lEventType);
		break;

		case HCI_EVNT_WLAN_KEEPALIVE :
			ConsoleInsertPrintf("Callback : KeepAlive 0x%04X", lEventType);
		break;

		default :
			// Not sure what this call back is!
			ConsoleInsertPrintf("Callback : Unknown : 0x%04X ", lEventType);

	}
}




//*****************************************************************************
//
//! WriteWlanPin
//!
//! \param  new val
//!
//! \return none
//!
//! \brief  Set enable pin
//
//*****************************************************************************
void WriteWlanPin( unsigned char val )
{
    if (val)
    {
    	LPC_GPIO->SET[WLAN_EN_PORT] = WLAN_EN_PIN_MASK;
    }
    else
    {
    	LPC_GPIO->CLR[WLAN_EN_PORT] = WLAN_EN_PIN_MASK;
    }
}

//*****************************************************************************
//
//! ReadWlanInterruptPin
//!
//! \param  none
//!
//! \return none
//!
//! \brief  return wlan interrup pin
//
//*****************************************************************************
long ReadWlanInterruptPin(void)
{
	uint32_t result = LPC_GPIO->PIN[WLAN_IRQ_PORT] & WLAN_IRQ_PIN_MASK;

	return (result);
}

//*****************************************************************************
//
//! sendWLFWPatch
//!
//! \param  pointer to the length
//!
//! \return none
//!
//! \brief  The function returns a pointer to the FW patch: since there is no patch yet - it returns 0
//
//*****************************************************************************

char *sendWLFWPatch(unsigned long *Length)
{
	*Length = 0;
	return NULL;
}

//*****************************************************************************
//
//! sendDriverPatch
//!
//! \param  pointer to the length
//!
//! \return none
//!
//! \brief  The function returns a pointer to the driver patch: since there is no patch yet -
//!				it returns 0
//
//*****************************************************************************
char *sendDriverPatch(unsigned long *Length)
{
	*Length = 0;
	return NULL;
}

//*****************************************************************************
//
//! sendBootLoaderPatch
//!
//! \param  pointer to the length
//!
//! \return none
//!
//! \brief  The function returns a pointer to the boot loader patch: since there is no patch yet -
//!				it returns 0
//
//*****************************************************************************
char *sendBootLoaderPatch(unsigned long *Length)
{
	*Length = 0;
	return NULL;
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief getCC3000Info
*/
// ------------------------------------------------------------------------------------------------------------
tNetappIpconfigRetArgs * getCC3000Info( bool getCached )
{
	if (! Wifi_IsConnected() )
	{
		IpConfDataCached = false;
	}

	if (! getCached )
	{
		// Make sure we refresh the ipconfig/status
		IpConfDataCached = false;
	}

	if (! IpConfDataCached )
	{
		netapp_ipconfig(&ipinfo);
		IpConfDataCached = true;
	}

    return (&ipinfo);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief ReadParameters - Reads all of the CC3000 Parameters - needed before a firmware update
*/
// ------------------------------------------------------------------------------------------------------------
bool ReadParameters ( void )
{
volatile uint8_t status = 0xFF;
volatile uint8_t counter = 0;
volatile uint8_t index = 0;
volatile uint8_t *pRMParams;
bool result = true;

	// read MAC address
	if (! nvmem_get_mac_address(cMacFromEeprom) )
	{
		status = 1;

		// have 3 goes at it, a non zero return seems to indicate an error
		// even tho nvmem_read's headers says otherwise
		while ((status) && (counter < 3))
		{
			status = 0;
			pRMParams = cRMParamsFromEeprom;

			// Read RM parameters, two chunks of 64
			// TODO : for some reason a single 128byte read hard faults!
			for (index = 0; index < 2; index++)
			{
				status |= nvmem_read(NVMEM_RM_FILEID, 64, 64*index, pRMParams);
				pRMParams += 64;
			}

			counter++;
		}

		if ( counter < 3 )
		{
			ParametersRead = true;
		}
	}
	else
	{
		// Failed to read the mac address
		result = false;
		ParametersRead = false;
	}

	if ( counter > 2 )
	{
		result = false;
		ParametersRead = false;
	}

	return ( result );
}


//*****************************************************************************
//
//! StartSmartConfig
//!
//!  @param  None
//!
//!  @return none
//!
//!  @brief  The function triggers a smart configuration process on CC3000.
//!			it exists upon completion of the process
//
//*****************************************************************************
void StartSmartConfig(void)
{
	ulSmartConfigFinished = 0;
	ulCC3000Connected = 0;
	ulCC3000DHCP = 0;
	OkToDoShutDown=0;

	// Reset all the previous configuration
	wlan_ioctl_set_connection_policy(DISABLE, DISABLE, DISABLE);

	//Wait until CC3000 is disconnected
	while (ulCC3000Connected == 1)
	{
		//__delay_cycles(1000);
		__WFI();
	}

	// Trigger the Smart Config process
	wlan_smart_config_set_prefix((char*)aucCC3000_prefix);

	// Start the SmartConfig start process
	wlan_smart_config_start(0 /* unencrypted SmartConfig */);

	// Wait for Smartconfig process complete
	while (ulSmartConfigFinished == 0)
	{

		//__delay_cycles(6000000);
		__WFI();
		__WFI();
		//__delay_cycles(6000000);
	}
#ifndef CC3000_UNENCRYPTED_SMART_CONFIG
	// create new entry for AES encryption key
	nvmem_create_entry(NVMEM_AES128_KEY_FILEID,16);

	// write AES key to NVMEM
	aes_write_key((unsigned char *)(&smartconfigkey[0]));

	// Decrypt configuration information and add profile
	wlan_smart_config_process();
#endif

	// Configure to connect automatically to the AP retrieved in the
	// Smart config process
	wlan_ioctl_set_connection_policy(DISABLE, DISABLE, ENABLE);

	// reset the CC3000
	wlan_stop();

	DelayUs(5000);

	wlan_start(0);

	// Mask out all non-required events
	wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|HCI_EVNT_WLAN_UNSOL_INIT|HCI_EVNT_WLAN_ASYNC_PING_REPORT);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Wifi_StartScan - Start a scan for wifi access points SSID's
*/
// ------------------------------------------------------------------------------------------------------------
void Wifi_StartScan ( uint32_t millseconds )
{
const uint32_t uiMinDwellTime = 100;
const uint32_t uiMaxDwellTime = 100;
const uint32_t uiNumOfProbeRequests = 5;
const uint32_t uiChannelMask = 0x1fff;
const int32_t iRSSIThreshold = -120;	// This might be a bit low, ti recommend -80
const uint32_t uiSNRThreshold = 0;
const uint32_t uiDefaultTxPower = 205;
const uint32_t aiIntervalList[16] = { 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000 };

	// if millseconds = 1 that's a 10 minute scan!
	wlan_ioctl_set_scan_params(millseconds, uiMinDwellTime,	uiMaxDwellTime,	uiNumOfProbeRequests, uiChannelMask, iRSSIThreshold, uiSNRThreshold, uiDefaultTxPower, aiIntervalList);

}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Wifi_GetScanResults - Get scan result
*/
// ------------------------------------------------------------------------------------------------------------
void Wifi_GetScanResults ( void )
{
uint8_t i = 0;
uint8_t encryptionType = 0;
uint8_t ssidLen = 0;
ResultStruct_t scanResults;

	wlan_ioctl_get_scan_results(0, (uint8_t*)&scanResults);
	ConsoleInsertPrintf("Scan found %u WiFi networks", scanResults.num_networks);

	i = scanResults.num_networks;
	while (i)
	{
		// is this record valid?
		//if (scanResults.rssiByte & BIT0)
		if (scanResults.valid)
		{
			//scanResults.rssiByte >>= 1;

			encryptionType = (scanResults.Sec_ssidLen & 0x03);

			ssidLen = scanResults.Sec_ssidLen >> 2;
			if ( ssidLen < 32 )
			{
				// 0 Terminate the string for easy printf-ing
				scanResults.ssid_name[ ssidLen ] = 0;
			}

			//ConsoleInsertPrintf("%s (%s) RSSI (%d) Age (%d)", scanResults.ssid_name, WIFI_SEC_TYPE[encryptionType], scanResults.rssiByte, scanResults.time);
			ConsoleInsertPrintf("%s (%s) RSSI (%d)", scanResults.ssid_name, WIFI_SEC_TYPE[encryptionType], scanResults.rssiByte);
		}
		wlan_ioctl_get_scan_results(0, (uint8_t*)&scanResults);

		i --;
	}
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Wifi_Ping -
*/
// ------------------------------------------------------------------------------------------------------------
void Wifi_SendPing ( uint32_t ip, uint32_t attempts, uint32_t packetsize, uint32_t timeout)
{
	netapp_ping_send(&ip, attempts, packetsize, timeout);
}

