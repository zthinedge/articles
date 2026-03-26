#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

using namespace std;

class TaskQueue {
private:
    queue<function<void()>> tasks_;
    mutex mtx_;
    condition_variable cv_;
    bool running =false;
    vector<thread>workers_;
    TaskQueue() = default;
    ~TaskQueue() { 
        stop(); 
        for(auto &t:workers_){
            t.join();
        }
    }
    TaskQueue(const TaskQueue&) = delete;
    TaskQueue& operator=(const TaskQueue&) = delete;
    void workerLoop(){
        while(true){
            function<void()>task;
            {
                unique_lock<mutex>lock(mtx_);
                cv_.wait(lock,[this]{return !tasks_.empty()||!running;});
                if(tasks_.empty()&&!running)return ;

                task=move(tasks_.front());
                tasks_.pop();
            }
            if(task)task();
        }
    }
    

public:
    static TaskQueue* getInstance() {
        static TaskQueue q;
        return &q;
    }

    void start() {
        unique_lock<mutex>lock(mtx_);
        if(running)return ;
        running=true;
        workers_.emplace_back([this]{this->workerLoop();});
    }

    bool push(function<void()> task) {
       {
            unique_lock<mutex>lock(mtx_);
            if(!running)return false;
            tasks_.push(move(task));
       }
       cv_.notify_one();
       return true;
    }

    void stop() {
        {
            unique_lock<mutex>lock(mtx_);   
            if(!running)return ;
            running =false;
        }
        cv_.notify_all();
    }
};

int main() {
    TaskQueue* q = TaskQueue::getInstance();
    q->start();
    q->push([]{cout<<"task 1 "<<endl;});
    q->push([]{cout<<"task 2 "<<endl;});
    this_thread::sleep_for(chrono::milliseconds(100));
    q->stop();
    return 0;
}
