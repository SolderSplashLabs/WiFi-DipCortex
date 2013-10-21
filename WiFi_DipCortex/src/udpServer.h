#define SSC_UDP_PORT_RX		11028
#define SSC_UDP_PORT_TX		11029

enum SSC_COMMANDS
{
	SSC_PING = 1,						// Check the Unit is there
	SSC_EXTENDED_PING,					// Get Extended Info
	SSC_RELAY_CON = 0x0A,				// Control it's relays
	SSC_PWM_DUTY,						// Duty for a single PWM
	SSC_PWM_DUTY_ALL,					// Duty for a PWMs
	SSC_PWM_FREQ,						// Set Freq of the PWMs masked
	SSC_PWM_COLOUR_MODE,				// Use a colour mode to control the PWMs
	SSC_SET_UNIT_NAME = 0x20,
	SSC_SET_RELAY_NAME,					// Set the supplied relay no's name
	SSC_SET_CONFIG,						// Change Settings
	SSC_SAVE_CONFIG,					// Save Current config to non vol
	SSC_OUTPUTS_ON_OFF = 0x30,			// Control all outputs, turn if on/off
	SSC_SPLASHPIXEL_FBSET = 0x45,		// Set the SplashPixel Framebuffer
	SSC_MANUAL_GPIO_DIR = 0x50, 		// Set gpio direction, 1 output
	SSC_MANUAL_GPIO_DATA = 0x51, 		// set gpio outputs high or low
	SSC_MANUAL_GPIO_CONF = 0x52,
	SSC_GET_ALL_GPIO = 0x53,
	SSC_INIT_GPIO_CONF = 0x54,			// Set init gpio dir and output
	SSC_INIT_GPIO_RUN = 0x55,
	SSC_LOGIC_COMMAND = 0x60,

	SSC_LOGIC_EDT_ACT = 0x62,			// Insert/Edit an action
	SSC_LOGIC_EDT_COND = 0x63,			// Insert/Edit a condition
	SSC_LOGIC_EDT_EVENT = 0x64,			// Insert/Edit an event

	SSC_LOGIC_READ_ACTION = 0x65,
	SSC_LOGIC_READ_CONDTION = 0x66,
	SSC_LOGIC_READ_EVENTS = 0x67,

	SSC_BRIDGE_SCAN = 0x80,
	SSC_SB_SERVOPOS = 0x90,

	SSC_REPLY_PING = 0xE1,
	SSC_REPLY_CONFIG = 0xE2,
	SSC_REPLY_ALL_GPIO = 0xE3,

	SSC_REPLY_LOGIC_ACT = 0xE4,
	SSC_REPLY_LOGIC_COND = 0xE5,
	SSC_REPLY_LOGIC_EVENTS = 0xE6,

	SSC_RESET = 0xFF
};

enum SSC_REPLY_POSITIONS
{
	SSC_POS_STARTBYTE = 0,
	SSC_POS_IP,
	SSC_POS_MAC = 5,
	SSC_POS_SWREV = 11,
	SSC_POS_UNITNAME = 13,
	SSC_POS_CONFIG_BITS = 36,
	SSC_POS_SPARE,
	SSC_POS_ADC0,
	SSC_POS_ADC1 = 40,
	SSC_POS_ADC2 = 42,
	SSC_POS_RELAYSTATE = 44,
	SSC_POS_PWM0DUTY,
	SSC_POS_PWM1DUTY = 47,
	SSC_POS_PWM2DUTY = 49,
	SSC_POS_PWMFREQ = 51,
	SSC_POS_COLOURMODE = 53,

	SSC_POS_PWM3DUTY = 54,
	SSC_POS_PWM4DUTY = 56,
	SSC_POS_PWMFREQ2 = 58,

	SSC_POS_REDDUTY = 60,
	SSC_POS_GREENDUTY,
	SSC_POS_BLUEDUTY,

	SSC_POS_STEPSIZE,
	SSC_POS_STEPCNT,

	SSC_POS_RELAY1NAME = 66,
	SSC_POS_RELAY2NAME = 78,
	SSC_POS_RELAY3NAME = 90,
	SSC_POS_RELAY4NAME = 102,

	SSC_POS_TIMESTAMP = 114
};

bool UdpServer_Close ( void );
void UdpServer_Init ( void );
void SSUDP_PingReply( uint32_t *addr );
void UdpServer_ProcessMessage ( uint32_t *addr,  uint8_t *buffer, uint16_t lend );
void UdpServer_Task ( void );
bool SSUDP_Close ( void );
bool UdpServer_Listen ( void );

