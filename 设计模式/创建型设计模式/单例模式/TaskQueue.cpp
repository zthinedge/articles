#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

using namespace std;

class TaskQueue {
private:
    queue<function<void()>> tasks_;
    mutex mtx_;
    condition_variable cv_;
    bool running = false;
    vector<thread> workers_;
    mutex cout_mtx;
    TaskQueue() = default;
    ~TaskQueue() {
        stop();
        for (auto& w : workers_) {
            if (w.joinable()) w.join();
        }
    }
    TaskQueue(const TaskQueue&) = delete;
    TaskQueue& operator=(const TaskQueue&) = delete;

    void workerLoop() {
        while (true) {
            function<void()> task;
            {
                unique_lock<mutex> lock(mtx_);
                cv_.wait(lock, [this] { return !tasks_.empty() || !running; });
                if (tasks_.empty() && !running) return;
                if (tasks_.empty()) continue;
                task = move(tasks_.front());
                tasks_.pop();
            }
            if (task) task();
        }
    }

public:
    static TaskQueue* getInstance() {
        static TaskQueue q;
        return &q;
    }

    void start(size_t threadCount = 4) {
        unique_lock<mutex> lock(mtx_);
        if (running) return;
        running = true;
        for (size_t i = 0; i < threadCount; ++i) {
            workers_.emplace_back([this] { this->workerLoop(); });
        }
    }

    bool push(function<void()> task) {
        {
            unique_lock<mutex> lock(mtx_);
            if (!running) return false;
            tasks_.push(move(task));
        }
        cv_.notify_one();
        return true;
    }

    void stop() {
        {
            unique_lock<mutex> lock(mtx_);
            if (!running) return;
            running = false;
        }
        cv_.notify_all();
    }
    template<typename... Args>
    void print(Args&&...args){
        lock_guard<mutex> lock(cout_mtx);  
        (cout << ... << forward<Args>(args)) << endl; 
    }
};

int main() {
    TaskQueue* q = TaskQueue::getInstance();
    q->start(4); 
    
    for (int i = 0; i < 10; ++i) {
       q->push([i, q] {
            q->print("task ", i, " thread_id=", this_thread::get_id());
            this_thread::sleep_for(chrono::milliseconds(50));
        });
    }

    this_thread::sleep_for(chrono::milliseconds(300));
    q->stop();
    return 0;
}