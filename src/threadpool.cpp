#include "threadpool.h"

ThreadPool::ThreadPool(size_t threadCount): stop(false) {// 构造函数，创建线程池
    for (size_t i = 0; i < threadCount; ++i) {// 创建指定数量的工作线程
        workers.emplace_back([this] {// 工作线程的函数
            while (true) {
                std::function<void()> task;// 定义一个任务函数对象
                {
                    std::unique_lock<std::mutex> lock(mtx);// 加锁保护任务队列
                    cv.wait(lock, [this] { return stop || !tasks.empty(); });// 等待条件变量通知，直到线程池停止或有任务可执行
                    if (stop && tasks.empty()) return;// 如果线程池停止且没有任务，退出线程
                    task = std::move(tasks.front());// 获取任务队列中的第一个任务
                    tasks.pop();// 从任务队列中移除该任务
                }
                task();// 执行任务
            }
        });
    }
}

void ThreadPool::addTask(std::function<void()> task) {// 添加任务到线程池
    {
        std::lock_guard<std::mutex> lock(mtx);// 加锁保护任务队列
        tasks.push(task);// 将任务添加到任务队列
    }
    cv.notify_one();// 通知一个工作线程有新任务可执行
}

ThreadPool::~ThreadPool() {// 析构函数，销毁线程池
    {
        std::lock_guard<std::mutex> lock(mtx);// 加锁保护任务队列
        stop = true;// 设置停止标志，通知工作线程退出
    }
    cv.notify_all();// 通知所有工作线程退出
    for (auto& t : workers) {// 等待所有工作线程完成
        t.join();
    }
}