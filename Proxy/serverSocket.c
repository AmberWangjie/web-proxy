#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include "parser.h"
#include "cache.h"
#include "socket.h"
#include "log.h"

pthread_mutex_t lock;
int uid = 100;
struct _threadPara {
    int fd;
    FILE* f;
    struct sockaddr_in server;
};
typedef struct _threadPara threadPara;

void* multiThreadHandle(void* para){
    printf("Build connection\n");

    threadPara* pa = (threadPara *)para;
    int conn_fd = pa->fd;
    FILE* log = pa->f;
    struct sockaddr_in server = pa->server;
    char buff[BUFF_SIZE];
    memset(buff, '\0', BUFF_SIZE);
    int receive_len = recv(conn_fd, buff, BUFF_SIZE, 0);
    if(receive_len < 0){
        perror("Failed to receive response\n");
        printf("%d: %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    else if(receive_len == 0){
        printf("No response received\n");
    }
    else{
        printf("Receive request:\n%s\n", buff);
    }

    req_info* reqinfo = (req_info *)malloc(sizeof(req_info));
    if(reqinfo == NULL){
        perror("Allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    char* sendbuff = parse_request(buff, reqinfo);
    printf("Length = %lu\nParsing result:\n%s", strlen(sendbuff), sendbuff);
    printf("Method: %s\nService: %s\nHost: %s\nPort: %d\nCompURL: %s\nPartURL: %s\n", reqinfo->method, reqinfo->prtc, reqinfo->host, reqinfo->port, reqinfo->c_url, reqinfo->p_url);
    pthread_mutex_lock(&lock);
    logRequest(log, reqinfo->header, uid, inet_ntoa(server.sin_addr));
    uid++;
    char* cache_read = NULL;
    if(strcmp(reqinfo->method, "GET") == 0){
        cache_read = readCache(reqinfo->c_url, log, uid);
        uid++;
    }
    int send_status;
    rsp_info* rspinfo;
    if(cache_read == NULL){
        logServer(log, reqinfo->header, NULL, reqinfo->host, uid);
        uid++;
        char* newbuff = clientSock(reqinfo->host, reqinfo->prtc, sendbuff);
        if(newbuff == NULL || strlen(newbuff) == 0){
            close(conn_fd);
            free(reqinfo->host);
            free(reqinfo->c_url);
            free(reqinfo->p_url);
            free(reqinfo->header);
            free(reqinfo);
            printf("Connection closed\n");
            pthread_exit(NULL);    
            //return NULL;
        }
        rspinfo = parse_response(newbuff);
        printf("Code: %d\nStatus: %s\nDate: %s\n", rspinfo->code, rspinfo->status, rspinfo->date);
        logServer(log, NULL, rspinfo->status, reqinfo->host, uid);
        uid++;
        if(rspinfo->code == 200){
            if(rspinfo->cache != NULL && strcmp(rspinfo->cache, "no-cache") == 0){
                logOkCheck(log, 4, NULL, uid);
            }
            else if(rspinfo->expire != NULL){
                logOkCheck(log, 1, rspinfo->expire, uid);
            }
            else{
                logOkCheck(log, 5, NULL, uid);
            }
            uid++;
        }
        double exptime = EXPIRE_TIME;
        if(rspinfo->expire != NULL){
            exptime = expMinusDate(rspinfo->date, rspinfo->expire);
        }
        if(strcmp(reqinfo->method, "GET") == 0 && rspinfo->code == 200){
            if(rspinfo->cache == NULL){
                allocCache(newbuff, reqinfo->c_url, rspinfo->date, exptime);
            }
            else{
                logC_control(log, rspinfo->cache, uid);
                uid++;
                if(strcmp(rspinfo->cache, "no-cache") != 0){
                    allocCache(newbuff, reqinfo->c_url, rspinfo->date, exptime);
                }
            }
        }
        if(rspinfo->etag != NULL){
            logEtag(log, rspinfo->etag, uid);
            uid++;
        }
        send_status = send(conn_fd, newbuff, strlen(newbuff)+1, 0);
        if(rspinfo->code != 200){
            logRespClient(log, rspinfo->status, 1, uid);
        }
        else{
            logRespClient(log, rspinfo->status, 0, uid);
        }
        uid++;
        if(rspinfo->etag != NULL){
            free(rspinfo->etag);
        }
        if(rspinfo->expire != NULL){
            free(rspinfo->expire);
        }
        if(rspinfo->length != NULL){
            free(rspinfo->length);
        }
        if(rspinfo->cache != NULL){
            free(rspinfo->cache);
        }
        free(rspinfo->date);
        free(rspinfo->status);
        free(rspinfo);
        free(newbuff);
    }
    else{
        send_status = send(conn_fd, cache_read, strlen(cache_read)+1, 0);
    }
    pthread_mutex_unlock(&lock);
    if(send_status < 0){
        perror("Failed to send response\n");
        exit(EXIT_FAILURE);
    }
    printf("Response is successfully sent\n");
    close(conn_fd);

    free(reqinfo->host);
    free(reqinfo->c_url);
    free(reqinfo->p_url);
    free(reqinfo->header);
    free(reqinfo);
    
    printf("Connection closed\n");
	pthread_exit(NULL);    
	//return NULL;
}

int main(int argc, char const *argv[])
{
    if(argc != 2){
        perror("Input listening port\n");
        exit(EXIT_FAILURE);
    }
    int po = atoi(argv[1]);
    if(po < 1024){
        perror("Engaging reserved port is banned\n");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server;
    unsigned int socket_len = sizeof(server);
    FILE* fp;
    char* name = "/var/log/erss-proxy.log";
    fp = fopen(name, "w+");
    if(fp == NULL){
        printf("fail to open the log file, error = %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    /* Create socket */
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&server, 0, socket_len);
    server.sin_family = AF_INET;
    server.sin_port = htons(po);
    server.sin_addr.s_addr = INADDR_ANY;
    
    /* Bind */
    int bind_status = bind(socket_fd, (struct sockaddr *)&server, socket_len);
    if(bind_status < 0){
        perror("Bind error\n");
        exit(EXIT_FAILURE);
    }

    /* Listen */
    int listen_status = listen(socket_fd, 4);
    if(listen_status < 0){
        perror("Listen error\n");
        exit(EXIT_FAILURE);
    }
    int daemon_status = daemon(0, 1);
    if(daemon_status != 0){
        perror("Daemon create failed\n");
        exit(EXIT_FAILURE);
    }
    /* Accept */
    
    while(1){
        int conn_fd = accept(socket_fd, (struct sockaddr *)&server, &socket_len);
        if(conn_fd < 0){
            perror("Connection error\n");
            exit(EXIT_FAILURE);
        }
        
        threadPara para;
        para.fd = conn_fd;
        para.f = fp;
        para.server = server;
        //multiThreadHandle(&para);
        pthread_t thread;
        if(pthread_create(&thread, NULL, multiThreadHandle, &para) != 0) {
            perror("Thread create error\n");
            exit(EXIT_FAILURE);
        }
        /*if(pthread_join(thread, NULL) != 0){
            perror("Thread join error\n");
            exit(EXIT_FAILURE);
        }*/
    }
    fclose(fp);
    return EXIT_SUCCESS;
}
