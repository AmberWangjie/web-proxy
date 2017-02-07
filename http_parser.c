#include "http_parer.h"
int send_data(int socket,char * buffer,int len)
{

  if(io_flag == W_S_ENC)
    {
      int i;
      for(i = 0; i < len ; i++)
	{
	  buffer[i] ^= 1;

	}
    }

  return send(socket,buffer,len,0);
}

int receive_data(int socket, char * buffer, int len)
{
  int n = recv(socket, buffer, len, 0);
  if(io_flag == R_C_DEC && n > 0)
    {
      int i;
      for(i = 0; i< n; i++ )
	{
	  buffer[i] ^= 1;
	  // printf("%d => %d\n",c,buffer[i]);
	}
    }

  return n;
}



ssize_t read_line_to_buf(int socket, void* buffer, size_t n){
  ssize_t numRead;
  size_t totRead;
  char *buf;
  char ch;
  
  if (n <= 0 || buffer == NULL) {
    errno = EINVAL; //invalid argument 
    return -1;
  }
  
  buf = buffer;
  totRead = 0;
  while(1) {
    numRead = receive_data(fd, &ch, 1);
    if (numRead == -1) {
      if (errno == EINTR){  /* interuppted function call */
	continue;
      } else{
	return -1;
      }              /* unknown err */
    } else if (numRead == 0) {      /* EOF */
      if (totRead == 0){           /* No bytes read; return 0 */
	return 0;
      } else{                        /* Some bytes read; add '\0' */
	break;
      }
    } else {
      if (totRead < n - 1) {      /* Discard > (n - 1) bytes */
	totRead++;
	*buf++ = ch;
      }
      if (ch == '\n'){
	break;
      }
    }
  }
  *buf = '\0';
  return totRead;
}

/*read the request header from client, save to buffer*/
int read_request_header(int client_sock, void* buffer){
  memset(header_buffer,0,MAX_HEADER_SIZE);
  char line_buffer[2048];
  char * base_ptr = header_buffer;
  while(1){
    memset(line_buffer,0,2048); //fill the memory with 0 
    ssize_t total_read =  read_line_to_buf(client_sock,line_buffer,2048);
    if(total_read <= 0){
      return CLIENT_SOCKET_ERROR;
    }
    //In case the header buffer overflow
    if(base_ptr + total_read - header_buffer <= MAX_HEADER_SIZE){
      strncpy(base_ptr,line_buffer,total_read);
      base_ptr += total_read;
    } else {
      return HEADER_BUFFER_FULL;
    }
    //read CRLF, header ends
    if(strcmp(line_buffer,"\r\n") == 0 || strcmp(line_buffer,"\n") == 0){
      break;
    }
  }
  return 0;
} 

/*The url needs to be modified to the form of path */
void rewrite_header(){
  char * p = strstr(header_buffer,"http://");
  char * p0 = strchr(p,'\0');
  char * p5 = strstr(header_buffer,"HTTP/"); 
  int len = strlen(header_buffer);
  if(p){
    char * p1 = strchr(p + 7,'/');
    if(p1 && (p5 > p1)){
      //convert url to path
      memcpy(p,p1,(int)(p0 -p1));
      int l = len - (p1 - p) ;
      header_buffer[l] = '\0';
    }else{
      char * p2 = strchr(p,' ');  //GET http://3g.sina.com.cn HTTP/1.1
      // printf("%s\n",p2);
      memcpy(p + 1,p2,(int)(p0-p2));
      *p = '/';  //url has no path, use root dir
      int l  = len - (p2  - p ) + 1;
      header_buffer[l] = '\0';
     }
  }
}

/* Return the additional information about the server-response message. */
/* Parameters: the socket to print the headers on
 *             the name of the file */
void resp_header_fields(int client, const char *filename){
  char buf[2048];
  (void)filename;  /* could use filename to determine file type */
  /*valid HTTP header */
  strcpy(buf, "HTTP/1.0 200 OK\r\n");
  send(client, buf, strlen(buf), 0);
  /*server info*/
  strcpy(buf, SERVER_STRING);  //how to get the server name, in any form?
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "Content-Type: text/html\r\n");
  send(client, buf, strlen(buf), 0);
  strcpy(buf, "\r\n");
  send(client, buf, strlen(buf), 0);
}

