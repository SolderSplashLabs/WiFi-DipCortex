
#define HTTP_PROWL_HOSTNAME "prowl.weks.net"
#define HTTP_PROWL_URI		"/publicapi/add"
#define HTTP_PROWL_API_KEY	"c1099524f066abde30137c06415bca68124927fe"

void httpPut (char *hostname, char *uri, uint32_t ip);
void httpGet (char *hostname, char *uri);
void httpPostProwl (char *event, char *text);
void httpPutJsonString (char *hostname, char *uri, uint32_t ip, char * jsonStr);

#ifdef _HTTP_

char HttpCmdBuffer[500];

#endif
