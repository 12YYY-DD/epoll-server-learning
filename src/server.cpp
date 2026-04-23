#include <iostream>
#include<string>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<cstring>
#include<string>
#include<vector>
using namespace std;

int main(){
    //1.创建socket套接字
    int sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);//AF_INET:IPv4协议族，SOCK_STREAM:面向连接的TCP协议，IPPROTO_TCP:TCP协议
    if(sockfd<0)
    {
        printf("socket error:errno=%d errmsg=%s\n",errno,strerror(errno));
        return -1;
    }
    else
    {
        printf("socket success\n");
    }
    //2.绑定socket
    string ip = "127.0.0.1";
    int port = 8080;

    struct sockaddr_in sockaddr;//定义一个sockaddr_in结构体变量，用于存储服务器的IP地址和端口号
    memset(&sockaddr,0,sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;//设置地址族为AF_INET，表示使用IPv4协议
    sockaddr.sin_addr.s_addr = inet_addr(ip.c_str());//将字符串形式的IP地址转换为网络字节序的二进制形式，并赋值给sockaddr.sin_addr.s_addr
    sockaddr.sin_port = htons(port);//将端口号转换为网络字节序，并赋值给sockaddr.sin_port
    if(bind(sockfd,(struct sockaddr *)&sockaddr,sizeof(sockaddr))<0)
    {
        printf("socket bind error:errno=%d errmsg=%s\n",errno,strerror(errno));
        return -1;  
    }
    else
    {
        printf("socket bind success: ip=%s port=%d\n",ip.c_str(),port);
    }
    //3.监听socket
    if(listen(sockfd,1024)<0)
    {
        printf("socket listen error:errno=%d errmsg=%s\n",errno,strerror(errno));
        return -1;  
    }
    else
    {
        printf("socket listening...\n");
    }
    //4.接受客户端连接
vector<int> clients;

while (true)
{
    int connfd = accept(sockfd, nullptr, nullptr);
    if (connfd >= 0)
    {
        printf("new client: %d\n", connfd);
        clients.push_back(connfd);
    }


    for (auto it = clients.begin(); it != clients.end(); )
    {
        int fd = *it;
        char buf[1024] = {0};

        ssize_t len = recv(fd, buf, sizeof(buf), 0); // 用 fd

        if (len > 0)
        {
            printf("recv client fd=%d msg=%s\n", fd, buf);
            send(fd, buf, len, 0);
            ++it;
        }
        else if (len == 0)
        {
            printf("client closed fd=%d\n", fd);
            close(fd);
            it = clients.erase(it);
        }
        else
        {
            printf("recv error fd=%d errno=%d\n", fd, errno);
            close(fd);
            it = clients.erase(it);
        }
    }
}
    //7.关闭连接
    close(sockfd);
    return 0;
}