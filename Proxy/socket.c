#include "socket.h"

char* clientSock(char* host, char* port, char* buff){
    int socket_fd;
    struct addrinfo client; 
    struct addrinfo* clientlist;
    unsigned int socket_len = sizeof(client);
    /* Create socket */
    memset(&client, '\0', socket_len);
    client.ai_family = AF_UNSPEC;
    client.ai_socktype = SOCK_STREAM;
    int info_status = getaddrinfo(host, port, &client, &clientlist);
    if(info_status != 0){
        perror("Failed to get server information\n");
        freeaddrinfo(clientlist);
        return NULL;
    }
    
    /* Connect */
    struct addrinfo* s;
    for(s = clientlist; s != NULL; s = s->ai_next){
        if((socket_fd = socket(s->ai_family, s->ai_socktype, s->ai_protocol)) < 0){
            perror("Try next connection option\n");
            continue;
        }
        if(connect(socket_fd, s->ai_addr, s->ai_addrlen) < 0){
            perror("Connection error\n");
            close(socket_fd);
            continue;
        }
        break;
    }
    if(s == NULL){
        fprintf(stderr, "failed to connect\n");
        freeaddrinfo(clientlist);
        return NULL;
    }
    printf("Build connection\n");
    int send_status = send(socket_fd, buff, strlen(buff)+1, 0);
    if(send_status < 0){
        perror("Failed to send message\n");
        close(socket_fd);
        freeaddrinfo(clientlist);
        return NULL;
    }
    printf("Message is successfully sent\n");
    free(buff);
    int receive_len;
    char* ret = malloc(sizeof(char));
    memset(ret, '\0', sizeof(char));
    do{
        char newbuff[BUFF_SIZE];
        memset(newbuff, '\0', BUFF_SIZE);
        receive_len = recv(socket_fd, newbuff, BUFF_SIZE, 0);
        printf("receive length = %d\n", receive_len);
        printf("newbuff length = %lu\n", strlen(newbuff));
        if(receive_len < 0){
            perror("Failed to receive response\n");
            printf("%d: %s\n", errno, strerror(errno));
            close(socket_fd);
            freeaddrinfo(clientlist);
            free(ret);
            return NULL;
        }
        ret = realloc(ret, strlen(ret)+receive_len+1);
        strcat(ret, newbuff);
    }while(receive_len != 0);
    printf("Response length %lu\n", strlen(ret));
    //printf("Response:\n%s\n", ret);
    close(socket_fd);
    freeaddrinfo(clientlist);
    return ret;
}
