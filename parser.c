#include "parser.h"

#define DEBUG

char* buffer;


/*extract information from the request_line buffer*/
/*req_sock = tokens->host, req_serv = tokens->prtc*/
req_info* parse_request(char* buffer){
#ifdef DEBUG
  printf("start parsing request:\n");
#endif
  req_info* tokens = (req_info*)malloc(sizeof(req_info));
  if(tokens == NULL){
    free(tokens);
    printf("Failed to malloc\n");
    return NULL;
  }
#ifdef DEBUG
  printf("malloced for tokens\n");
#endif
  size_t i = 0;
  //skip space
  while(IS_SPACE(buffer[i])){
    i++;
  }
#ifdef DEBUG
  printf("skiped space\n");
#endif
  if(i >= strlen(buffer)-1){
    printf("Too many spaces, out of buffer bound\n");
    return NULL;
  }
  char* m_s = buffer+i; //first non-space char, should be the start of the method
  if(m_s){
#ifdef DEBUG
    printf("The input request line is:%s\n", m_s);
#endif
    char* m_e = strchr(m_s,' '); //first space after the start, should be the end of the method
    if(m_e){
      //printf("m_e point to %s\n", m_e);
      strncpy(tokens->method, m_s,(int)(m_e - m_s)); //copy into method[]
      i += (size_t)(m_e - m_s); 
      //  tokens->method[i+1] = '\0'; //end it by null terminator
#ifdef DEBUG
      printf("DEBUG: Method is: %s\n", tokens->method);
#endif
      //only accpet three method request
      if (strcasecmp(tokens->method, "GET") && strcasecmp(tokens->method, "POST")&& strcasecmp(tokens->method, "CONNECT")) {
	printf("This method cannot be implemented\n");
	return NULL;
      }
    }else{
      printf("There should be space after the request method\n");
      return NULL;
    }
  }else{
    printf("Please enter the request line\n");
    return NULL;
  }
  /*get the complete url*/
  char* c_s = strchr(m_s+strlen(tokens->method)-1,' '); 
  char* c_e = strchr(c_s+1, ' ');
  //printf("End of the url is:%s\n",c_e);
  // tokens->c_url=(char*)malloc(strlen(tokens->c_url) +1);
  strncpy(tokens->c_url, c_s+1, (int)(c_e - c_s));
#ifdef DEBUG
  printf("DEBUG: Complete url is:%s\n", tokens->c_url);
#endif
  //need to get the host and the protocol type
  char* p_s = c_s + 1; //start of the protocol ex.http, ftp, ...
  if(p_s){
    char* p_e = strstr(p_s + 1, "://"); //the colon before "//" ex.http://
    if(p_e){
      strncpy(tokens->prtc, p_s, (int)(p_e - p_s));
#ifdef DEBUG
      printf("DEBUG: Prtc is:%s\n", tokens->prtc);//copy into prtc[]
#endif
      //socket->serv = tokens->prtc;
    }
  }
  char* p_e = strchr(p_s + 1, ':'); 
  char* h_s = p_e + 3;//3 is the length of "://"
  while(IS_SPACE(*h_s)){
    h_s++;
  }
  if(h_s){
    char* po_s = strchr(h_s, ':');//after which is the port number if offered
    if(po_s){
#ifdef DEBUG
      printf("There is request port number in the url\n");
#endif
      char* po_e = strchr(po_s, '/'); //end of the host token, after the port number
      if(po_e){
	/*get the partial url from here*/
	char* pu_s = po_e;
	char* pu_e = strchr(pu_s, ' ');
	strncpy(tokens->p_url, pu_s, (int)(pu_e - pu_s)+1);
#ifdef DEBUG
	printf("DEBUG: The partial url is:%s\n", tokens->p_url);
#endif
	/*get the port*/
	char s_port[10];
	bzero(s_port,10); //fill with 0
	strncpy(tokens->host, h_s, (int)(po_s - h_s));
#ifdef DEBUG
	printf("DEBUG: Host is:%s\n", tokens->host);
#endif
	strncpy(s_port, po_s + 1, (int)(po_e - po_s)-1);
	//printf("DEBUG: Port is:%s\n", s_port);
	tokens->port = atoi(s_port);
#ifdef DEBUG
	printf("DEBUG: Port int is:%d\n",tokens->port);
#endif
      }else{
	printf("DEBUG:The url path is incomplete\n");
	return NULL;
      }
    }else{
      char* po_e = strchr(h_s, '/');
#ifdef DEBUG
      printf("DEBUG: There is no port field in request line, use default\n");
#endif
      strncpy(tokens->host, h_s, (int)(po_e - h_s) );
#ifdef DEBUG
      printf("DEBUG:Host is: %s\n",tokens->host);
#endif
      tokens->port = 80;
#ifdef DEBUG
      printf("DEBUG: port int is: %d\n", tokens->port);
#endif
    }
  }else{
    printf("DEBUG:Cannot find the start of the host in the request line\n");
    return NULL;
  }
  /*No host in request line, go find the header*/
  char* h_h = strstr(buffer, "Host:");
  if(!h_h){
#ifdef DEBUG
    printf("DEBUG: No Host header field, need to be added as following\n");
    printf("Host: %s\n", tokens->host);
#endif
  }
  return tokens;
}

/*rewrite the complete url to the partial url*/
void rewrite_header(){
  char * p = strstr(header_buffer,"://");
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

void forward_header(int dest_sock){
  rewrite_header();
  //#ifdef DEBUG
    // LOG("================ The Forward HEAD =================");
  //LOG("%s\n",header_buffer);
  //#endif
  
  int len = strlen(buffer);
  send(dest_sock,buffer,len,0) ;
}

void handle_client(int client_sock, struct sockaddr_in client_addr,req_info tokens){
  int is_http_tunnel = 0; 
  /* if(strlen(tokens->host) == 0) { */
  /*    if(read_header(client_sock,header_buffer) < 0){ */
  /*      LOG("Read Http header failed\n"); */
  /*      return; */
  /*    } else { */
  /*      char * p = strstr(header_buffer,"CONNECT"); /\*connect request?*\/ */
  /*      if(p){ */
  /* 	 LOG("receive CONNECT request\n"); */
  /* 	 is_http_tunnel = 1; */
  /*      } */
       
  if ((remote_sock = create_connection()) < 0) {
    LOG("Cannot connect to host [%s:%d]\n",remote_host,remote_port);
    return;
  }
  
  if (fork() == 0) { // 创建子进程用于从客户端转发数据到远端socket接口
    
    if(strlen(header_buffer) > 0 && !is_http_tunnel) {
	forward_header(remote_sock); //普通的http请求先转发header
    } 
    
    forward_data(client_sock, remote_sock);
    exit(0);
  }
}
  
