#include <stdio.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

/* Basic idea for sequential proxy
1. Parse GET request
2. Extract host name and query path
3. Connect to host and send query path
4. Receive data from server
5. Forward data to client/brower connected
*/
int main()
{
    printf("%s", user_agent_hdr);
    return 0;
}
