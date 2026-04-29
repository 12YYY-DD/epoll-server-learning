#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

class ThreadPool {
public:
    ThreadPool(size_t threadCount);// 构造函数，创建线程池
    ~ThreadPool();// 析构函数，销毁线程池

    void addTask(std::function<void()> task);// 添加任务到线程池
private:
    std::vector<std::thread> workers;// 工作线程
    std::queue<std::function<void()>> tasks;// 任务队列

    std::mutex mtx;// 任务队列的互斥锁
    std::condition_variable cv;// 条件变量，用于通知工作线程有新任务
    bool stop;// 标志线程池是否停止

};