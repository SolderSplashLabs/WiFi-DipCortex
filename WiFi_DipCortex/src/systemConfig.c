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

	A central system configuration

*/

#include "string.h"

#include "SolderSplashLpc.h"

#define _SYS_CONF_
#include "systemConfig.h"

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief SysConfig_FactoryDefault
*/
// ------------------------------------------------------------------------------------------------------------
void SysConfig_FactoryDefault ( void )
{
	memcpy( (SYSTEM_CONFIG *)&SystemConfig, (SYSTEM_CONFIG *)&SYSTEM_CONFIG_DEFAULTS, sizeof(SystemConfig));
	SystemConfig.flags.StaticIp = 0;
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief SysConfig_Load
*/
// ------------------------------------------------------------------------------------------------------------
void SysConfig_Load ( void )
{
	// TODO : Read from internal eeprom
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief SysConfig_Save
*/
// ------------------------------------------------------------------------------------------------------------
void SysConfig_Save ( void )
{
	// TODO : save to internal eeprom
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief SysConfig_Init
*/
// ------------------------------------------------------------------------------------------------------------
void SysConfig_Init ( void )
{
	SysConfig_FactoryDefault();
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Set the name of the module
*/
// ------------------------------------------------------------------------------------------------------------
void SysConfig_SetName ( char *newName, uint8_t len )
{
	if (len >= DIP_CORTEX_NAME_LEN-1)
	{
		len = DIP_CORTEX_NAME_LEN-1;
	}
	memset(SystemConfig.deviceName, 0, DIP_CORTEX_NAME_LEN);
	memcpy(SystemConfig.deviceName, newName, len);
	SystemConfig.deviceName[len] = 0;
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Set the name of the module
*/
// ------------------------------------------------------------------------------------------------------------
void SysConfig_SetNtp  ( char *newNtp, uint8_t len )
{
	if (len >= SNTP_SERVER_LEN-1)
	{
		len = SNTP_SERVER_LEN-1;
	}
	memset(SystemConfig.sntpServerAddress, 0, SNTP_SERVER_LEN);
	memcpy(SystemConfig.sntpServerAddress, newNtp, len);
	SystemConfig.sntpServerAddress[len] = 0;
}

