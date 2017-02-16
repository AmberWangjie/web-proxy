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
#include <time.h>

#define IS_SPACE(x) isspace((int)(x))

typedef struct req_info_t{
  char method[10];
  char prtc[10];
  char* host;
  int port;
  char* c_url;
  char* p_url;
  char* header;
}req_info;

typedef struct rsp_info_t{
  int code;
  char* status;
  char* cache;
  char* date;
  char* expire;
  char* length;
  char* etag;
}rsp_info;

void* memmem(const void*, size_t, const void*, size_t);
char* parse_request(char* buff, req_info* tokens);
rsp_info* parse_response(char* buffer);
