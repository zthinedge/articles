#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define PORT 8888

void *client_thread(void *arg) {
    int client_fd = *(int*)arg;
    free(arg); 
    
    while(1) {
        char buf[128] = {0};
        int recv_len = recv(client_fd, buf, sizeof(buf), 0);
        if(recv_len <= 0) {
            break;
        }
        send(client_fd, buf, recv_len, 0);
    }
    close(client_fd);
    return NULL;
}

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        perror("socket error");
        return -1;
    }
    
    // 设置SO_REUSEADDR选项以避免地址使用错误
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
    
    while(1) {
        struct sockaddr_in client_addr;
        socklen_t sock_len = sizeof(client_addr);
        
        int client_fd = accept(sockfd, (struct sockaddr*)&client_addr, &sock_len);
        if(client_fd == -1) {
            perror("accept error");
            continue; // 继续接受其他连接而不是退出
        }
        
        pthread_t tid;
        if(pthread_create(&tid, NULL, client_thread, client_fd) != 0) {
            perror("pthread_create error");
            close(client_fd);
            continue;
        }
        // 分离线程，避免僵尸线程
        //僵尸进程是指已经完成执行（即已经终止），但其父进程尚未对其进行善后处理（如读取其退出状态）的进程 pthread_detach 能让线程的资源在结束时自动回收
        pthread_detach(tid);
    }
    
    close(sockfd);
    return 0;
}
