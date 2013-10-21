
#define HTTP_PROWL_HOSTNAME "prowl.weks.net"
#define HTTP_PROWL_URI		"/publicapi/add"
#define HTTP_PROWL_API_KEY	"YOUR_KEY_HERE"

void httpPut (char *hostname, char *uri, uint32_t ip);
void httpGet (char *hostname, char *uri);
void httpPostProwl (char *event, char *text);

#ifdef _HTTP_

char HttpCmdBuffer[500];

#endif
