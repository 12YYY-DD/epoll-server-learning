#pragma once
#include <map>
#include <queue>
#include <functional>
#include <mutex>    

#include "threadpool.h"
#include "connection.h"

using namespace std;

class Reactor {
public:
    Reactor(int port);// 构造函数，初始化服务器
    void run();// 运行服务器，进入事件循环
    void addTask(function<void()> task);// 添加任务到线程池
    void handlePendingTasks();// 处理待执行的任务
private:
    int sockfd;// 服务器 fd
    int epfd;// epoll fd
    ThreadPool pool;
    map<int, Connection*> conns;// fd 到连接对象的映射
    queue<function<void()>> pendingTasks;// 待执行的任务队列
    mutex taskMutex;// 任务队列的互斥锁

    void handleAccept();
    void handleRead(int fd);
    void closeConnection(int fd);
    void handleWrite(int fd);
};

