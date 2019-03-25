//#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* Web object structure */
typedef struct {
    char *name;         /* Name of web object name (query path) */
    void *data_buf;     /* Pointer to data buffer of web object */
    int size;           /* Size of data buffer */
    int rcount;         /* Num of times object accessed (for evicition policy) */
    sem_t mutex;        /* Mutex to protect shared acess to web object */
    sem_t num_reader;   /* Number of active readers */
    void *next;         /* Pointer to next object in list */  

} webobj_t;

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *connection_hdr = "Connection: close\r\n";
static const char *proxy_connection_hdr = "Proxy-Connection: close\r\n";
static const char *user_agent_key = "User-Agent";
static const char *host_key = "Host";
static const char *connection_key = "Connection";
static const char *proxy_connection_key = "Proxy-Connection";

static const char *eof_hdr = "\r\n";

static webobj_t *head = NULL;  /* Head of webobj_t linked list */
static int cache_buf_size = 0; /* To monitor size of cache buffer size */

/* Basic idea for sequential proxy
1. Parse GET request
2. Read header from client
3. Extract host name and query path and port from GET request
4. Connect to host and send query path and request header
5. Receive data from server
6. Forward data to client/brower connected
*/

/* Implementation ideas for caching web proxy
1. Each web object need to have a name which can be
   picked up from a query send by client.
2. There can a large buffer of MAX_CACHE_SIZE which would hold
   web cached objects.
3. There can be linked list of meta information about web objects.
   Following information would be required.
   3.1 Web object name (path or query)
   3.2 Offset within main cache buffer
   3.3 Size of web object in bytes
   3.4 Number of times object accessed (for LRU policy)
   3.5 Semaphore mutex
4. Whenever a web object is requested a check is made from 
   metadata linked list. If that object is present in cache.
   If its present read its content from cache and send it to
   client. If not request a web object from server, send it to
   client and store it in cache buffer.
5. Eviction policy: Whenver a cached web object is accessed its
   read count should be incremented. So when cache is full and 
   eviction policy kicks in. It would check web object with least
   read count evicit it from cache and store new object in its place.
6. Each web object can be protected with a samaphore mutex for 
   thread-safe access.

*/
void doit(int fd);
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg);
void make_http_header(rio_t *rp, char *http_hdr_buf, char *host, char *query); 
/* Function to extract host and query path from request header */
int parse_uri(char *src, char *host, char *query, char *port); 
void *server_thread(void *vargp);
/* Helper function for web object management */
void add_web_obj(char *name, char *data_buf); /* Add web object to list */
void evicit_web_obj(); /* Remove a web object from list */
webobj_t *is_web_obj_present(char *wobj); /* Is web object present in list */
void display_web_obj(); /* Function to display cached web object used for debug process */
void increment_read_count(webobj_t *wobjp); /* Increment read count */
void increment_reader_count(webobj_t *wobjp); /* Increment readers count */
void decrement_reader_count(webobj_t *wobjp); /* Decrement readers count */

int main(int argc, char *argv[])
{
    long listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    /* Check command line args */
    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(1);
    }
    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);
	    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
	    
        Pthread_create(&tid, NULL, server_thread, (void *)connfd);
        //doit(connfd);
        //Close(connfd);
    }
    return 0;
}

void *server_thread(void *vargp)
{
    Pthread_detach(Pthread_self());
    long connfd = (long)vargp;
    doit(connfd);
    Close(connfd);
    Pthread_cancel(Pthread_self());
    return NULL;
}

void doit(int fd)
{
    rio_t rio_client, rio_server;
    int serverfd, wobj_size;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE], 
         host[MAXLINE], query[MAXLINE], port[MAXLINE], http_header[MAXLINE],
         cache_buf[MAX_OBJECT_SIZE];
    
    webobj_t *wobjp;

    Rio_readinitb(&rio_client, fd);
    /* Read GET command from client */
    if (!Rio_readlineb(&rio_client, buf, MAXLINE))  //line:netp:doit:readrequest
        return;
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET")) {                     //line:netp:doit:beginrequesterr
        clienterror(fd, method, "501", "Not Implemented",
                    "Proxy does not implement this method");
        return;
    }
    
    if (!parse_uri(uri, host, query, port))
    {
        printf("Proxy error: Invalid URL\n");
        return;
    }
    /*
    printf("host: %s\n", host);
    printf("query: %s\n", query);
    printf("port: %s\n", port);
    */
    /* Read request header from client generate request header to be sent to main server */
    make_http_header(&rio_client, http_header, host, query);
    printf("----http header for main server------\n");
    printf("%s", http_header);
    printf("-------------------------------------\n");
    
    /* Display web object for debug process */
    display_web_obj();

    /* Check for web object if present */
    if ((wobjp = is_web_obj_present(query)) != NULL) {
        printf("Servering request from cache\n");
        increment_read_count(wobjp);
        increment_reader_count(wobjp);
        Rio_writen(fd, wobjp->data_buf, wobjp->size);
        decrement_reader_count(wobjp);
        return;
    }
    /* Establish connect with main server */
    serverfd  = Open_clientfd(host, port);
    if (serverfd < 0) {
        printf("Unable to connect to server: %s on port %s\n", host, port);
        return;
    }
    Rio_readinitb(&rio_server, serverfd);

    /* Write generated http_request header to main server */
    Rio_writen(serverfd, http_header, strlen(http_header));

    size_t n;
    wobj_size = 0;
    while((n=Rio_readlineb(&rio_server,buf,MAXLINE))!=0)
    {
        //printf("proxy received %zd bytes,then send\n",n);
        Rio_writen(fd,buf,n);
        wobj_size += n;
        if (wobj_size < MAX_OBJECT_SIZE)
            strcat(cache_buf, buf);
    }
    
    if (wobj_size < MAX_OBJECT_SIZE) {

        if (cache_buf_size >= MAX_CACHE_SIZE) 
            evicit_web_obj();
        add_web_obj(query, cache_buf);
        cache_buf_size += wobj_size;
    }
    Close(serverfd);
}


