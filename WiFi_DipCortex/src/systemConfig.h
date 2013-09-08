
#define DIP_CORTEX_NAME_LEN			24
#define SNTP_SERVER_LEN				32

void SysConfig_Init ( void );
void SysConfig_Save ( void );
void SysConfig_Load ( void );
void SysConfig_FactoryDefault ( void );

typedef struct
{
	uint8_t StaticIp:1;					// Use a Static IP
	uint8_t NtpEnabled:1;				// Auto time/date update
	uint8_t Spare:6;
}
SYSTEM_CONFIG_FLAGS;

typedef struct
{
	uint16_t	checksum;
	uint16_t	version;				// Version of these settings
	uint32_t 	ulStaticIP;				// The static IP address to use if DHCP is not in use
	uint32_t 	ulGatewayIP;			// The default gateway IP address to use if DHCP is not in use.
	uint32_t 	ulSubnetMask;			// The subnet mask to use if DHCP is not in use.
	uint32_t	ulDnsServer;			// DNS Server Address

	int16_t 	timeOffset;				// Signed Time Outset in minutes, some time zones are .30's different
	char 	deviceName[DIP_CORTEX_NAME_LEN];		// Name of the unit
	char 	sntpServerAddress[SNTP_SERVER_LEN];			// NTP server used to fetch time
	char 	apiKey[64];					// An API key

	SYSTEM_CONFIG_FLAGS flags;
} SYSTEM_CONFIG;


static const SYSTEM_CONFIG SYSTEM_CONFIG_DEFAULTS =
{
	(uint16_t)0x0000,						// Checksum
	(uint16_t)0x0000,						// version
	(uint32_t)0x00000000,					// Static IP address
	(uint32_t)0x00000000,					// Default gateway IP address (used only if static IP is in use).
	(uint32_t)0xFFFFFF00,					// Subnet mask (used only if static IP is in use).
	(uint32_t)0x00000000,					// DNS Server Address

	(int16_t)0,

	// WiFi DipCortex Name
	{
		'W','i','F','i','-','D','i','p','c','o', 'r' , 't' , 'e' , 'x' , 0 , 0,
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0,
	},
	// SNTP Address 0.pool.ntp.org
	{
		'0','.','p','o','o','l','.','n','t','p','.','o','r','g', 0 , 0 ,
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	},
	// API Key
	{
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	},
	// flags
	{0,1,0},
};

#ifdef _SYS_CONF_

SYSTEM_CONFIG SystemConfig;

#else

extern SYSTEM_CONFIG SystemConfig;

#endif
