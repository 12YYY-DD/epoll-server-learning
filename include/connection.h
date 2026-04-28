#pragma once
#include <string>

using namespace std;



class Connection {//补充数据和状态
public:
    int fd;
    string buffer;//缓存数据

    Connection(int fd_);

};