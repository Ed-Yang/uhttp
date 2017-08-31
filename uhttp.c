#include <memory.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_BUF_SIZE 1024
#define NAME_LEN 64
#define LOCAL_FOLDER "./json/" // the mapped folder

#ifndef WIN32
#define closesocket close
#endif

#define STATUS_200 "HTTP/1.0 200 OK"
#define STATUS_404 "HTTP/1.0 404 NOT FOUND"

#define AGENT_STR "Server: small JSON web server"

#define CONTENT_JSON "Content-type: application/json"

typedef struct header_info_t
{
    char action[NAME_LEN];
    char fpath[NAME_LEN];
    // char date[NAME_LEN];
} HEADER_INFO_T;

int sock_create(uint16_t port)
{
    int sockfd, rv;
    int optval;
    struct sockaddr_in addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror("sock_create: socket");
        return -1;
    }

    optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&optval,
                   sizeof(optval)) < 0)
    {
        perror("sock_create: setsockopt reuse");
        closesocket(sockfd);
        return -1;
    }

    /* bind local address */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = 0;
    addr.sin_port = ntohs(port);

    if ((rv = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr))) < 0)
    {
        perror("\nbind");
        closesocket(sockfd);
        return -1;
    }

    return sockfd;
}

int fill_resp_header(char *buf, char *status, char *agent, char *date,
                     char *ctype)
{
    int n = 0;

    if (status)
        n += sprintf(buf, "%s\r\n", status);

    if (agent)
        n += sprintf(&buf[n], "%s\r\n", agent);

    if (date)
        n += sprintf(&buf[n], "%s\r\n", date);

    if (ctype)
        n += sprintf(&buf[n], "%s\r\n", ctype);

    // end of header
    n += sprintf(&buf[n], "\r\n");

    return n;
}

int parse_header(HEADER_INFO_T *header, char *buf)
{
    char *h, *p;

    // GET or POST
    h = buf;
    p = strchr(h, (int)' ');
    *p = 0;

    if (h[0])
        strcpy(header->action, h);

    // File
    h = p + 1;
    p = strchr(h, (int)' ');
    *p = 0;

    if (*h == '/') h += 1; // skip first '/'

    if (h[0])
        strcpy(header->fpath, h);

    return 0;
}

void proc_req(int so)
{
    int n, len, plen;
    HEADER_INFO_T h_line;
    char rbuf[MAX_BUF_SIZE], sbuf[MAX_BUF_SIZE];
    FILE *fp = NULL;
    char full_path[NAME_LEN * 2];
    char *status;

    // FIXME: recv handling
    n = recv(so, rbuf, MAX_BUF_SIZE, 0);

    memset(&h_line, 0, sizeof(h_line));
    parse_header(&h_line, rbuf);

    memset(full_path, 0, sizeof(full_path));

    if (h_line.fpath[0] != 0  && strcmp(h_line.fpath, "/") != 0)
    {
        plen = strlen(LOCAL_FOLDER);
        memcpy(full_path, LOCAL_FOLDER, plen);
        memcpy(&full_path[plen], h_line.fpath, strlen(h_line.fpath));

        fp = fopen(full_path, "rb");
    }

    fprintf(stderr, "ACTION: %s, PATH: %s: %s\n", h_line.action, h_line.fpath,
            full_path);

    if (fp != NULL)
    {
        status = STATUS_200;
        len = fill_resp_header(sbuf, status, AGENT_STR, NULL, CONTENT_JSON);
        n = send(so, sbuf, len, 0);

        while ((len = fread(sbuf, 1, sizeof(sbuf), fp)) > 0)
        {
            n = send(so, sbuf, len, 0);
        }

        fclose(fp);
    }
    else
    {
        status = STATUS_404;
        len = fill_resp_header(sbuf, status, AGENT_STR, NULL, NULL);
        n = send(so, sbuf, len, 0);
    }

    fprintf(stderr, "STATUS: %s\n", status);

    closesocket(so);
}

void run_server(uint16_t port)
{
    fd_set readfds;
    struct timeval tv;
    int sock, so, n, rv;
    struct sockaddr_in addr;
    socklen_t fromlen = sizeof(addr);

    if ((sock = sock_create(port)) < 0)
    {
        fprintf(stderr, "run_server: sock_create failed !!\n");
        return;
    }

    listen(sock, 5);

    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);

    fprintf(stderr, "server is running on %d\n", port);

    while ((rv = select(FD_SETSIZE, &readfds, NULL, NULL, NULL)) > 0)
    {
        // check socket
        if (FD_ISSET(sock, &readfds))
        {
            so = accept(sock, (struct sockaddr *)&addr, &fromlen);
            proc_req(so);

        } /* if */
    }

    fprintf(stderr, "server stopped\n");
}

void usage(char *s)
{
    fprintf(stderr, "usage: %s <port>\n", s);

    return;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        usage(argv[0]);
        exit(0);
    }

    run_server(atoi(argv[1]));

    return (0);
}
