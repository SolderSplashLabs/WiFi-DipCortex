
#define WLAN_EN_PIN_NO		15
#define WLAN_EN_PIN_MASK	1<<WLAN_EN_PIN_NO
#define WLAN_EN_PORT		1


void Wifi_AppInit ( uint8_t initType );
void CC3000_UsynchCallback(long lEventType, char * data, unsigned char length);

char *sendWLFWPatch(unsigned long *Length);
char *sendDriverPatch(unsigned long *Length);
char *sendBootLoaderPatch(unsigned long *Length);
void WlanInterruptEnable();
void WlanInterruptDisable();
long ReadWlanInterruptPin(void);
void WriteWlanPin( unsigned char val );

#ifdef _WIFI_APP_

const unsigned long WIFI_DHCP_TIMEOUT			=	86400;
const unsigned long WIFI_ARP_TIMEOUT			=	3600;
const unsigned long WIFI_KEEPALIVE_TIMEOUT		=	10;
const unsigned long WIFI_INACTIVITY_TIMEOUT		=	0;				// When listening for UDP Commands the port will be closed automatically unless we disable the timeout

// Simple Config Prefix
volatile char aucCC3000_prefix[] = {'T', 'T', 'T'};

// State flags
volatile unsigned long ulSmartConfigFinished, ulCC3000Connected,ulCC3000DHCP,OkToDoShutDown, ulCC3000DHCP_configured;

volatile uint8_t cMacFromEeprom[MAC_ADDR_LEN];
bool ParametersRead = false;
uint8_t cRMParamsFromEeprom[128];						// This array holds the CC3000's eeprom parameters

#else


#endif
