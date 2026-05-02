#pragma once

#include <string>   

void setNonBlocking(int fd);// 设置 fd 为非阻塞模式
bool isWouldBlock();// 判断 errno 是否是 EAGAIN 或 EWOULDBLOCK，表示操作会阻塞
void log(const std::string& level, const std::string& msg);// 简单的日志函数，输出日志级别和消息