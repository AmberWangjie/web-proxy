#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#define BUFF_SIZE 104800
#define MSG1 "GET http://www.cplusplus.com/reference/unordered_map/unordered_map/ HTTP/1.1\r\nHost: www.cplusplus.com\r\nProxy-Connection: Keep-Alive\r\n\r\n"
#define MSG2 "GET http://www.sina.com.cn/ HTTP/1.1\r\nHost: www.sina.com.cn\r\nProxy-Connection: keep-alive\r\n\r\n"
#define MSG3 "GET http://stackoverflow.com/questions/37907986/error-in-recover-free-invalid-next-size-normal HTTP/1.1\r\nHost: stackoverflow.com\r\nProxy-Connection: Keep-Alive\r\n\r\n"
#define MSG4 "GET http://beej.us/guide/bgnet/output/html/multipage/getaddrinfoman.html HTTP/1.1\r\nHost: beej.us\r\nProxy-Connection: Keep-Alive\r\n\r\n"
#define MSG5 "GET http://pubs.opengroup.org/onlinepubs/9699919799/functions/strncat.html HTTP/1.1\r\nHost: pubs.opengroup.org\r\nProxy-Connection: Keep-Alive\r\n\r\n"
#define MSG6 "GET http://www.gnu.org/software/gsl HTTP/1.1\r\nHost: www.gnu.org\r\nProxy-Connection: Keep-Alive\r\n\r\n"
#define MSG7 "GET http://man7.org/linux/man-pages/man3/pthread_create.3.html HTTP/1.1\r\nHost: man7.org\r\nProxy-Connection: Keep-Alive\r\n\r\n"

int main(int argc, char const *argv[])
{
    if(argc != 3){
        perror("Input port and No. of message\n");
        exit(EXIT_FAILURE);
    }
    int po = atoi(argv[1]);
    if(po < 1024){
        perror("Engaging reserved port is banned\n");
        exit(EXIT_FAILURE);
    }
    char *buff;
    switch(atoi(argv[2])){
    case 1:
	buff = MSG1;
	break;
    case 2:
	buff = MSG2;
	break;
    case 3:
	buff = MSG3;
	break;
    case 4:
	buff = MSG4;
	break;
    case 5:
	buff = MSG5;
	break;
    case 6:
	buff = MSG6;
	break;
    case 7:
	buff = MSG7;
	break;
    default:
	buff = MSG5;
	break;
    }
    struct sockaddr_in client;
    unsigned int socket_len = sizeof(client);
    
    /* Create socket */
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0){
        perror("Failed to create socket\n");
        exit(EXIT_FAILURE);
    }

    memset(&client, 0, socket_len);
    client.sin_family = AF_INET;
    client.sin_port = htons(po);
    int ip_status = inet_pton(AF_INET, "127.0.0.1", &client.sin_addr);
    if(ip_status != 1){
        perror("Failed to construct socket\n");
        exit(EXIT_FAILURE);
    }

    /* Connect */
    int conn_fd = connect(socket_fd, (struct sockaddr *)&client, socket_len);
    if(conn_fd < 0){
        perror("Connection error\n");
        exit(EXIT_FAILURE);
    }
    printf("Build connection\n");
    int send_status = send(socket_fd, buff, strlen(buff)+1, 0);
    if(send_status < 0){
        perror("Failed to send message\n");
        exit(EXIT_FAILURE);
    }
    printf("Message is successfully sent\n");
    printf("%s\n", buff);
    char newbuff[BUFF_SIZE];
    int receive_len;
    char* ret = malloc(sizeof(char));
    memset(ret, 0, sizeof(char));
    do{
        memset(newbuff, '\0', BUFF_SIZE);
        receive_len = read(socket_fd, newbuff, BUFF_SIZE);
        printf("receive length = %d\n", receive_len);
        printf("newbuff length = %lu\n", strlen(newbuff));
        if(receive_len < 0){
            perror("Failed to receive response\n");
            printf("%d: %s\n", errno, strerror(errno));
            close(socket_fd);
            free(ret);
            exit(EXIT_FAILURE);
        }
        ret = realloc(ret, strlen(ret)+receive_len+1);
        strcat(ret, newbuff);
    }while(receive_len != 0);
    printf("Response length %lu\n", strlen(ret));
    printf("Response:\n%s\n", ret);
    close(socket_fd);
    free(ret);
    return EXIT_SUCCESS;
}
