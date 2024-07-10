#include "Server.h"
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <fcntl.h>

int initListenFd(unsigned short port)
{
    //1、创建监听的fd
    int lfd = socket(AF_INET, SOCK_STREAM, 0);   //ipv4 流式
    if(lfd == -1)
    {
        perror("socket");
        return -1;
    }
    //2、设置端口复用
    int opt = 1;
    int ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if(ret == -1)
    {
        perror("setsockopt");
        return -1;
    }
    //3、绑定
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    ret = bind(lfd, (struct sockaddr*)&addr, sizeof(addr));
    if(ret == -1)
    {
        perror("bind");
        return -1;
    }
    //4、设置监听
    ret = listen(lfd, 128);
    if(ret == -1)
    {
        perror("listen");
        return -1;
    }
    //返回fd
    return lfd;
}

int epollRun(int lfd)
{
    //1.创建epoll实例
    int epfd = epoll_create(1);
    if(epfd == -1)
    {
        perror("epoll_create");
        return -1;
    }
    //2.lfd上树
    struct epoll_event ev;  
    ev.data.fd = lfd;
    ev.events = EPOLLIN;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
    if(ret == -1)
    {
        perror("epoll_ctl");
        return -1;
    }
    //3.检测
    struct epoll_event evs[1024];
    int size = sizeof(evs) / sizeof(struct epoll_event);
    while(1)
    {
        int num = epoll_wait(epfd, evs, size, -1);
        for(int i = 0; i < num; i++)
        {
            int fd = evs[i].data.fd;
            if (fd == lfd)
            {
                //建立新连接 accept
                acceptClient(lfd,epfd);
                
            }
            else
            {
                //主要是接收对端的数据
            }
        }
    }
}

int acceptClient(int lfd, int epfd)
{
    //1.建立连接
    int cfd = accept(lfd, NULL, NULL);
    if(cfd == -1)
    {
        perror("accept");
        return -1;
    }

    //2.设置非阻塞
    int flag = fcntl(cfd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(cfd, F_SETFL, flag);

    //3.cfd添加到epoll中
    struct epoll_event ev;  
    ev.data.fd = cfd;
    ev.events = EPOLLOUT | EPOLLET;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
    if(ret == -1)
    {
        perror("epoll_ctl");
        return -1;
    }

}
