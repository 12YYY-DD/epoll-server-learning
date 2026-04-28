#include "utils.h"

#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>

using namespace std;

void setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

bool isWouldBlock() {
    return errno == EAGAIN || errno == EWOULDBLOCK;
}

void log(const string& level, const string& msg) {
    cout << "[" << level << "] " << msg << std::endl;
}