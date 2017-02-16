#include "cache.h"
#include "log.h"

#define DEBUG

double expMinusDate(char* date, char* expire){
  struct tm d;
  struct tm e;
  strptime(date, "%a, %d %b %Y %H:%M:%S GMT", &d);
  strptime(expire, "%a, %d %b %Y %H:%M:%S GMT", &e);
  time_t date_t = mktime(&d);  
  time_t expire_t = mktime(&e);  
  double diff = difftime(expire_t, date_t);
#ifdef DEBUG
  printf("expire - date in seconds is: %f\n", diff);
#endif
  return diff;
}

double curMinusDate(char* date){
  char* curr = getLoctime();
  struct tm c;
  strptime(curr, "%a, %b %d %H:%M:%S %Y", &c);
  struct tm d;
  strptime(date, "%a, %d %b %Y %H:%M:%S GMT", &d);
  time_t date_t = mktime(&d);
  time_t curr_t = mktime(&c);
  double diff = difftime(curr_t, date_t);
#ifdef DEBUG
  printf("curr - date in seconds is: %f\n", diff);
#endif
  return diff;  
}


cache* head = NULL;
cache* tail = NULL;
int obj_num = 0;

bool allocCache(char* buff, char* url, char* date, double extime){
	int buffsize = strlen(buff)+1;
	if(buffsize > MAX_OBJECT_SIZE){
		printf("Cache response failed. Response size = %d bytes is too big and won't be cached\n", buffsize);
		return false;
	}
	if(obj_num >= MAX_OBJECT_NUM){
		deleteCache(tail);
	}
	obj_num += 1;
	cache* node = (cache *)malloc(sizeof(cache));
	memset(node, '\0', sizeof(cache));
	node->url = (char *)malloc(strlen(url)+1);
	memset(node->url, '\0', strlen(url)+1);
	strcpy(node->url, url);
	node->res = (char *)malloc(buffsize);
	memset(node->res, '\0', buffsize);
	strcpy(node->res, buff);
	node->date = (char *)malloc(strlen(date)+1);
	memset(node->date, '\0', strlen(date)+1);
	strcpy(node->date, date);
	node->ext = extime;
	cache* temp = head;
	if(head != NULL){
		head->prev = node;
	}
	else{
		tail = node;
	}
	head = node;
	head->next = temp;
	head->prev = NULL;
	printf("Cache response successfully\n");
	return true;
}

/* 
	1. Read for buffer size, if larger than threshold, then won't exist in cache
	2. Scan for cache, if found and return cached response
	3. If not found, allocate a memory and store response
*/
char* readCache(char* url, FILE* log, int uid){
	if(!scanCache(url, log, uid)){
		printf("Cached response not found\n");
		return NULL;
	}
	printf("Found cached response\n");
	printf("%s\n", head->res);
	return head->res;
}

/* Scan for: 
	1. check if any URL matches(main)
	2. check if any cache expires(peripheral) 
*/
bool scanCache(char* url, FILE* log, int uid){
	cache* curr = head;
	bool found = false;
	int status = 0;
	while(curr != NULL){
		//curr->ext -= 1;
		if(isExpired(curr->date, curr->ext) == 1){
		//if(curr->ext <= 0){
			if(strcmp(curr->url, url) == 0){
				status = 1;
			}
			cache* temp = curr;
			curr = curr->next;
			deleteCache(temp);
			continue;
		}
		if(strcmp(curr->url, url) == 0){
			status = 2;
			cache* temp = curr;
			curr = curr->next;
			moveCache(temp);
			found = true;
			continue;
		}
		curr = curr->next;
	}
	logCheckRequest(log, uid, cacheStatus(status));
	return found;
}

/* Move hit cache to the head */
void moveCache(cache* curr){
	if(head == curr){
		return;
	}
	cache* pre = curr->prev;
	cache* suf = curr->next;
	pre->next = suf;
	if(suf != NULL){
		suf->prev = pre;
	}
	else{
		tail = pre;
	}
	cache* temp = head;
	head->prev = curr;
	head = curr;
	head->next = temp;
	head->prev = NULL;
}

void deleteCache(cache* curr){
	if(obj_num == 0){
		return;
	}
	cache* pre = curr->prev;
	cache* suf = curr->next;
	if(pre != NULL){
		pre->next = suf;
	}
	else{
		head = suf;
	}
	if(suf != NULL){
		suf->prev = pre;
	}
	else{
		tail = pre;
	}
	free(curr->date);
	free(curr->url);
	free(curr->res);
	free(curr);
	obj_num -= 1;
}

void printCache(){
	cache* curr = head;
	printf("Cache number: %d\n", obj_num);
	while(curr != NULL){
		printf("%s: %s\nexp: %f\n", curr->url, curr->res, curr->ext);
		curr = curr->next;
	}
}

int isExpired(char* date, double count){
  if(date == NULL){
    printf("Failed to get the date of response in cache\n");
    return 0;
  }
  int exp = -1;
  double diff = curMinusDate(date) - count;
  
#ifdef DEBUG
    printf("new - cur in seconds is: %f\n", diff);
#endif
    if(diff >= 0){
#ifdef DEBUG
      printf("DEBUG:  expired\n");
#endif
      exp = 1;
    }else{
#ifdef DEBUG
      exp = 0;
#endif
    }
  return exp;
}

/*use weak(Last-Modified time value) and strong validators(entity tag) to check for conditions*/
/*need to compare these validatory with the latest one, is there any way to get the head of the cache?*/
/*need to check whether the head is avaliable, if null, response is the first one, must valid??*/

/*see if expired, in cache, valid, need validation*/
int cacheStatus(int status){
  switch(status){
  case 0:
    return 0; //not in cache
  case 1:
    return 1; //in cache but expired
  case 2:
    return 2;
  }
  return -1;
}
