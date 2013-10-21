
#ifdef  __cplusplus
extern "C" {
#endif

#define DNS_HEADER_SIZE					12
#define DNS_PORT_NO						53

void Dns_Task ( void );
void Dns_RefreshServerIp ( void );
int32_t Dns_GetHostByName ( char * strHostname, uint16_t hostnameLen, unsigned long * ipAddress );

#ifdef  __cplusplus
}
#endif // __cplusplus
