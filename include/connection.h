#pragma once
#include <string>
#include <chrono>
using namespace std;



class Connection {//补充数据和状态
public:
    int fd;
    string buffer;//缓存数据
    string writebuffer;//待发送数据
    chrono::steady_clock::time_point lastActive;// 上次活跃时间
    Connection(int fd_);// 构造函数声明

};