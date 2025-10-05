#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include<sys/select.h>
#include<sys/poll.h>
#define PORT 8888

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        perror("socket error");
        return -1;
    }
    
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if(bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind error");
        close(sockfd);
        return -1;
    }
    
    if(listen(sockfd, 10) < 0) {
        perror("listen error");
        close(sockfd);
        return -1;
    }

    struct pollfd fds[1024]={0};
    //初始化监听套接字的pollfd结构
    fds[sockfd].fd=sockfd;
    fds[sockfd].events=POLLIN;//--events:需要关注的事件 POLLIN:可读
    int maxfd=sockfd;

    while(1){
        int nready=poll(fds,maxfd+1,-1);
        //revents:由内核填充的实际发生的事件
        if(fds[sockfd].revents&POLLIN){
            struct sockaddr_in clientaddr;
            socklen_t len=sizeof(clientaddr);

            int clientfd=accept(sockfd,(struct sockaddr*)&clientaddr,&len);
            printf("sockfd:%d\n",clientfd);
            fds[clientfd].fd=clientfd;
            fds[clientfd].events=POLLIN;
            maxfd=clientfd;
        }

        int i=0;
        for(i=sockfd+1;i<=maxfd;i++){
            if(fds[i].revents&POLLIN){
                char buf[128] = {0};
                int recv_len = recv(i, buf, 128, 0);
                if(recv_len <= 0) {
                    printf("disconnect\n");
                    fds[i].fd=-1;//poll忽略fd=-1的条目
                    fds[i].revents=0;
                    close(i);
                    continue;
                }
                send(i, buf, recv_len, 0); 
                printf("clientfd:%d  recv_len :%d buffer :%s\n",i,recv_len,buf);
            }
        }
    }
    return 0;
}
