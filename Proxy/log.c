#include "log.h"

//int uid = 0;
/*before log, create a file stream to output*/
//FILE* fp;
//fp = fopen("erss-proxy.log","w+"); //in /var/log/erss-proxy.log, need to indicate the dir in code?
//don't forget to close(fp) at the end
void logRequest(FILE* log, char* req_line, int uid,  char* ip_addr){
  if(!log){
    printf("log file point to null\n");
    return;
  }
  char* time = getLoctime();
  fprintf(log, "%d: %s from %s @ %s\n", uid, req_line, ip_addr, time);
  if(fflush(log) != 0){
    printf("Failed to flush the log buffer\n");
    free(time);
    return;
  }
}

/*if request is GET, check the cache for status*/
void logCheckRequest(FILE* log, int uid, int cacheStatus){
  if(!log){
    fprintf(log, "log file point to null\n");
    return;
  }
  switch(cacheStatus){
  case notInCache:
    fprintf(log, "%d: not in cache\n", uid);
    break;
  case cacheExpire:
    fprintf(log, "%d: not in cache, has already expired and been kicked out\n", uid);
    break;
  /* case reqValid: */
  /*   fprintf(log, "%d: in cache, requires validation\n", uid); */
  /*   break; */
  case isValid:
    fprintf(log, "%d: in cache, valid\n", uid);
    break;
  default:
    fprintf(log, "No idea what the cache response is\n");
    break;
  }
  if(fflush(log) != 0){
    printf("Failed to flush the log buffer\n");
    return;
  }
}

/*when need to cotact with or get the respoonse from the origin server*/
//either request or response, the unavailable one pass null
//origin server? 
void logServer(FILE* log, char* request, char* response, char* server, int uid){
  if(!log){
    fprintf(log, "log file point to null\n");
    return;
  }
  if(request != NULL){
    fprintf(log, "%d: Requesting %s from %s\n", uid, request, server);
  }
  else if(response != NULL){
    fprintf(log, "%d: Received %s from %s\n", uid, response, server);
  }
  if(fflush(log) != 0){
    printf("Failed to flush the log buffer\n");
    return;
  }
}

/*if receives a 200-OK in response to a GET*/
//expire is the expire string entity in rsp_info_t
void logOkCheck(FILE* log, int respStatus, char* expire, int uid){
  if(!log){
    fprintf(log, "log file point to null\n");
    return;
  }
  switch(respStatus){
  case notCacheable:
    fprintf(log, "%d: not cacheable because no-cache entity exists\n", uid);
    break;
  case cacheExpire:
    fprintf(log, "%d: cached, but has expired at %s\n", uid, expire);
    break;
  case reqReValid:
    fprintf(log, "%d: cached, but requires re-validation\n", uid);
    break;
  default:
    fprintf(log, "No idea what happened to cache after receiving 200-OK response\n");
    break;
  }
  if(fflush(log) != 0){
    printf("Failed to flush the log buffer\n");
    return;
  }
}

/*whenver proxy respond to client, even with an error*/
//need to see if the request is malformed-> set bool malFormed
void logRespClient(FILE* log, char* response, int malFormed, int uid){
  if(!log){
    fprintf(log, "log file point to null\n");
    return;
  }
  if(malFormed){
    fprintf(log, "%d: Responding error: %s\n",uid, response);
    return; //not sure
  }
  fprintf(log, "%d: Responding %s\n", uid, response);
  if(fflush(log) != 0){
    printf("Failed to flush the log buffer\n");
    return;
  }
}

//if(req_info->method == "CONNECT")&&(rsp_info->code == 200, rsp_info->status == "OK")
//need bool is_closed(for tunnel)
void logTunnelOk(FILE* log, int is_closed, int uid){
  if(!log){
    fprintf(log, "log file point to null\n");
    return;
  }
  fprintf(log, "%d: Tunnel closed\n", uid); 
  if(fflush(log) != 0){
    printf("Failed to flush the log buffer\n");
    return;
  }
}

//may want to check if c_control is null before call this log
void logC_control(FILE* log, char* c_control, int uid){
  if(!log){
    fprintf(log,"log file point to null\n");
    return;
  }
  fprintf(log, "%d: NOTE Cache-Control: %s\n",uid,c_control);
  if(fflush(log) != 0){
    printf("Failed to flush the log buffer\n");
    return;
  }
}

void logEtag(FILE* log, char* E_tag, int uid){
  if(!log){
    fprintf(log,"log file point to null\n");
    return;
  }
  fprintf(log, "%d: NOTE ETag: %s\n",uid,E_tag);
  if(fflush(log) != 0){
    printf("Failed to flush the log buffer\n");
    return;
  }
}

char* getLoctime(){
  time_t curtime;
  struct tm* loc_time;
  curtime = time(NULL);
  loc_time = localtime(&curtime);
  char* curr = asctime(loc_time);
#ifdef DEBUG
  printf("getLoctime is:%s\n", curr);
#endif
  char* time = malloc(strlen(curr)+1);
  memset(time, '\0', strlen(curr)+1);
  strcpy(time, curr);
  return time; 
}
