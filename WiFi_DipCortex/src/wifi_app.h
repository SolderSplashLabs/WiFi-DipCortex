
#define WLAN_EN_PIN_NO		15
#define WLAN_EN_PIN_MASK	1<<WLAN_EN_PIN_NO
#define WLAN_EN_PORT		1

#define DISABLE                                                 (0)
#define ENABLE                                                  (1)
#define CC3000_UNENCRYPTED_SMART_CONFIG

void Wifi_Task ( void );
void Wifi_AppInit ( uint8_t initType );
bool Wifi_IsConnected ( void );

tNetappIpconfigRetArgs * getCC3000Info( bool getCached );

void CC3000_UsynchCallback(long lEventType, char * data, unsigned char length);

char *sendWLFWPatch(unsigned long *Length);
char *sendDriverPatch(unsigned long *Length);
char *sendBootLoaderPatch(unsigned long *Length);
void WlanInterruptEnable();
void WlanInterruptDisable();
long ReadWlanInterruptPin(void);
void WriteWlanPin( unsigned char val );
bool ReadParameters ( void );
void Wifi_GetScanResults ( void );
void Wifi_SendPing ( uint32_t ip, uint32_t attempts, uint32_t packetsize, uint32_t timeout);
void Wifi_StartScan ( uint32_t millseconds );
void StartSmartConfig(void);
void Wifi_StartAutoConnect ( void );

#ifdef _WIFI_APP_

typedef struct Result_Struct
{
  uint32_t  num_networks;
  uint32_t  scan_status;
  uint8_t 	valid:1;			// Note : this works in LpcXpresso, but structure packing depends on the compiler!, valid bit is bit0 LSB
  uint8_t  	rssiByte:7;
  uint8_t   Sec_ssidLen;
  uint16_t  time;
  uint8_t   ssid_name[32];
  uint8_t   bssid[6];
} ResultStruct_t;

//   0 - Open, 1 - WEP, 2 WPA, 3 WPA2
const char * WIFI_SEC_TYPE[] = {"Open", "WEP", "WPA", "WPA2"};
const char * WIFI_STATUS[] = {"Disconnected", "Scanning", "Connecting", "Connected"};

bool IpConfRequested = false;
bool IpConfDataCached = false;

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

netapp_pingreport_args_t *WifiPingReport;

tNetappIpconfigRetArgs ipinfo;

#else

extern volatile uint8_t cMacFromEeprom[MAC_ADDR_LEN];
extern bool ParametersRead;
extern uint8_t cRMParamsFromEeprom[128];						// This array holds the CC3000's eeprom parameters
extern const char * WIFI_STATUS[];

#endif
