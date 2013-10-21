
#ifdef  __cplusplus
extern "C" {
#endif

#define DNS_MAX_CACHE_SIZE 		5		// number of DNS records we can cache
#define DNS_MAX_AGE				10800	// Max age in seconds of a record, records older are not used = 10800 3 Hours, CC3000 doesnt supply the TTL value from the DNS record so we have to make it up
#define DNS_MAX_HOSTNAME_LEN	25		// Max hostname size of cacheable hostnames, longer will not be cached

typedef struct DNS_CACHE
{
			 char 	hostname[DNS_MAX_HOSTNAME_LEN];		// Hostname string
	unsigned long 	ipAddress;							// IP address in ???? format
	unsigned long	updatedTimestamp;					// Timestamp this record was last updated
} DNS_CACHE;

int8_t DnsCache_Query( char * strHostname, uint16_t hostnameLen, unsigned long * ipAddress );
void DnsCache_Print( void );
void DnsCache_Clear ( void );

#ifdef _DNS_CACHE_

DNS_CACHE DnsCache[ DNS_MAX_CACHE_SIZE ];

#endif

#ifdef  __cplusplus
}
#endif // __cplusplus
