#pragma once
#include <map>

#include "threadpool.h"
#include "connection.h"

using namespace std;

class Reactor {
public:
    Reactor(int port);// 构造函数，初始化服务器
    void run();// 运行服务器，进入事件循环
private:
    int sockfd;// 服务器 fd
    int epfd;// epoll fd
    ThreadPool pool;
    map<int, Connection*> conns;// fd 到连接对象的映射

    void handleAccept();
    void handleRead(int fd);
    void closeConnection(int fd);
    void handleWrite(int fd);
};

