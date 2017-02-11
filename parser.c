#include "http_parer.h"

/* get_from_connect connect; */
/* connect->remost_host = NULL; */
/* connect->remote_port = 0; */
/* connect->local_port = 0; */

/*extract the host name and protocol type from the request buffer*/
void accept_request(char* buffer, req_sock socket,connect){
  char method[128];
  char prtc[128];
  //char url[512];
  char host[512];
  size_t i = 0;
  //skip space
  while(IS_SPACE(buffer[i])){
    i++;
  }
  if(i >= BUFFER_SIZE){
    printf("Too many spaces, out of buffer bound\n");
    return;
  }
  char* m_s = buffer[i]; //first non-space char, should be the start of the method
  if(m_s){
    char* m_e = strchr(m_s,' '); //first space after the start, should be the end of the method
    if(m_e){
      strncpy(method, m_s,(size_t)(m_e - m_s)); //copy into method[]
      i += (size_t)(m_e - m_s); 
      method[i+1] = '\0'; //end it by null terminator
      //only accpet three method request
      if (strcasecmp(method, "GET") && strcasecmp(method, "POST")&& strcasecmp(method, "CONNECT")) {
	printf("This method cannot be implemented\n");
	return;
      }
    }
  }
  //for GET, need to get the host and the protocol type
  if(strcasecmp(method, "GET") == 0 || strcasecmp(method, "POST") == 0){
    //get_request_path(buffer,);
    char* p_s = m_s + 1; //start of the protocol ex.http, ftp, ...
    if(p_s){
      char* p_e = strchr(p_s + 1, ':'); //the colon before "//" ex.http://
      if(p_e){
	strncpy(prtc, p_s, (size_t)(p_e - p_s));  //copy into prtc[]
	socket->serv = prtc;
      }
    }
    char* h_s = p_e + 3;
    if(h_s){
      char* h_e = strchr(h_s + 1, '/');
      if(h_e){
	strncpy(host, h_s, (size_t)(h_e - h_s));
      }else{
	printf("Cannot find the end of the host, invalid request\n");
	return;
      }
      socket->host = host;
    }else{
      printf("Cannot find the start of the host, invalid request\n");
      return;
    }
  }else{
    /*In connect method, get the host and port, encapsulated into a struct*/
    get_from_connect connect;
    connect->remost_host = NULL;
    connect->remote_port = 0;
    connect->local_port = 0;
    get_host_from_connect(buffer,connect);
  }
    
    
 }

/* In CONNECT method,parse the host name and port number*/
void get_host_from_connect(char* buffer, char* output){
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

/*get server path*/
/* void get_request_path(char* buffer,char* output){ */
/*   char * p = strstr(header,"GET /"); */
/*   if(p) { */
/*     char * p1 = strchr(p+4,' '); */
/*     strncpy(output,p+4,(int)(p1  - p - 4) ); */
/*   } */
/* }