void not_found(int client)
{
  char buf[1024];

  /* 404 not found*/
  sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
  send(client, buf, strlen(buf), 0);
  /*server information*/
  sprintf(buf, SERVER_STRING); //同上
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "Content-Type: text/html\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "your request because the resource specified\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "is unavailable or nonexistent.\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "</BODY></HTML>\r\n");
  send(client, buf, strlen(buf), 0);
}

/* Send a regular file to the client.  Use headers, and report
 * errors to client once occur.
 * Parameters: a pointer to a file structure produced from the socket file descriptor， the name of the file to serve */

void serve_file(int client, const char *filename)
{
  FILE *resource = NULL;
  int numchars = 1;
  char buf[1024];

  /*read and discard header */
  buf[0] = 'A'; buf[1] = '\0';
  while ((numchars > 0) && strcmp("\n", buf)){  /* read & discard headers */
    numchars = get_line(client, buf, sizeof(buf));
  }
  /*open the server file*/
  resource = fopen(filename, "r");
  if (resource == NULL){
    not_found(client);
  } else {
    /*write HTTP header */
    headers(client, filename);
    /*copy*/
    cat(client, resource);
  }
  fclose(resource);
}

/* Inform the client that the requested web method fail to be  implemented.
 * Parameter: the client socket */
void unimplemented(int client){
  char buf[1024];
  /* HTTP method unsupported*/
  sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
  send(client, buf, strlen(buf), 0);
  /*server information*/
  sprintf(buf, SERVER_STRING);
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "Content-Type: text/html\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "</TITLE></HEAD>\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "</BODY></HTML>\r\n");
  send(client, buf, strlen(buf), 0);
}

void get_server_path(char* header,char* output){
  char * p = strstr(header,"GET /");
  if(p) {
    char * p1 = strchr(p+4,' ');
    strncpy(output,p+4,(int)(p1  - p - 4) );
  }
}

/* In CONNECT method,parse the host name and port number*/
void get_host(char* header, char* output){
  char * p = strstr(header,"CONNECT");  
  if(p){
    char * p1 = strchr(p,' ');
    char * p2 = strchr(p1 + 1,':');
    char * p3 = strchr(p1 + 1,' ');
    if(p2){
      char s_port[10];
      bzero(s_port,10); //fill with 0
      strncpy(remote_host,p1+1,(int)(p2  - p1) - 1);
      strncpy(s_port,p2+1,(int) (p3 - p2) -1);
      remote_port = atoi(s_port);
    } else{
      strncpy(remote_host,p1+1,(int)(p3  - p1) -1);
      remote_port = 80;
    }
    return 0;
  }
  char * p = strstr(header,"Host:");
  if(!p){
    return BAD_HTTP_PROTOCOL;
  }
  char * p1 = strchr(p,'\n');
  if(!p1){
    return BAD_HTTP_PROTOCOL;
  }
  char * p2 = strchr(p + 5,':'); /* 5:length of 'Host:'*/
  if(p2 && p2 < p1){
    int p_len = (int)(p1 - p2 -1);
    char s_port[p_len];
    strncpy(s_port,p2+1,p_len);
    s_port[p_len] = '\0';
    remote_port = atoi(s_port);
    int h_len = (int)(p2 - p -5 -1 );
    strncpy(remote_host,p + 5 + 1  ,h_len); //Host:
    //assert h_len < 128;
    remote_host[h_len] = '\0';
  }else{
    int h_len = (int)(p1 - p - 5 -1 -1);
    strncpy(remote_host,p + 5 + 1,h_len);
    //assert h_len < 128;
    remote_host[h_len] = '\0';
    remote_port = 80;
  }
  return 0;
}

/* A request has caused a call to accept() on the server port to
 * return.  Process the request appropriately.
 * Parameters: the socket connected to the client */
