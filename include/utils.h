#pragma once

#include <string>   

void setNonBlocking(int fd);
bool isWouldBlock();
void log(const std::string& level, const std::string& msg);