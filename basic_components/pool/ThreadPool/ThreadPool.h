#include<vector>
#include<atomic>
#include<thread>
#include<queue>
#include<functional>
#include<unistd.h>
#include<mutex>
#include<iostream>
#include<sys/syscall.h>
#include<condition_variable>

class ThreadPool
{
private:
    std::vector<std::thread>threads_; //线程池钟的线程
    std::queue<std::function<void()>>taskqueue_; //任务队列
    std::mutex mutex_;  //任务队列同步互斥锁
    std::condition_variable conditon_;  // 任务队列同步的条件变量
    std::atomic_bool stop_; 


public:
    ThreadPool(size_t threadnum);
    ~ThreadPool();
    void addtask(std::function<void()>fn);
};