void accept_request(int client){
  char buf[2048];
  ssize_t numchars;
  char method[255];
  char url[255];
  char path[512];
  size_t i;
  size_t j;
  struct stat st;
  int cgi = 0;  /* becomes true if server decides this is a CGI program */
  char* query_string = NULL;
  /*get the first line of the request*/
  numchars = read_line_to_buf(client, buf, sizeof(buf));
  i = 0;
  j = 0;
  /*save the method from client to method array*/
  while (!IS_SPACE(buf[j]) && (i < sizeof(method) - 1)){
    method[i] = buf[j];
    i++;
    j++;
  }
  method[i] = '\0';
  /*If it is not one of the three method, cannot implement */
  if (strcasecmp(method, "GET") && strcasecmp(method, "POST") && strcasecmp(method, "CONNECT")) {
    unimplemented(client);
    return;
  }
  /* POST: start cgi */
  if (strcasecmp(method, "POST") == 0){
    cgi = 1;
  }
  /*read url address*/
  i = 0;
  while (IS_SPACE(buf[j]) && (j < sizeof(buf))){
    j++;
  }
  while (!IS_SPACE(buf[j]) && (i < sizeof(url) - 1) && (j < sizeof(buf))){
    /*save the url */
    url[i] = buf[j];
    i++;
    j++;
  }
  url[i] = '\0';
  /*handle the GET method*/
  if (strcasecmp(method, "GET") == 0){
    /* the request to be handled is url */
    query_string = url;
    while ((*query_string != '?') && (*query_string != '\0')){
      query_string++;
    }
    /* query string is after the "?", need the cgi program to handle */
    if (*query_string == '?'){
      /*start cgi */
      cgi = 1;
      *query_string = '\0';
      query_string++;
    }
  }
  /*formalize url to path array，html file is in the htdocs */
  sprintf(path, "htdocs%s", url);
  /*default: index.html */
  if (path[strlen(path) - 1] == '/'){
    strcat(path, "index.html");
  }
  /*find the file by path */
  if (stat(path, &st) == -1) {
    /*discard all  headers info*/
    while ((numchars > 0) && strcmp("\n", buf)){  /* read & discard headers */
      numchars = get_line(client, buf, sizeof(buf));
    }
    /*report to the client*/
    not_found(client);
  } else {
    /*If the file is a dir, default is index.html file*/
    if ((st.st_mode & S_IFMT) == S_IFDIR){
      strcat(path, "/index.html");
    }
    if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH)){
      cgi = 1;
    }
    /*not cgi,return the server file,otherwise, cgi */
    if (!cgi){
      serve_file(client, path);
    } else{
      execute_cgi(client, path, method, query_string);
    }
  }
  /*HTTP: no constant connection, so close here*/
  close(client);
}

/* Inform the client that a request it has made has a problem.
 * Parameters: client socket */
void bad_request(int client){
  char buf[1024];
  /*reponse to the invalid request from client */
  sprintf(buf, "HTTP/1.0 400 BAD REQUEST\r\n");
  send(client, buf, sizeof(buf), 0);
  sprintf(buf, "Content-type: text/html\r\n");
  send(client, buf, sizeof(buf), 0);
  sprintf(buf, "\r\n");
  send(client, buf, sizeof(buf), 0);
  sprintf(buf, "<P>Your browser sent a bad request, ");
  send(client, buf, sizeof(buf), 0);
  sprintf(buf, "such as a POST without a Content-Length.\r\n");
  send(client, buf, sizeof(buf), 0);
}

/* Put the entire contents of a file out on a socket. 
 * Parameters: the client socket descriptor
 *             FILE pointer for the file to cat */
void cat(int client_sock, FILE *resource)
{
  char buf[1024];

  /*read the file and write to  socket */
  fgets(buf, sizeof(buf), resource);
  while (!feof(resource))
    {
      send(client_sock, buf, strlen(buf), 0);
      fgets(buf, sizeof(buf), resource);
    }
}

/* Inform the client that a CGI script could not be executed.
 * Parameter: the client socket descriptor. */
