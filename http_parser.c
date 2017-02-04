#include "http_parer.h"

ssize_t read_line_to_buf(int fd, void* buffer, size_t n){ssize_t numRead;
  size_t totRead;
  char *buf;
  char ch;

  if (n <= 0 || buffer == NULL) {
    errno = EINVAL;
    return -1;
  }

  buf = buffer;

  totRead = 0;
  while(1) {
    numRead = receive_data(fd, &ch, 1);
    if (numRead == -1) {
      if (errno == EINTR)  /* interuppted function call */
	continue;
      else
	return -1;              /* unknown err */
      
    } else if (numRead == 0) {      /* EOF */
      if (totRead == 0)           /* No bytes read; return 0 */
	return 0;
      else                        /* Some bytes read; add '\0' */
	break;
      
    } else {
      
      if (totRead < n - 1) {      /* Discard > (n - 1) bytes */
	totRead++;
	*buf++ = ch;
      }
      
      if (ch == '\n')
	break;
    }
  }
  
  *buf = '\0';
  return totRead;
}

/*read the request header from client, save to buffer*/
int read_request_header(int fd,void* buffer){
  memset(header_buffer,0,MAX_HEADER_SIZE);
  char line_buffer[2048];
  char * base_ptr = header_buffer;

  while(1)
    {
      memset(line_buffer,0,2048); //fill the memory with 0 

      int total_read = readLine(fd,line_buffer,2048);
      if(total_read <= 0)
	{
	  return CLIENT_SOCKET_ERROR;
	}
      //In case the header buffer overflow
      if(base_ptr + total_read - header_buffer <= MAX_HEADER_SIZE)
	{
	  strncpy(base_ptr,line_buffer,total_read);
	  base_ptr += total_read;
	} else
	{
	  return HEADER_BUFFER_FULL;
	}

      //read CRLF, header ends
      if(strcmp(line_buffer,"\r\n") == 0 || strcmp(line_buffer,"\n") == 0)
	{
	  break;
	}

    }
  return 0;
} 

/* Return the additional information about the server-response message. */
/* Parameters: the socket to print the headers on
 *             the name of the file */
void resp_header_fields(int client, const char *filename)
{
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
  while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
    numchars = get_line(client, buf, sizeof(buf));

  /*open the server file*/
  resource = fopen(filename, "r");
  if (resource == NULL)
    not_found(client);
  else
    {
      /*write HTTP header */
      headers(client, filename);
      /*copy*/
      cat(client, resource);
    }
  fclose(resource);
}

/* Inform the client that the requested web method has not been
 * implemented.
 * Parameter: the client socket */
