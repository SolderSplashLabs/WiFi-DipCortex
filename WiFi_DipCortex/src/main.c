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

#include "SolderSplashLpc.h"

#include "time.h"
#include "usb\usbcdc_fifo.h"
#include "console.h"
#include "cli.h"
#include "cc3000_headers.h"
#include "systemConfig.h"
#include "wifi_app.h"
#include "usbcdc.h"
#include "udpServer.h"
#include "sntpClient.h"
#include "timeManager.h"
#include "dns.h"
#include "buttonCon.h"

#define _MAIN_
#include "main.h"

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief SysTick_Handler - Timer interrupt
*/
// ------------------------------------------------------------------------------------------------------------
void SysTick_Handler(void)
{
	sysTicked = true;
	Time_Task();
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief on board led control
*/
// ------------------------------------------------------------------------------------------------------------
void Led(bool ledOn)
{
	if ( ledOn )
	{
		LPC_GPIO->CLR[LED1_PORT] = LED1_PIN_MASK;
	}
	else
	{
		LPC_GPIO->SET[LED1_PORT] = LED1_PIN_MASK;
	}
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief init
*/
// ------------------------------------------------------------------------------------------------------------
void init(void)
{
	SystemCoreClockUpdate();

	// Enable AHB clock to the GPIO domain.
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6);

	// Enable AHB clock to the PinInt, GroupedInt domain and RAM1
	LPC_SYSCON->SYSAHBCLKCTRL |= ((1<<19) | (1<<23) | (1<<24) | (1<<26));

	LpcLowPowerIoInit();

	LED1_INIT;

	SysConfig_Init();

	CLI_Init();

	Buttons_Init();

	// Start USB CDC Console
	UsbCdcInit();

	// Set up the System Tick for a 1ms interrupt
	SysTick_Config(SYSTICK);

	SntpInit();

	Wifi_AppInit(0);
	Wifi_StartAutoConnect();

}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief main - C Entry point
*/
// ------------------------------------------------------------------------------------------------------------
int main(void)
{
 	init();

	while(1)
	{
		if ( sysTicked )
		{
			sysTicked = false;

			Buttons_Task(SYSTICKMS);
			Wifi_Task();

			Dns_Task();
			SntpTask();
			UdpServer_Task();
			Console_Task();
		}
  	}

	return 0;
}
