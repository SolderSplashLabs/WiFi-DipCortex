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
