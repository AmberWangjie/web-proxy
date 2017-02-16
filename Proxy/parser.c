#include "parser.h"

#define DEBUG

char* parse_request(char* buffer, req_info* tokens){
  char* m_s = buffer;
  char* m_e = strstr(m_s, "\r\n");
  tokens->header = (char*)malloc((int)(m_e - m_s + 1));
  memset(tokens->header, '\0', (int)(m_e - m_s + 1)); 
  strncpy(tokens->header, m_s, (int)(m_e - m_s));
  m_s = buffer;
  m_e = strchr(m_s, ' '); 

  /* method */
  for(int i = 0; i < 10; i++){
	  tokens->method[i] = '\0';
  }
  strncpy(tokens->method, m_s, (int)(m_e - m_s));
  /*if(strcasecmp(tokens->method, "GET") && strcasecmp(tokens->method, "POST") && strcasecmp(tokens->method, "CONNECT")) {
	  printf("This method cannot be implemented\n");
	  return NULL;
  }*/

  /* completed URL */
  m_s = m_e + 1;
  m_e = strchr(m_s,' '); 
  tokens->c_url = (char*)malloc((int)(m_e - m_s + 1));
  memset(tokens->c_url, '\0', (int)(m_e - m_s + 1)); 
  strncpy(tokens->c_url, m_s, (int)(m_e - m_s));

  /* protocol */
  m_e = strstr(m_s, "://"); 
printf("1st str\n");
  for(int i = 0; i < 10; i++){
  	tokens->prtc[i] = '\0';
  }
  if(m_e != NULL){
    strncpy(tokens->prtc, m_s, (int)(m_e - m_s));
    m_s = m_e + 3; //3 is the length of "://"
  }
  else{
    m_s = strchr(buffer, ' ') + 1;
  }
printf("1st strncp\n");
  /* host, port, partial URL */
  if(strchr(m_s, ':') != NULL && strchr(m_s, ':') < strchr(m_s, ' ')){ // has port e.g. www.google.com:8080/maps HTTP/1.1\r\n
  	/* host */
    m_e = strchr(m_s, ':');
    tokens->host = (char*)malloc((int)(m_e - m_s + 1)); 
    memset(tokens->host, '\0', (int)(m_e - m_s + 1));
	  strncpy(tokens->host, m_s, (int)(m_e - m_s));
printf("2nd strncp\n");
	  m_s = m_e + 1;
    if(strchr(m_s, '/') != NULL && strchr(m_s, '/') < strchr(m_s, ' ')){ // has partial URL e.g. www.google.com:8080/maps HTTP/1.1\r\n
      /* port */
      m_e = strchr(m_s, '/'); 
      char* temp = malloc((int)(m_e - m_s + 1)); 
      memset(temp, '\0', (int)(m_e - m_s + 1));
      strncpy(temp, m_s, (int)(m_e - m_s));
      if(strcmp(tokens->prtc, "") == 0){
        strcpy(tokens->prtc, temp);
      }
printf("3rd strncp\n");
      tokens->port = atoi(temp);
      free(temp);
      m_s = m_e;
	  
	  /* Partial URL */
	    m_e = strchr(m_s, ' ');
      tokens->p_url = (char*)malloc((int)(m_e - m_s + 1));
	    memset(tokens->p_url, '\0', (int)(m_e - m_s + 1));
      strncpy(tokens->p_url, m_s, (int)(m_e - m_s));
printf("4th strncp\n");
    }
    else{ // no partial URL e.g. www.google.com:8080 HTTP/1.1\r\n
      /* port */
      m_e = strchr(m_s, ' '); 
      char* temp = malloc((int)(m_e - m_s + 1)); 
	    memset(temp, '\0', (int)(m_e - m_s + 1));
      strncpy(temp, m_s, (int)(m_e - m_s));
printf("5th strncp\n");
      if(strcmp(tokens->prtc, "") == 0){
        strcpy(tokens->prtc, temp);
      }
      tokens->port = atoi(temp);
      free(temp);

      /* partial URL */
      tokens->p_url = (char*)malloc(2*sizeof(char));
      memset(tokens->p_url, '\0', 2*sizeof(char));
      strcat(tokens->p_url, "/");
    }
  }
  else{ // no port
    if(strchr(m_s, '/') != NULL && strchr(m_s, '/') < strchr(m_s, ' ')){ // has partial URL e.g. www.google.com/maps HTTP/1.1
      /* Host */
      m_e = strchr(m_s, '/');
      tokens->host = (char*)malloc((int)(m_e - m_s + 1)); 
	    memset(tokens->host, '\0', (int)(m_e - m_s + 1));
      strncpy(tokens->host, m_s, (int)(m_e - m_s));
printf("6th strncp\n");
	    m_s = m_e;
	  
	    /* Partial URL */
	    m_e = strchr(m_s, ' ');
      tokens->p_url = (char*)malloc((int)(m_e - m_s + 1)); 
	    memset(tokens->p_url, '\0', (int)(m_e - m_s + 1));
      strncpy(tokens->p_url, m_s, (int)(m_e - m_s));
printf("7th strncp\n");
    }
    else{ //  no partial URL e.g. www.google.com HTTP/1.1
      /* Host */
      m_e = strchr(m_s, ' ');
      tokens->host = (char*)malloc((int)(m_e - m_s + 1)); 
	    memset(tokens->host, '\0', (int)(m_e - m_s + 1));
      strncpy(tokens->host, m_s, (int)(m_e - m_s));
printf("8th strncp\n");
      /* Partial URL */
      tokens->p_url = (char*)malloc(2*sizeof(char));
      memset(tokens->p_url, '\0', 2*sizeof(char));
      strcat(tokens->p_url, "/");
    }
    /* Port */
    if(strcmp(tokens->prtc, "") == 0){
        strcpy(tokens->prtc, "80");
      }
    tokens->port = 80;
  }
  m_s = strstr(m_e, "\r\n");
printf("2nd str\n");
  char* str_host = "\r\nHost: ";
  int need_host = 0;
  if(strstr(m_s, str_host) == NULL){
  	need_host = 1;
  }
  else if(strstr(m_s, str_host) > strstr(m_s, "\r\n\r\n")){
  	need_host = 1;
  }
printf("4th str\n");
  char* str_conn = "\r\nConnection: ";
  int need_conn = 0;
  if(strstr(m_s, str_conn) == NULL){
  	need_conn = 1;
  }
  else if(strstr(m_s, str_conn) > strstr(m_s, "\r\n\r\n")){
  	need_conn = 1;
  }
printf("6th str\n");
  char* http = "HTTP/1.1";
  char* clos = "close";
  if(strcmp(tokens->method, "CONNECT") == 0){
    printf("Buffer size = %lu\n", strlen(buffer));
    char* ans = malloc(strlen(buffer)+1);
    memset(ans, '\0', strlen(buffer)+1);
    strcpy(ans, buffer);
    return ans;
  }
printf("7th str\n");
  size_t alloclen = strlen(tokens->method)+strlen(tokens->p_url)+strlen(http)+2 + need_host*(strlen(str_host)+strlen(tokens->host)) + need_conn*(strlen(str_conn)+strlen(clos)) + strlen(m_s)+1;
  printf("allocation size = %lu\n", alloclen);
  char* ans = malloc(alloclen);
  if(ans == NULL){
    printf("Malloc failed\n");
    return NULL;
  }
  memset(ans, '\0', alloclen);
  strcpy(ans, tokens->method);
  strcat(ans, " ");
  strcat(ans, tokens->p_url);
  strcat(ans, " ");
  strcat(ans, http);
  if(need_host == 1){
    strcat(ans, str_host);
    strcat(ans, tokens->host);
  }
  if(need_conn == 1){
    strcat(ans, str_conn);
    strcat(ans, clos);
  }
  if((m_e = strstr(m_s, "\r\nProxy-Connection: ")) != NULL){
    strncat(ans, m_s, (int)(m_e - m_s));
    m_e += 2;
    m_s = strstr(m_e, "\r\n");
  }
printf("8th str\n");
  strcat(ans, m_s);
  return ans;
}


