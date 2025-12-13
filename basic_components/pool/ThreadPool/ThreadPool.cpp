#include "ThreadPool.h"
ThreadPool::ThreadPool(size_t threadnum):stop_(false){
    //启动threadnum个线程，将每一个线程阻塞在条件变量上
    for(int i=0;i<threadnum;i++){
        //创建工作线程并加入线程池
        threads_.emplace_back([this]
        {
            printf("create thread(%ld).\n",syscall(SYS_gettid));     
            std::cout << "子线程：" << std::this_thread::get_id() << std::endl;
            while(stop_=false){
                std::function<void()>task;  //存放从队列中取出的任务
                {
                    std::unique_lock<std::mutex>lock(this->mutex_);
                    //等待生产者的条件变量，休眠等待唤醒
                    this->conditon_.wait(lock,[this]
                    {
                        return ((this->stop_==true)||(this->taskqueue_.empty()==false));//线程池停止或任务队列有任务
                    });

                    // 在线程池停止之前，如果队列中还有任务，执行完再退出。
					if ((this->stop_==true)&&(this->taskqueue_.empty()==true)) return;

                    // 出队一个任务。
                    task = std::move(this->taskqueue_.front()); 
                    this->taskqueue_.pop();
                }
                
                printf("thread is %ld.\n",syscall(SYS_gettid));
				task();  // 执行任务。
            }

        });
    }

}

ThreadPool::~ThreadPool(){
    stop_=true;
    conditon_.notify_all(); //唤醒全部线程
    for(std::thread &th:threads_){
        th.join();
    }
}

void ThreadPool::addtask(std::function<void()>fn){
    {
        std::lock_guard<std::mutex>lock(mutex_);
        taskqueue_.push(fn);
    }
    conditon_.notify_one();
}