/* Function to read http request from client and generate http request 
   to be sent to main server
*/

void make_http_header(rio_t *rp, char *http_hdr_buf, char *host, char *query) 
{
    char buf[MAXLINE], host_hdr[MAXLINE], other_hdr[MAXLINE], get_hdr[MAXLINE];

    while (Rio_readlineb(rp, buf, MAXLINE)>0) {
        if (!strcasecmp(buf, eof_hdr)) { /* End of header reached */
            break;
        }
        
        /* If its a host, user_agent, proxy-connection, connection then skip it
           we will supply our own headers for this fields */
        if (!strncasecmp(buf, host_key, strlen(host_key)) ||
            !strncasecmp(buf, user_agent_key, strlen(user_agent_key)) ||
            !strncasecmp(buf, proxy_connection_key, strlen(proxy_connection_key)) ||
            !strncasecmp(buf, connection_key, strlen(connection_key))) {
                continue;
        }
        /* If some other headers copy them */
        strcat(other_hdr, buf);
    }
    /* Prepare GET header */
    sprintf(get_hdr, "GET %s HTTP/1.0\r\n", query);
    /* Prepare host header */
    sprintf(host_hdr, "Host: %s\r\n", host);
    /* Prepare whole http_header */
    sprintf(http_hdr_buf, "%s%s%s%s%s%s%s",
    get_hdr, host_hdr, other_hdr, user_agent_hdr, connection_hdr, proxy_connection_hdr, eof_hdr);
}

/* Function to extract host, query path and port from src uri */
int parse_uri(char *src_uri, char *host, char *query, char *port)
{
    
    char *temp, *temp1;
    temp = strstr(src_uri, "://"); 
    if (temp == NULL) /* Invalid URL */
        return 0;
    temp+= 3;
    temp1 = strstr(temp, "/");
    
    /* Copy host part */
    if (temp1 == NULL) {
        strcpy(host, temp);
        strcpy(query, "/");
    }
    else
    {
        strncpy(host, temp, temp1-temp);
        /* Copy query part */
        strcpy(query, temp1);
    }

    /* Now extract port from url */
    temp = strstr(host, ":");
    if (temp != NULL)
    {
        /* Found it */
        strcpy(port, temp+1);
        *temp = 0;
    }
    else
    {
        /* Default port */
        strcpy(port, "80");
    }
    
    return 1;
}

void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Proxy Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The csapp proxy server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}

/* Add web object to list */
void add_web_obj(char *name, char *data_buf)
{
    /* Create webobj_t on heap memory */
    int data_len = strlen(data_buf);
    webobj_t *rover = head;
    webobj_t *wobjp = (webobj_t *)Malloc(sizeof(webobj_t));
    /* Allocate space for name and data_buf for a web object */
    wobjp->name = (char *)Malloc(strlen(name));
    wobjp->data_buf = (char *)Malloc(data_len);
    /* Copy data to allocated buffers */
    strcpy(wobjp->name, name);
    strcpy(wobjp->data_buf, data_buf);
    
    /*Set the size of data_buf */
    wobjp->size = data_len;

    /* Set initial read count for LRU policy */
    wobjp->rcount = 1;

    /* Set nex pointer to NULL */
    wobjp->next = NULL;

    /* Initalize mutex */
    Sem_init(&wobjp->mutex, 0, 1);

    /* Initalize number of active readers */
    Sem_init(&wobjp->num_reader, 0, 1);

    /* Add web object to end of list */
    if (rover == NULL) { /* Its a first element in list */
        head = wobjp;
        return;
    }
    printf("add_web_obj: start of while loop\n");
    while (rover->next != NULL) {
        rover = rover->next;
    }
    printf("add_web_obj: end of while loop\n");
    rover->next = wobjp; 
}

/* Is web object present in list */
webobj_t *is_web_obj_present(char *name) 
{
    webobj_t *rover = head;
    
    /* Iterate over whole list */
    while (rover) {
        if (!strcasecmp(name, rover->name))
            return rover;
        rover = rover->next;
    }

    /* Not present any matching web object */
    return NULL;
}

/* Display list of all cached web objects for debug purpose */
void display_web_obj()
{
    webobj_t *rover = head;
    printf("----List of cached web objects----\n");
    while (rover) {
        printf("+++++++\n");
        printf("Name: %s\n", rover->name);
        printf("Size: %d\n", rover->size);
        printf("+++++++\n");
        rover = rover->next;
    }
    printf("Total size of cache: %d\n", cache_buf_size);
    printf("--------------\n");
}

/* Function to increment read count of object */
void increment_read_count(webobj_t *wobjp)
{
    P(&wobjp->mutex);
    wobjp->rcount++;
    V(&wobjp->mutex);
}

/* Function to increment current reader count of web object */
void increment_reader_count(webobj_t *wobjp)
{
    P(&wobjp->num_reader);
}

/* Function to decrement current reader count of web object */
void decrement_reader_count(webobj_t *wobjp)
{
    V(&wobjp->num_reader);
}

/* Function to evicit a web object */
void evicit_web_obj()
{
    webobj_t *victim, *rover;
    /* Set rover and vicitim to head of list */
    victim = rover = head;
    int mincount;

    while (rover) {
        rover->rcount;

    }

}