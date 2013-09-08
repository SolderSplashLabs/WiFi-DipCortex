#define SECONDS_IN_AN_MIN		60L
#define SECONDS_IN_AN_HOUR		3600L
#define SECONDS_IN_AN_DAY		86400L
#define SECONDS_IN_HALF_DAY		43200L


typedef struct TIME_STRUCT
{
	uint8_t hour;
	uint8_t min;
	uint8_t second;

	uint8_t day;
	uint8_t month;
	uint8_t year;

} TIME_STRUCT;

void Time_Task ( void );
uint32_t Time_StampNow ( int16_t minOffset );
uint32_t Time_Uptime ( void );
void Time_Get ( TIME_STRUCT *time );
void Time_SetUnix( uint32_t timeStamp );
