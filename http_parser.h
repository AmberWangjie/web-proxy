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

enum{
  FLAG_NONE = 0, //normal data stream, no need to decode
  READ_CLIENT_DECODE = 1, //receive the data from client to decode
  WRITE_SERVER_ENCODE = 2 //encode before sending data to server
};

static int io_flag; //
static int m_pid; //保存主进程id

void rewrite_header();
void forward_header(int);
int send_data(int, char*, int);
int receive_data(int, char*, int);
void handle_client(int, struct sockaddr_in);
ssize_t read_line_to_buf(int, void*, size_t);
void hand_proxy_info_req(int, char*);
void get_info(char*);
int read_request_header(int fd,void* buffer);
void response_header_fields(int, const char*);
void not_found(int);
void serve_file(int, const char*);
void unimplement(int);
void get_server_path(char*, char*);
void get_host(char*);
void accept_request(int);
void bad_request(int);
void cat(int, FILE *);
void cannot_execute(int);
void error_die(const char *);
void execute_cgi(int, const char *, const char *, const char *);
