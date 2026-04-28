#include "connection.h"

Connection::Connection(int fd_) : fd(fd_), buffer("") {}// 构造函数初始化 fd，buffer 默认空字符串