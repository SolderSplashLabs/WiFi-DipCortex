/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2012 - www.soldersplash.co.uk - C. Matthews - R. Steel

	@file     timeManager.c
	@author   Carl Matthews (soldersplash.co.uk)
	@date     1 Sep 2013

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

	Keep track of uptime and trigger NTP updates
*/

#include "SolderSplashLpc.h"
#include "systemConfig.h"

#include "sntpClient.h"
#include "main.h"

#include "timeManager.h"

uint32_t Uptime = 0;
uint32_t UnixTime = 0;
uint32_t MsTimer = 0;

// *****************************************************************************
//
// Time_Task
//
// *****************************************************************************
void Time_Task ( void )
{
	static uint16_t msCounter = 0;
	static uint32_t ntpCounter = 0;

	MsTimer += SYSTICKMS;
	msCounter += SYSTICKMS;

	if (msCounter >= 1000)
	{
		msCounter = msCounter - 1000;
		UnixTime++;
		Uptime++;
		ntpCounter ++;

		// Refresh twice a day, is this too much?
		if ( ntpCounter >= SECONDS_IN_HALF_DAY )
		{
			// Each day update the clock
			if ( SystemConfig.flags.NtpEnabled )
			{
				SntpUpdate(false);
			}
			ntpCounter = 0;
		}
	}

}


// *****************************************************************************
//
// Time_SetUnix
//
// *****************************************************************************
void Time_SetUnix( uint32_t timeStamp )
{
	UnixTime = timeStamp;
}

// *****************************************************************************
//
// Time_StampNow
//
// *****************************************************************************
uint32_t Time_StampNow ( int16_t minOffset )
{
uint32_t secOffset = 0;

	// Convert to seconds
	secOffset = minOffset * 60;

	return (UnixTime+secOffset);
}

// *****************************************************************************
//
// Time_Uptime
//
// *****************************************************************************
uint32_t Time_Uptime ( void )
{
	return (Uptime);
}

// *****************************************************************************
// Time_Get
// the calling function supplies a structure and it is populated with the current time
// *****************************************************************************
void Time_Get ( TIME_STRUCT *time )
{
	// Make sure it's not a null pointer
	if (time != 0)
	{
		// get the remainder after taking the days out, then calc the hours
		time->hour = (UnixTime % SECONDS_IN_AN_DAY) / SECONDS_IN_AN_HOUR;
		// next do the same to get minutes
		time->min = (UnixTime % SECONDS_IN_AN_HOUR) / SECONDS_IN_AN_MIN;
		time->second = UnixTime % SECONDS_IN_AN_MIN;
	}
}
