#pragma once
#include <map>

#include "threadpool.h"
#include "connection.h"

using namespace std;

class Reactor {
public:
    Reactor(int port);
    void run();
private:
    int sockfd;
    int epfd;
    ThreadPool pool;
    map<int, Connection*> conns;

    void handleAccept();
    void handleRead(int fd);
    void closeConnection(int fd);
};

