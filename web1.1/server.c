#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

struct SockInfo
{
    struct sockaddr_in addr;
    int fd;
};

struct SockInfo infos[512];

void* working(void* arg);


int main() {
    //1.创建监听的套接字
    int fd = socket(AF_INET, SOCK_STREAM,0);
    if(fd == -1){
        perror("socket");
        return -1;
    }

    //2.绑定本地的IP Port
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(9999);   //转换成大端
    saddr.sin_addr.s_addr = INADDR_ANY; //0 = 0.0.0.0 可以绑定本地任意ip地址，读实际网卡IP地址

    int ret = bind(fd, (struct sockaddr*) &saddr, sizeof(saddr));
    if(ret == -1) {
        perror("bind");
        return -1;
    }

    //3.设置监听
    ret = listen(fd, 128);
    if(ret == -1) {
        perror("listen");
        return -1;
    }

    int max = sizeof(infos)/sizeof(infos[0]);
    for(int i = 0; i < max; i++){
        bzero(&infos[i],sizeof(infos[i]));
        infos[i].fd = -1;
    }

    //4.阻塞并等待客户端的连接
    int addrlen = sizeof(struct sockaddr_in);
    while(1){
        struct SockInfo* pinfo;
        for(int i = 0; i < max; ++i){
            if(infos[i].fd == -1){
                pinfo = &infos[i];
                break;
            }
        }
        int cfd = accept(fd, (struct sockaddr*) &pinfo->addr, &addrlen);
        pinfo->fd = cfd;
        if(cfd == -1) {
            perror("accept");
            break;
        }
        //创建子线程
        pthread_t tid;
        pthread_create(&tid, NULL, working, pinfo);
        pthread_detach(tid);
    }
    close(fd);
    return 0;
}

void* working(void* arg)
{
    struct SockInfo* pinfo = (struct SockInfo*)arg;
    //连接建立成功，打印客户端的IP和端口信息,将网络字节序二进制值转换成点分十进制串
    char ip[32];

    printf("客户端的IP: %s     端口:  %d\n",
    inet_ntop(AF_INET, &pinfo->addr.sin_addr.s_addr, ip, sizeof(ip)),ntohs(pinfo->addr.sin_port));

    //5.通信
    while(1) {
        //接受数据
        char buff[1024];
        int len = recv(pinfo->fd, buff, sizeof(buff), 0);
        if(len > 0) {
            printf("client say: %s\n", buff);
            send(pinfo->fd, &buff, len, 0);
        }
        else if (len == 0) {
            printf("客户端已经断开连接...\n");
            break;
        }
        else{
            perror("recv");
            break;
        }
    }

    //关闭文件描述符
    close(pinfo->fd);
    pinfo->fd = -1;
    return NULL;
}