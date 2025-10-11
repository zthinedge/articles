#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include<sys/select.h>
#include<sys/poll.h>
#include<sys/epoll.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#define PORT 2048
#define BUFFER_SIZE 1024 
#define ENABLE_HTTP_RESPONSE  1
#define ROOT_DIR "/home/zym/Share/2.1.1-multi-io"

typedef int (*RCALLBACK)(int fd);
struct conn_item{
    int fd;
    char rbuffer[BUFFER_SIZE];
    int rlen;
    char wbuffer[BUFFER_SIZE];
    int wlen;
    union{
        RCALLBACK accept_callback;
        RCALLBACK recv_callback;
    }recv_t;
    RCALLBACK send_callback;
};
#if ENABLE_HTTP_RESPONSE
typedef struct conn_item connction_t;
int http_response(connction_t *conn){

    int filefd=open("index.html",O_RDONLY);
    struct stat stat_buf;
    fstat(filefd,&stat_buf);
    conn->wlen=sprintf(conn->wbuffer, 
        "HTTP/1.1 200 OK\r\n"
        "Server: MyHTTPServer/1.0\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: %ld\r\n"
        "Connection: close\r\n"
        "\r\n",  // 空行分隔头部和主体
        stat_buf.st_size);
    int count=read(filefd,conn->wbuffer+conn->wlen,BUFFER_SIZE-conn->wlen);
    conn->wlen+=count;
    close(filefd);
    return conn->wlen;
}
#endif

int accept_cb(int fd);
int recv_cb(int fd);
int send_cb(int fd);
int set_event(int fd,int event,int flag);
int epfd=0;
struct conn_item connlist[1024]={0};
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


    connlist[sockfd].fd=sockfd;
    connlist[sockfd].recv_t.accept_callback=accept_cb;
    epfd=epoll_create(1);
    set_event(sockfd,EPOLLIN,1);

    struct epoll_event events[1024]={0};

    while(1){
        int nready=epoll_wait(epfd,events,1024,-1);
        int i=0;
        for(i=0;i<nready;i++){
            int connfd=events[i].data.fd;
            
            if(events[i].events&EPOLLIN){
                int count =connlist[connfd].recv_t.recv_callback(connfd);
                printf("recv count:%d <-- buffer:%s\n",count,connlist[connfd].rbuffer);
            }else if(events[i].events&EPOLLOUT){
                int count =connlist[connfd].send_callback(connfd);
                printf("send --> buffer:%s\n",connlist[connfd].wbuffer);

            }
        }
    }
    
    return 0;
}
int accept_cb(int fd){
    struct sockaddr_in clientaddr;
    socklen_t len=sizeof(clientaddr);

    int clientfd=accept(fd,(struct sockaddr*)&clientaddr,&len);
    if(clientfd<0){
        return -1;
    }
    set_event(clientfd,EPOLLIN,1);
    connlist[clientfd].fd=clientfd;
    memset(connlist[clientfd].wbuffer,0,sizeof(BUFFER_SIZE));
    memset(connlist[clientfd].rbuffer,0,sizeof(BUFFER_SIZE));
    connlist[clientfd].wlen=0;
    connlist[clientfd].rlen=0;
    connlist[clientfd].recv_t.recv_callback=recv_cb;
    connlist[clientfd].send_callback=send_cb;
    return clientfd;

}
int recv_cb(int fd){
    char*buffer=connlist[fd].rbuffer;
    int idx=connlist[fd].rlen;

    int count = recv(fd, buffer+idx, BUFFER_SIZE-idx, 0);
    if(count <= 0) {
        printf("disconnect\n");
        epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL);
        close(fd);
       
    }
    connlist[fd].rlen+=count;
    #if 0 
    memcpy(connlist[fd].wbuffer,connlist[fd].rbuffer,connlist[fd].rlen);
    connlist[fd].wlen=connlist[fd].rlen;
    #else
    //http_request(&connlist[fd]);
    http_response(&connlist[fd]);
    
    #endif
    set_event(fd,EPOLLOUT,0);
   
    return count;
}
int send_cb(int fd){
    char*buffer=connlist[fd].wbuffer;
    int idx=connlist[fd].wlen;
    int count=send(fd,buffer,idx,0);
    set_event(fd,EPOLLIN,0);
    return count;
}
int set_event(int fd,int event,int flag){
    //1 add 0 mod
    if(flag){
        struct epoll_event ev;
        ev.events=event;
        ev.data.fd=fd;
        epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev);//一个fd对应一个事件
    }else{
        struct epoll_event ev;
        ev.events=event;
        ev.data.fd=fd;
        epoll_ctl(epfd,EPOLL_CTL_MOD,fd,&ev);
    }
    return 0;
}