void cannot_execute(int client_sock)
{
  char buf[1024];

  /* reponse to client that cgi not executable*/
  sprintf(buf, "HTTP/1.0 500 Internal Server Error\r\n");
  send(client_buf, buf, strlen(buf), 0);
  sprintf(buf, "Content-type: text/html\r\n");
  send(client_buf, buf, strlen(buf), 0);
  sprintf(buf, "\r\n");
  send(client_buf, buf, strlen(buf), 0);
  sprintf(buf, "<P>Error prohibited CGI execution.\r\n");
  send(client_buf, buf, strlen(buf), 0);
}

/* Print out an error message with perror() (for system errors; based  on value of errno, which indicates system call errors) and exit the program indicating an error. */
void error_die(const char *sc)
{
  /*handle the err */
  perror(sc);
  exit(1);
}

/* Execute a CGI script.  Will need to set environment variables as
 * appropriate.
 * Parameters: client socket descriptor
 *             path to the CGI script */
/**********************************************************************/
void execute_cgi(int client_sock, const char *path, const char *method, const char *query_string)
{
  char buf[1024];
  int cgi_output[2];
  int cgi_input[2];
  pid_t pid;
  int status;
  int i;
  char c;
  int numchars = 1;
  int content_length = -1;

  buf[0] = 'A'; buf[1] = '\0';
  if (strcasecmp(method, "GET") == 0){
    while ((numchars > 0) && strcmp("\n", buf)){  /* read & discard headers */
      numchars = read_line_to_buf(client_sock, buf, sizeof(buf));
    }
  }
  else if(strcasecmp(method, "POST") == 0){
    /*  get the content_length from HTTP request of POST*/
    numchars = read_line_to_buf(client_sock, buf, sizeof(buf));
    while ((numchars > 0) && strcmp("\n", buf)){
      /*use \0 to seperate */
      buf[15] = '\0';
      if (strcasecmp(buf, "Content-Length:") == 0){
	content_length = atoi(&(buf[16]));
      }
      numchars = read_line_to_buf(client_sock, buf, sizeof(buf));
    }
    /*fail to find content_length */
    if (content_length == -1) {
      /*invalid request*/
      bad_request(client_buf);
      return;
    }
  }
  /*correct, status code is 200 */
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  send(client_sock, buf, strlen(buf), 0);
}

/* */
int send_connect_ok(int client_sock){
  char * resp = "HTTP/1.1 200 Connection Established\r\n\r\n";
  int len = strlen(resp);
  char buffer[len+1];
  strcpy(buffer,resp);
  if(send_data(client_sock,buffer,len) < 0){
    perror("Send http tunnel response  failed\n");
    return -1;
  }
  return 0;
}


//return the basic info of a response
void hand_proxy_info_resp(int server_sock, char * header) {
  char server_path[255] ;
  char response[8192];
  extract_server_path(header,server_path);

  // LOG("server path:%s\n",server_path);
  char info_buf[1024];
  get_info(info_buf);
  sprintf(response,"HTTP/1.0 200 OK\nServer: webproxy/0.1\n\
                    Content-type: text/html; charset=utf-8\n\n\
                     <html><body>\
                     <pre>%s</pre>\
                     </body></html>\n",info_buf);
  write(server_sock,response,strlen(response));
}

/* /\* get the basic execution info and output to the buffer *\/ */
/* void get_info(char * output){ */
/*   int pos = 0; */
/*   char line_buffer[512]; */
/*   sprintf(line_buffer,"======= webproxy (v0.1) ========\n"); */
/*   int len = strlen(line_buffer); */
/*   memcpy(output,line_buffer,len); */
/*   pos += len ; */
/*   sprintf(line_buffer,"%s\n",get_work_mode()); */
/*   len = strlen(line_buffer); */
/*   memcpy(output + pos,line_buffer,len); */
/*   pos += len; */
/*   if(strlen(remote_host) > 0){ */
/*     sprintf(line_buffer,"start server on %d and next hop is %s:%d\n",local_port,remote_host,remote_port); */
/*   } else{ */
/*     sprintf(line_buffer,"start server on %d\n",local_port); */
/*   } */
/*   len = strlen(line_buffer); */
/*   memcpy(output+ pos,line_buffer,len); */
/*   pos += len ; */
/*   output[pos] = '\0'; */
/* } */

