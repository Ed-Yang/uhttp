#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <winsock.h>
//#include <WS2tcpip.h> // socklen_t
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>


#define closesocket close

#endif

#define MAX_BUF_SIZE 1024
#define NAME_LEN 64

#ifdef WIN32
#define LOCAL_FOLDER ".\\json\\" // the mapped folder
#else
#define LOCAL_FOLDER "./json/" // the mapped folder
#endif

#ifdef WIN32
#define METHOD_FOLDER ".\\web-data\\" // the mapped folder
#else
#define METHOD_FOLDER "./web-data/" // the mapped folder
#endif


#define STATUS_200 "HTTP/1.0 200 OK"
#define STATUS_404 "HTTP/1.0 404 NOT FOUND"

#define AGENT_STR "Server: small JSON web server"

#define CONTENT_JSON "Content-type: application/json"

typedef struct header_info_t
{
    char action[NAME_LEN];
    char fpath[NAME_LEN];
    char method[NAME_LEN];
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
    int remain_size = MAX_BUF_SIZE;

    if (status)
    {
        n += snprintf(buf, remain_size, "%s\r\n", status);
        remain_size -= n;
    }

    if (agent)
    {
        n += snprintf(&buf[n], remain_size, "%s\r\n", agent);
        remain_size -= n;
    }

    if (date)
    {
        n += snprintf(&buf[n], remain_size, "%s\r\n", date);
    }

if (ctype)
{
    n += snprintf(&buf[n], remain_size, "%s\r\n", ctype);
    remain_size -= n;
}
// end of header
n += snprintf(&buf[n], remain_size, "\r\n");

return n;
}

int recv_http_message(int so, char *rbuf, int rbuf_len, int *content_len)
{
    char *s, *p;
    int found = FALSE;
    int hlen, cont_len, rlen;
    int i, n;

    // FIXME: recv handling
    memset(rbuf, 0, rbuf_len);
    rlen = 0;
    while ((n = recv(so, &rbuf[rlen], MAX_BUF_SIZE - rlen, 0)) > 0)
    {
        if (!found && n < 4) continue;

        // find delimeter
        p = &rbuf[rlen];
        rlen += n;

        for (i = 0; i < n - 3; i++, p++)
        {
            if (*p == '\r' && *(p + 1) == '\n' && *(p + 2) == '\r' && *(p + 3) == '\n')
            {
                found = TRUE;
                hlen = (int)(p - &rbuf[0]) + 4;
                break;
            }
        }

        if (found)
            break; // while
    }

    // find Content-length
    *content_len = 0;
    if (content_len != NULL && (s = strstr(rbuf, "Content-Length: ")) != NULL)
    {
        s = s + strlen("Content-Length: ");
        cont_len = atoi(s);
        *content_len = cont_len;

        // check if all data is received
        if (rlen < (hlen + cont_len))
        {
            n = (hlen + cont_len) - rlen;
            if ((n = recv(so, &rbuf[rlen], n, 0)) > 0)
                rlen += n;
            else
                rlen = -1;
        }
    }

    return rlen;
}

int parse_header(HEADER_INFO_T *header, char *buf, int buf_len)
{
    char *h, *p;
    int len;
    int remaining = 0;

    memset(header, 0, sizeof(HEADER_INFO_T));

    // GET or POST
    h = buf;
    p = strchr(h, (int)' ');
    //*p = 0;

    if (h[0])
    {
        len = (int)(p-h) > NAME_LEN ? NAME_LEN - 1 : (int)(p - h);
        memcpy(header->action, h, len);
    }

    // File
    h = p + 1;
    p = strchr(h, (int)' ');
    //*p = 0;

    if (*h == '/')
        h += 1; // skip first '/'

    if (h[0])
    {
        len = (int)(p - h) > NAME_LEN ? NAME_LEN - 1 : (int)(p - h);

        memcpy(header->fpath, LOCAL_FOLDER, strlen(LOCAL_FOLDER));
        memcpy(&header->fpath[strlen(LOCAL_FOLDER)], h, len);
    }

    // Method
    if ((h=strstr(buf, "\"method\":")) != NULL)
    {
        h += strlen("\"method\":");
        h++; // skip '"'
        if ((p = strchr(h, '"')) != NULL)
        {
            len = (int)(p - h) > NAME_LEN ? NAME_LEN - 1 : (int)(p - h);

            memcpy(header->method, METHOD_FOLDER, strlen(METHOD_FOLDER));
            memcpy(&header->method[strlen(METHOD_FOLDER)], h, len);
            // append .json
            strcat(header->method, ".json");
        }
    }

    return 0;
}


void proc_req(int so)
{
    int n, len;
    HEADER_INFO_T h_line;
    char rbuf[MAX_BUF_SIZE], sbuf[MAX_BUF_SIZE];
    FILE *fp = NULL;
    //char full_path[NAME_LEN * 2];
    char *status;
    int rlen, clen;    

    rlen = recv_http_message(so, rbuf, MAX_BUF_SIZE, &clen);
    if (rlen < 0)
        return;

    memset(&h_line, 0, sizeof(h_line));
    parse_header(&h_line, rbuf, rlen);

    // try method and file

    if (h_line.method[0] == NULL || (fp = fopen(h_line.method, "rb")) == NULL)
    {
        if (h_line.fpath[0] != NULL)
            fp = fopen(h_line.fpath, "rb");
    }

    fprintf(stderr, "ACTION: %s, PATH: %s\n", h_line.action, h_line.fpath);

    if (clen > 0)
    {
        fprintf(stderr, "DATA: -----------------------\n");
        fprintf(stderr, "%s\n", &rbuf[rlen-clen]);
    }

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
    int sock, so, rv;
    struct sockaddr_in addr;
    int fromlen = sizeof(addr);

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
#ifdef WIN32
    WSADATA wsaData;
    WORD wVersionRequested;
    int err;

    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
#endif

    if (argc != 2)
    {
        usage(argv[0]);
        exit(0);
    }

    run_server(atoi(argv[1]));

    return (0);
}
