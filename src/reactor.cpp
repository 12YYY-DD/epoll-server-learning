#include "reactor.h"
#include "utils.h"

#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <arpa/inet.h>
#include <sys/epoll.h>


using namespace std;

Reactor::Reactor(int port){
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    setNonBlocking(sockfd);// 设置服务器 fd 为非阻塞模式
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (sockaddr*)&addr, sizeof(addr));
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    listen(sockfd, 10);

    // 1. 创建 epoll
    epfd = epoll_create(1);

    // 2. 把服务器 fd 加进去
    epoll_event ev{};
    ev.events = EPOLLIN;// 监听可读事件
    ev.data.fd = sockfd;// 关联服务器 fd
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);// 把服务器 fd 加入 epoll 监听

   

    log("INFO", "Server started");
}

void Reactor:: run(){
     epoll_event events[1024];
    while (true) {
        // 3. 等待事件
        int n = epoll_wait(epfd, events, 1024, -1);// 阻塞等待事件发生

        for (int i = 0; i < n; i++) {// 遍历发生事件的 fd
            int fd = events[i].data.fd;// 只获取发生事件的 fd

            if (fd != sockfd && (events[i].events & (EPOLLERR | EPOLLHUP))) {// 客户端发生错误或挂起，关闭连接
                closeConnection(fd);
                continue;
            }

            // 有新连接
            if (fd == sockfd) {// 服务器 fd 有事件，说明有新连接
                handleAccept();
            }
            else {// 客户端有事件，说明有数据可读
                handleRead(fd);
            }
        }
    }
}



void Reactor:: handleAccept(){
    while (true) {
        int connfd = accept(sockfd, nullptr, nullptr);
        if (connfd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; // 没有更多连接了
            } else {
                perror("accept");
                break;
            }
        }
        setNonBlocking(connfd);
        cout << "new client: " << connfd << endl;
        // 创建连接对象并保存
        conns[connfd] = new Connection(connfd);
        epoll_event ev_client{};
        ev_client.events = EPOLLIN;
        ev_client.data.fd = connfd;

        epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev_client);
        log("INFO", "new client fd=" + std::to_string(connfd));
    }
}

void Reactor::handleRead(int fd){
    auto it = conns.find(fd);
    if (it == conns.end()) {
        log("ERROR", "fd not found");
        return;
    }
    Connection* conn = it->second;
    //Connection* conn = conns[fd];// 获取连接对象

    char buf[1024];
    while (true) {
        
        int len = recv(fd, buf, sizeof(buf), 0);

        if (len > 0) {
            //不直接处理存起来
            conn->buffer.append(buf, len);
        }
        else if (len == 0) {
            cout << "client closed: " << fd << endl;
            closeConnection(fd);
            break;
        }
        else {
            if (isWouldBlock()) {
                break; // 数据读完了
            } else {
                closeConnection(fd);
                break;
            }
        }
    }
    //处理buffer中的数据
    size_t pos;// 处理完整的请求
    while((pos = conn->buffer.find('\n')) != string::npos){
        string msg = conn->buffer.substr(0, pos);
        conn->buffer.erase(0, pos + 1);

        cout<<"完整消息："<<msg<<endl;
        size_t total = 0;
        while (total < msg.size()) {
            ssize_t n = send(fd, msg.c_str() + total, msg.size() - total, 0);
            if (n <= 0) break;
            total += n;
        }// 处理完全发送给客户端
    }
}

void Reactor:: closeConnection(int fd){// 关闭连接，清理资源
    log("INFO", "close fd=" + std::to_string(fd));
    cout << "close client: " << fd << endl;
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);// 关闭 fd
    close(fd);
    delete conns[fd];
    conns.erase(fd);
}