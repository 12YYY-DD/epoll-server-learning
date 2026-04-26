#include <iostream>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <sys/epoll.h>
#include <fcntl.h>
#include <map>

using namespace std;

class Connection {//补充数据和状态
public:
    int fd;
    string buffer;//缓存数据

    Connection(int fd_) : fd(fd_) {}

};

void setNonBlocking(int fd){// 设置 fd 为非阻塞模式
    int flags = fcntl(fd, F_GETFL, 0);// 获取当前 fd 的 flags
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void handleAccept(int sockfd, int epfd, map<int, Connection*>& conns){
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
    }
}

void handleRead(int fd, int epfd,map<int, Connection*>& conns){
    Connection* conn = conns[fd];// 获取连接对象

    char buf[1024];
    while (true) {
        
        int len = recv(fd, buf, sizeof(buf), 0);

        if (len > 0) {
            //不直接处理存起来
            conn->buffer.append(buf, len);
        }
        else if (len == 0) {
            cout << "client closed: " << fd << endl;
            epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
            close(fd);
            
            delete conn;
            conns.erase(fd);
            break;
        }
        else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; // 数据读完了
            } else {
                epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
                close(fd);
                delete conn;
                conns.erase(fd);
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
        send(fd, msg.c_str(), msg.size(), 0);// 回显给客户端
    }
}

int main() {
    map<int, Connection*> connections;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    setNonBlocking(sockfd);// 设置服务器 fd 为非阻塞模式
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (sockaddr*)&addr, sizeof(addr));
    listen(sockfd, 10);

    // 1. 创建 epoll
    int epfd = epoll_create(1);

    // 2. 把服务器 fd 加进去
    epoll_event ev{};
    ev.events = EPOLLIN;// 监听可读事件
    ev.data.fd = sockfd;// 关联服务器 fd
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);// 把服务器 fd 加入 epoll 监听

    epoll_event events[1024];

    cout << "epoll server start...\n";

    while (true) {
        // 3. 等待事件
        int n = epoll_wait(epfd, events, 1024, -1);// 阻塞等待事件发生

        for (int i = 0; i < n; i++) {// 遍历发生事件的 fd
            int fd = events[i].data.fd;// 只获取发生事件的 fd

            // 有新连接
            if (fd == sockfd) {// 服务器 fd 有事件，说明有新连接
                handleAccept(sockfd, epfd, connections);
            }
            else {// 客户端有事件，说明有数据可读
                handleRead(fd, epfd, connections);
            }
        }
    }

    close(sockfd);
    return 0;
}