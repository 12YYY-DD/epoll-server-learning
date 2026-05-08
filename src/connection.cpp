#include "connection.h"

Connection::Connection(int fd_) : fd(fd_), buffer("") , writebuffer("") {
    lastActive = chrono::steady_clock::now();// 初始化上次活跃时间为当前时间
}// 构造函数初始化 fd，buffer 默认空字符串