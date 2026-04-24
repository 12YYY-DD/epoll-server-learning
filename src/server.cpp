#include <iostream>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <sys/epoll.h>

using namespace std;

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (sockaddr*)&addr, sizeof(addr));
    listen(sockfd, 10);

    // 🔥 1. 创建 epoll
    int epfd = epoll_create(1);

    // 2. 把服务器 fd 加进去
    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

    epoll_event events[1024];

    cout << "epoll server start...\n";

    while (true) {
        // 🔥 3. 等待事件
        int n = epoll_wait(epfd, events, 1024, -1);

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            // 👉 有新连接
            if (fd == sockfd) {
                int connfd = accept(sockfd, nullptr, nullptr);
                cout << "new client: " << connfd << endl;

                epoll_event ev_client{};
                ev_client.events = EPOLLIN;
                ev_client.data.fd = connfd;

                epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev_client);
            }
            else {
                // 👉 客户端有数据
                char buf[1024] = {0};
                int len = recv(fd, buf, sizeof(buf), 0);

                if (len <= 0) {
                    cout << "client closed: " << fd << endl;
                    close(fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
                }
                else {
                    cout << "recv fd=" << fd << " msg=" << buf << endl;
                    send(fd, buf, len, 0);
                }
            }
        }
    }

    close(sockfd);
    return 0;
}