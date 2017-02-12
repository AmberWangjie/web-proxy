#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>

#define IS_SPACE(x) isspace((int)(x))

#define BUF_SIZE  8192
#define MAX_HEADER_SIZE  8192


#define SERVER_SOCKET_ERROR  -1
#define SERVER_SETSOCKOPT_ERROR -2
#define SERVER_BIND_ERROR -3
#define SERVER_LISTEN_ERROR -4
#define CLIENT_SOCKET_ERROR -5
#define CLIENT_RESOLVE_ERROR -6
#define CLIENT_CONNECT_ERROR -7
#define HEADER_BUFFER_FULL -8
#define BAD_HTTP_PROTOCOL -9


typedef struct req_info_t{
  char method[10];
  char prtc[10];
  char host[512];
  int port;
  char c_url[512];
  char p_url[512];
}req_info;


typedef struct rsp_info_t{
  char status[10];
  char message[512];
}rsp_info;

req_info* parse_request(char*);
void rewrite_request(req_info*);
rsp_info parse_response(char*);
