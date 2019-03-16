//#include <stdio.h>
#include "csapp.h"

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
void doit(int fd);
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg);
void rw_requesthdrs(rio_t *rp, int clientfd);
void read_responsehdrs(rio_t *rp);
/* Function to extract host and query path from request header */
int host_query(char *src, char *host, char *query, char *port); 
int main(int argc, char *argv[])
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

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
	    doit(connfd);
        Close(connfd);                                            //line:netp:tiny:close
    }
    printf("%s", user_agent_hdr);
    return 0;
}

void doit(int fd)
{
    rio_t rio, rio_server;
    int clientfd;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE], 
         host[MAXLINE], query[MAXLINE], port[MAXLINE], response[MAXBUF];

    Rio_readinitb(&rio, fd);
    if (!Rio_readlineb(&rio, buf, MAXLINE))  //line:netp:doit:readrequest
        return;
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET")) {                     //line:netp:doit:beginrequesterr
        clienterror(fd, method, "501", "Not Implemented",
                    "Proxy does not implement this method");
        return;
    }
    if (!host_query(uri, host, query, port))
    {
        printf("Proxy error: Invalid URL\n");
        return;
    }
    /* Establish connect with main server */
    
    printf("host: %s\n", host);
    printf("query: %s\n", query);
    printf("port: %s\n", port);
    printf("Before Open_clientfd\n");
    
    clientfd  = Open_clientfd(host, port);
    //clientfd  = Open_clientfd("localhost", "2100");
    printf("After Open_clientfd\n");
    if (clientfd > -1) {
        printf("Connect established with host: %s at port: %s\n", host, port);
        Rio_readinitb(&rio_server, clientfd);
    }
    else {
        printf("Connect not established with host: %s at port: %s\n", host, port);
        return;
    }
    
    /* Write GET request to tiny server */
    sprintf(buf, "GET %s HTTP/1.0", query);
    //sprintf(buf, "GET / HTTP/1.0");
    printf("buf=%s", buf);
    Rio_writen(clientfd, buf, strlen(buf));
    rw_requesthdrs(&rio, clientfd);
    read_responsehdrs(&rio_server);
    //Rio_readnb(clientfd, response, sizeof(response));
    //printf("Response: %s\n", response);
    //Rio_writen(fd, response, sizeof(response));
    


}
void rw_requesthdrs(rio_t *rp, int clientfd) 
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
    //Rio_writen(clientfd, buf, strlen(buf));
    while(strcmp(buf, "\r\n")) {          //line:netp:readhdrs:checkterm
	    Rio_readlineb(rp, buf, MAXLINE);
	    printf("%s", buf);
        //Rio_writen(clientfd, buf, strlen(buf));
    }
    return;
}

void read_responsehdrs(rio_t *rp)
{
    char buf[MAXLINE];
    printf("-----Response from server-------\n");

    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
    while(strcmp(buf, "\r\n")) {          //line:netp:readhdrs:checkterm
	    Rio_readlineb(rp, buf, MAXLINE);
	    printf("%s", buf);
    }

}

int host_query(char *src, char *host, char *query, char *port)
{
    
    char *temp, *temp1;
    temp = strstr(src, "://"); 
    if (temp == NULL) /* Invalid URL */
        return 0;
    temp+= 3;
    temp1 = strstr(temp, "/");
    
    /* Copy host part */
    strncpy(host, temp, temp1-temp);
    /* Copy query part */
    strcpy(query, temp1);
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
    
    //printf("src: %s\n", src);
    //printf("host: %s\n", host);
    //printf("query: %s\n", query);
    //printf("port: %s\n", port);
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
