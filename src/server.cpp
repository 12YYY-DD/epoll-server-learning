#include <iostream>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <sys/epoll.h>
#include <fcntl.h>

using namespace std;

void setNonBlocking(int fd){// 设置 fd 为非阻塞模式
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}


int main() {
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

                    epoll_event ev_client{};
                    ev_client.events = EPOLLIN;
                    ev_client.data.fd = connfd;

                    epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev_client);
                }
            }
            else {// 客户端有事件，说明有数据可读
               while (true) {
                    char buf[1024];
                    int len = recv(fd, buf, sizeof(buf), 0);

                    if (len > 0) {
                        cout << "recv fd=" << fd << " msg=" << string(buf, len) << endl;
                        send(fd, buf, len, 0);
                    }
                    else if (len == 0) {
                        cout << "client closed: " << fd << endl;
                        close(fd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
                        break;
                    }
                    else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break; // 数据读完了
                        } else {
                            epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
                            close(fd);
                            break;
                        }
                    }
                }
            }
        }
    }

    close(sockfd);
    return 0;
}