void usage(){
  printf("Usage:\n");
  printf(" -l <port number>  specifyed local listen port \n");
  printf(" -h <remote server and port> specifyed next hop server name\n");
  printf(" -d <remote server and port> run as daemon\n");
  printf("-E encode data when forwarding data\n");
  printf ("-D decode data when receiving data\n");
  exit (8);
}

nt _main(int argc, char *argv[])
{
  local_port = DEFAULT_LOCAL_PORT;
  io_flag = FLG_NONE;
  int daemon = 0;

  char info_buf[2048];

  int opt;
  char optstrs[] = ":l:h:dED";
  char *p = NULL;
  while(-1 != (opt = getopt(argc, argv, optstrs)))
    {
      switch(opt)
	{
	case 'l':
	  local_port = atoi(optarg);
	  break;
	case 'h':
	  p = strchr(optarg, ':');
	  if(p)
	    {
	      strncpy(remote_host, optarg, p - optarg);
	      remote_port = atoi(p+1);
	    }
	  else
	    {
	      strncpy(remote_host, optarg, strlen(remote_host));
	    }
	  break;
	case 'd':
	  daemon = 1;
	  break;
	case 'E':
	  io_flag = W_S_ENC;
	  break;
	case 'D':
	  io_flag = R_C_DEC;
	  break;
	case ':':
	  printf("\nMissing argument after: -%c\n", optopt);
	  usage();
	case '?':
	  printf("\nInvalid argument: %c\n", optopt);
	default:
	  usage();
	}
    }

  get_info(info_buf);
  printf("%s\n",info_buf);
  start_server(daemon);
  return 0;

}

/* Handle the connection from client */
void handle_client(int client_sock, struct sockaddr_in client_addr)
{
  int is_http_tunnel = 0;
  if(strlen(remote_host) == 0) /* 未指定远端主机名称,从http 请求 HOST 字段中获取 */
    {

#ifdef DEBUG
      printf(" ============ handle new client ============\n");
      prinf(">>>Header:%s\n",header_buffer);
#endif

      if(read_header(client_sock,header_buffer) < 0){
	prinf("Read Http header failed\n");
	return;
      } else{
	char * p = strstr(header_buffer,"CONNECT"); /* 判断是否是http 隧道请求 */
	if(p)
	  {
	    LOG("receive CONNECT request\n");
	    is_http_tunnel = 1;
	  }
	
	if(strstr(header_buffer,"GET /mproxy") >0 )
	  {
	    printf("====== hand proxy info request ====");
	    //返回proxy的运行基本信息
	    hand_mproxy_info_req(client_sock,header_buffer);
	    return;
	  }
	
	if(extract_host(header_buffer) < 0)
	  {
	    printf("Cannot extract host field,bad http protrotol");
	    return;
	  }
	printf("Host:%s port: %d io_flag:%d\n",remote_host,remote_port,io_flag);
	
      }
    }
  
  if ((remote_sock = create_connection()) < 0) {
    printf("Cannot connect to host [%s:%d]\n",remote_host,remote_port);
    return;
  }

  if (fork() == 0) { // 创建子进程用于从客户端转发数据到远端socket接口

    if(strlen(header_buffer) > 0 && !is_http_tunnel)
      {
	forward_header(remote_sock); //普通的http请求先转发header
      }

    forward_data(client_sock, remote_sock);
    exit(0);
  }

  if (fork() == 0) { // 创建子进程用于转发从远端socket接口过来的数据到客户端

    if(io_flag == W_S_ENC)
      {
	io_flag = R_C_DEC; //发送请求给服务端进行编码，读取服务端的响应则进行解码
      } else if (io_flag == R_C_DEC)
      {
	io_flag = W_S_ENC; //接收客户端请求进行解码，那么响应客户端请求需要编码
      }

    if(is_http_tunnel)
      {
	send_tunnel_ok(client_sock);
      }

    forward_data(remote_sock, client_sock);
    exit(0);
  }

  close(remote_sock);
  close(client_sock);
}
