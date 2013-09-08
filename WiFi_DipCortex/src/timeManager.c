/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2012 - www.soldersplash.co.uk - C. Matthews - R. Steel

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