void unimplemented(int client)
{
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

/*/* In CONNECT method,parse the host name and port number*/
void get_host(char* header, char* output){
  char * _p = strstr(header,"CONNECT");  
  if(_p)
    {
      char * _p1 = strchr(_p,' ');

      char * _p2 = strchr(_p1 + 1,':');
      char * _p3 = strchr(_p1 + 1,' ');

      if(_p2)
	{
	  char s_port[10];
	  bzero(s_port,10);

	  strncpy(remote_host,_p1+1,(int)(_p2  - _p1) - 1);
	  strncpy(s_port,_p2+1,(int) (_p3 - _p2) -1);
	  remote_port = atoi(s_port);

	} else
	{
	  strncpy(remote_host,_p1+1,(int)(_p3  - _p1) -1);
	  remote_port = 80;
	}


      return 0;
    }


  char * p = strstr(header,"Host:");
  if(!p)
    {
      return BAD_HTTP_PROTOCOL;
    }
  char * p1 = strchr(p,'\n');
  if(!p1)
    {
      return BAD_HTTP_PROTOCOL;
    }

  char * p2 = strchr(p + 5,':'); /* 5:length of 'Host:'*/

  if(p2 && p2 < p1)
    {

      int p_len = (int)(p1 - p2 -1);
      char s_port[p_len];
      strncpy(s_port,p2+1,p_len);
      s_port[p_len] = '\0';
      remote_port = atoi(s_port);

      int h_len = (int)(p2 - p -5 -1 );
      strncpy(remote_host,p + 5 + 1  ,h_len); //Host:
      //assert h_len < 128;
      remote_host[h_len] = '\0';
    } else
    {
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
  int numchars;
  char method[255];
  char url[255];
  char path[512];
  size_t i;
  size_t j;
  struct stat st;
  int cgi = 0;  /* becomes true if server decides this is a CGI program */
  char* query_string = NULL;

  /*get the first line of the request*/
  numchars = get_line(client, buf, sizeof(buf));
  i = 0;
  j = 0;
  /*save the method from client to method array*/
  while (!IS_SPACE(buf[j]) && (i < sizeof(method) - 1))
    {
      method[i] = buf[j];
      i++;
      j++;
    }
  method[i] = '\0';

  /*If it is not one of the three method, cannot implement */
  if (strcasecmp(method, "GET") && strcasecmp(method, "POST") && strcasecmp(method, "CONNECT"))
    /*return 0 if match*/
    {
      unimplemented(client);
      return;
    }

  /* POST: start cgi */
  if (strcasecmp(method, "POST") == 0)
    cgi = 1;

  /*read url address*/
  i = 0;
  while (IS_SPACE(buf[j]) && (j < sizeof(buf)))
    j++;
  while (!IS_SPACE(buf[j]) && (i < sizeof(url) - 1) && (j < sizeof(buf)))
    {
      /*save the url */
      url[i] = buf[j];
      i++;
      j++;
    }
  url[i] = '\0';

  /*handle the GET method*/
  if (strcasecmp(method, "GET") == 0)
    {
      /* the request to be handled is url */
      query_string = url;
      while ((*query_string != '?') && (*query_string != '\0'))
	query_string++;
      /* query string is after the "?", need the cgi program to handle */
      if (*query_string == '?')
	{
	  /*start cgi */
	  cgi = 1;
	  *query_string = '\0';
	  query_string++;
	}
    }

  /*formalize url to path array，html file is in the htdocs */
  sprintf(path, "htdocs%s", url);
  /*default: index.html */
  if (path[strlen(path) - 1] == '/')
    strcat(path, "index.html");
  /*find the file by path */
  if (stat(path, &st) == -1) {
    /*discard all  headers info*/
    while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
      numchars = get_line(client, buf, sizeof(buf));
    /*report to the client*/
    not_found(client);
  }
  else
    {
      /*If the file is a dir, default is index.html file*/
      if ((st.st_mode & S_IFMT) == S_IFDIR)
	strcat(path, "/index.html");
      if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH)    )
	cgi = 1;
      /*not cgi,return the server file,otherwise, cgi */
      if (!cgi)
	serve_file(client, path);
      else
	execute_cgi(client, path, method, query_string);
    }

  /*HTTP: no constant connection, so close here*/
  close(client);
}

/* Inform the client that a request it has made has a problem.
 * Parameters: client socket */
void bad_request(int client)
{
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

/* Put the entire contents of a file out on a socket.  This function
 * is named after the UNIX "cat" command, because it might have been
 * easier just to do something like pipe, fork, and exec("cat").
 * Parameters: the client socket descriptor
 *             FILE pointer for the file to cat */
void cat(int client, FILE *resource)
{
  char buf[1024];

  /*read the file and write to  socket */
  fgets(buf, sizeof(buf), resource);
  while (!feof(resource))
    {
      send(client, buf, strlen(buf), 0);
      fgets(buf, sizeof(buf), resource);
    }
}

/* Inform the client that a CGI script could not be executed.
 * Parameter: the client socket descriptor. */
void cannot_execute(int client)
{
  char buf[1024];

  /* reponse to client that cgi not executable*/
  sprintf(buf, "HTTP/1.0 500 Internal Server Error\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "Content-type: text/html\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "<P>Error prohibited CGI execution.\r\n");
  send(client, buf, strlen(buf), 0);
}

/* Print out an error message with perror() (for system errors; based  on value of errno, which indicates system call errors) and exit the program indicating an error. */
void error_die(const char *sc)
{
  /*handle the err */
  perror(sc);
  exit(1);
}

