/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2013 - www.soldersplash.co.uk - C. Matthews - R. Steel


	@file     dnsCache.c
	@author   Carl Matthews (soldersplash.co.uk)
	@date     03 July 2013

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

	stores recent DNS results in a list with a timestamp so old entries can be removed or replaced

*/

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "SolderSplashLpc.h"
#include "cc3000_headers.h"
#include "console.h"
#include "timeManager.h"
#include "dns.h"

#define _DNS_CACHE_
#include "dnsCache.h"

// ------------------------------------------------------------------------------------------------------------
/*!
	@brief Clear the cache
*/
// ------------------------------------------------------------------------------------------------------------
void DnsCache_Clear ( void )
{
uint8_t i = 0;

	for (i=0; i<DNS_MAX_CACHE_SIZE; i++)
	{
		DnsCache[i].hostname[0] = 0;
	}
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief DnsCacheQuery - Get an IP for the hostname, if it's in our cache ipAddress is loaded with the address

	Returns a negative number if it fails or a positive value on success similar to gethostbyname
*/
// ------------------------------------------------------------------------------------------------------------
int8_t DnsCache_Query( char * strHostname, uint16_t hostnameLen, unsigned long * ipAddress )
{
uint8_t attempts = 0;
uint8_t i = 0;
int8_t result = -1;
uint8_t updateRecordNo = 0;
uint32_t oldestEntry = 0xffffffff;
int gethostret = 0;

	// Check the pointers have been set and the length is valid
	if (( strHostname ) && ( ipAddress ))
	{
		if (( hostnameLen > 0 ) && ( hostnameLen < DNS_MAX_HOSTNAME_LEN-1 ))
		{
			// Search the cache
			for (i=0; i<DNS_MAX_CACHE_SIZE; i++)
			{
				if ( DnsCache[i].hostname[0] )
				{
					// this record holds a hostname, lets compare

					if(! strcmp(strHostname, DnsCache[i].hostname) )
					{
						// Found it !, how old is it?
						if ( ( DnsCache[i].updatedTimestamp + DNS_MAX_AGE ) > Time_Uptime() )
						{
							// Record valid lets use it
							*ipAddress = DnsCache[i].ipAddress;
							result = true;
						}
						else
						{
							// Record is old, remove it
							DnsCache[i].hostname[0] = 0;
						}
					}
				}
			}
		}

		if ( result < 0 )
		{
			// Couldn't find it in in the cache
			attempts = 0;
			while ( attempts < 3 )
			{
				gethostret = gethostbyname(strHostname, hostnameLen, ipAddress);
				if ( gethostret < 0)
				{
					ConsoleInsertPrintf("DNS Resolve failed - Return Code %d", gethostret);

					// Failed to resolve the hostname
					attempts ++;
				}
				else
				{
					// we have a valid result
					result = true;
					break;
				}
			}

			if ( result > 0 )
			{
				// Have a result, cache it for next time if we can
				if ( hostnameLen < DNS_MAX_HOSTNAME_LEN-1 )
				{
					updateRecordNo = 0;

					for (i=0; i<DNS_MAX_CACHE_SIZE; i++)
					{
						if (! DnsCache[i].hostname[0] )
						{
							// Found a space to cache it
							updateRecordNo = i;
							break;
						}
						else
						{
							if ( DnsCache[i].updatedTimestamp < oldestEntry )
							{
								oldestEntry = DnsCache[i].updatedTimestamp;
								updateRecordNo = i;
							}
						}
					}

					memcpy(	DnsCache[updateRecordNo].hostname, strHostname, hostnameLen );
					DnsCache[updateRecordNo].hostname[hostnameLen] = 0;
					DnsCache[updateRecordNo].ipAddress = *ipAddress;
					DnsCache[updateRecordNo].updatedTimestamp = Time_Uptime();

					// TODO : No space, replace the oldest record instead
				}
			}
		}
	}

	return ( result );
}

// ------------------------------------------------------------------------------------------------------------
/*!
    DnsCache_Print
*/
// ------------------------------------------------------------------------------------------------------------
void DnsCache_Print( void )
{
uint8_t i = 0;
uint8_t *ip;

	for (i=0; i<DNS_MAX_CACHE_SIZE; i++)
	{
		if ( DnsCache[i].hostname[0] )
		{
			ip = (uint8_t *)&DnsCache[i].ipAddress;
			ConsoleInsertPrintf("%s - IP %d.%d.%d.%d - Seconds old %u", (char *)&DnsCache[i].hostname[0], ip[3], ip[2], ip[1], ip[0], (Time_Uptime() - DnsCache[i].updatedTimestamp));
		}
	}
}

