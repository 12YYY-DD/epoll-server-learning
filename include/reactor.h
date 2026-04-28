#pragma once
#include <map>
#include "connection.h"

using namespace std;

class Reactor {
public:
    Reactor(int port);
    void run();
private:
    int sockfd;
    int epfd;
    map<int, Connection*> conns;

    void handleAccept();
    void handleRead(int fd);
    void closeConnection(int fd);
};

