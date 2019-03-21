//#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *connection_hdr = "Connection: close\r\n";
static const char *proxy_connection_hdr = "Proxy-Connection: close\r\n";
static const char *user_agent_key = "User-Agent";
static const char *host_key = "Host";
static const char *connection_key = "Connection";
static const char *proxy_connection_key = "Proxy-Connection";

static const char *eof_hdr = "\r\n";

/* Basic idea for sequential proxy
1. Parse GET request
2. Read header from client
3. Extract host name and query path and port from GET request
4. Connect to host and send query path and request header
5. Receive data from server
6. Forward data to client/brower connected
*/
void doit(int fd);
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg);
void make_http_header(rio_t *rp, char *http_hdr_buf, char *host, char *query); 
/* Function to extract host and query path from request header */
int parse_uri(char *src, char *host, char *query, char *port); 
void *server_thread(void *vargp);
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
    int serverfd;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE], 
         host[MAXLINE], query[MAXLINE], port[MAXLINE], http_header[MAXLINE];

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
    while((n=Rio_readlineb(&rio_server,buf,MAXLINE))!=0)
    {
        printf("proxy received %zd bytes,then send\n",n);
        Rio_writen(fd,buf,n);
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