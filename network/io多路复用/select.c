#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include<sys/select.h>
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
    
    fd_set rfds,rset;
    FD_ZERO(&rfds);
    FD_SET(sockfd,&rfds);

    int maxfd=sockfd;
    printf("loop\n");
    while(1){
        rset=rfds;//副本  -- select会修改文件描述符集合
        int nready=select(maxfd+1,&rset,NULL,NULL,NULL);//返回事件的个数

        if(FD_ISSET(sockfd,&rset)){
           
            struct sockaddr_in clientaddr;
            socklen_t len=sizeof(clientaddr);

            int clientfd=accept(sockfd,(struct sockaddr*)&clientaddr,&len);
            printf("sockfd：%d\n",clientfd);
            FD_SET(clientfd,&rfds);//放入总集
            maxfd=clientfd;
        }
        int i=0;
        for(i=sockfd+1;i<=maxfd;i++){
            if(FD_ISSET(i,&rset)){
               char buf[128] = {0};
                int recv_len = recv(i, buf, 128, 0);
                if(recv_len <= 0) {
                    printf("disconnect\n");
                    FD_CLR(i,&rfds);//不需要再监控该客户端
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
