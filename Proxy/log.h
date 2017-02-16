#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define notInCache 0  //cannot find in cache
#define cacheExpire 1 //in cache but expired
#define isValid 2 //in cache and not expired
#define reqReValid 5 //if revalidate, resend the Etag to compare if changed
#define notCacheable 4

//int uid = 0;
void logRequest(FILE*, char*, int, char*);
void logCheckRequest(FILE*, int, int);
void logServer(FILE*, char*, char*, char*, int);
void logOkCheck(FILE*, int, char*, int);
void logRespClient(FILE*, char*, int, int);
void logTunnelOk(FILE*, int, int);
void logC_control(FILE*, char*, int);
void logEtag(FILE*, char*, int);
char* getLoctime();
