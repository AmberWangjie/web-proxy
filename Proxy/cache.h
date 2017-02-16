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

#define MAX_OBJECT_NUM 1024
#define MAX_OBJECT_SIZE 1073741824
#define EXPIRE_TIME 10000.0

struct _cache{
	char* url;
	char* res;
	char* date;
	double ext;
	struct _cache* next;
	struct _cache* prev;
};
typedef struct _cache cache;

double curMinusDate(char*);
double expMinusDate(char*, char*);
bool allocCache(char* buff, char* url, char* date, double extime);
char* readCache(char* url, FILE* log, int uid);
bool scanCache(char* url, FILE* log, int uid);
void moveCache(struct _cache* curr);
void deleteCache(struct _cache* curr);
void printCache();


int isExpired(char*, double);
int cacheStatus(int);