rsp_info* parse_response(char* buffer){
  rsp_info* tokens = (rsp_info*)malloc(sizeof(rsp_info));
  char* m_s = buffer;
  char* m_e = strstr(m_s, "\r\n");
  char* c_s = m_e;
  // status
  tokens->status = malloc((int)(m_e - m_s + 1));
  memset(tokens->status, '\0', (int)(m_e - m_s + 1));
  strncpy(tokens->status, m_s, (int)(m_e - m_s));

  m_s = strchr(m_s, ' ') + 1;
  m_e = strchr(m_s, ' ');
  // code
  char* temp = malloc((int)(m_e - m_s + 1));
  memset(temp, '\0', (int)(m_e - m_s + 1));
  strncpy(temp, m_s, (int)(m_e - m_s));
  tokens->code = atoi(temp);
  free(temp);
  // date
  if((m_s = strstr(c_s, "\r\nDate: ")) != NULL){
    m_s = strchr(m_s, ' ') + 1;
    m_e = strstr(m_s, "\r\n");
    tokens->date = malloc((int)(m_e - m_s + 1));
    memset(tokens->date, '\0', (int)(m_e - m_s + 1));
    strncpy(tokens->date, m_s, (int)(m_e - m_s));
  }
  else{
    tokens->date = NULL;
  }
  // length
  if((m_s = strstr(c_s, "\r\nContent-Length: ")) != NULL){
    m_s = strchr(m_s, ' ') + 1;
    m_e = strstr(m_s, "\r\n");
    tokens->length = malloc((int)(m_e - m_s + 1));
    memset(tokens->length, '\0', (int)(m_e - m_s + 1));
    strncpy(tokens->length, m_s, (int)(m_e - m_s));
  }
  else{
    tokens->length = NULL;
  }
  // cache
  if((m_s = strstr(c_s, "\r\nCache-Control: ")) != NULL){
    m_s = strchr(m_s, ' ') + 1;
    m_e = strstr(m_s, "\r\n");
    tokens->cache = malloc((int)(m_e - m_s + 1));
    memset(tokens->cache, '\0', (int)(m_e - m_s + 1));
    strncpy(tokens->cache, m_s, (int)(m_e - m_s));
  }
  else{
    tokens->cache = NULL;
  }
  // expires
  if((m_s = strstr(c_s, "\r\nExpires: ")) != NULL){
    m_s = strchr(m_s, ' ') + 1;
    m_e = strstr(m_s, "\r\n");
    tokens->expire = malloc((int)(m_e - m_s + 1));
    memset(tokens->expire, '\0', (int)(m_e - m_s + 1));
    strncpy(tokens->expire, m_s, (int)(m_e - m_s));
  }
  else{
    tokens->expire = NULL;
  }
  // etag
  if((m_s = strstr(c_s, "\r\nEtag: ")) != NULL){
    m_s = strchr(m_s, ' ') + 1;
    m_e = strstr(m_s, "\r\n");
    tokens->etag = malloc((int)(m_e - m_s + 1));
    memset(tokens->etag, '\0', (int)(m_e - m_s + 1));
    strncpy(tokens->etag, m_s, (int)(m_e - m_s));
  }
  else{
    tokens->etag = NULL;
  }
  return tokens;
}
