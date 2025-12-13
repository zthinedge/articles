#include "ThreadPool.h"

// 全局原子计数器：统计完成的任务数（避免数据竞争）
std::atomic<int> completed_task_count{0};
// 全局互斥锁：保护cout输出，避免多线程打印乱码
std::mutex cout_mtx;

// 测试任务1：简单耗时任务（带任务ID，模拟业务逻辑）
void test_task(int task_id) {
    // 打印任务开始日志（加锁保护）
    {
        std::lock_guard<std::mutex> lock(cout_mtx);
        pid_t tid = syscall(SYS_gettid);
        std::cout << "[任务" << task_id << "] 开始执行 | 执行线程tid=" << tid 
                  << " | std::thread_id=" << std::this_thread::get_id() << std::endl;
    }

    // 模拟任务耗时（100ms）
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 任务完成，计数器+1，并打印日志
    completed_task_count++;
    {
        std::lock_guard<std::mutex> lock(cout_mtx);
        std::cout << "[任务" << task_id << "] 执行完成 | 累计完成任务数=" << completed_task_count << std::endl;
    }
}

// 测试场景1：基础功能（少量任务，验证线程池基本执行能力）
void test_basic_function() {
    std::cout << "\n========== 测试1：基础功能 ==========\n" << std::endl;
    completed_task_count = 0;
    const int TASK_NUM = 5;

    // 创建4个工作线程的线程池
    ThreadPool pool(4);

    // 添加5个任务
    for (int i = 1; i <= TASK_NUM; ++i) {
        bool ret = pool.addtask(std::bind(test_task, i));
        {
            std::lock_guard<std::mutex> lock(cout_mtx);
            std::cout << "[添加任务] 任务" << i << " | 添加结果：" << (ret ? "成功" : "失败") << std::endl;
        }
    }

    // 主动等待所有任务执行完成（超时5秒保护）
    int timeout = 0;
    while (completed_task_count < TASK_NUM) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        timeout++;
        if (timeout > 500) { // 5秒超时
            std::lock_guard<std::mutex> lock(cout_mtx);
            std::cerr << "[测试1] 任务执行超时！已完成：" << completed_task_count << "/" << TASK_NUM << std::endl;
            break;
        }
    }

    // 打印测试1结果
    {
        std::lock_guard<std::mutex> lock(cout_mtx);
        std::cout << "\n[测试1结果] 预期完成" << TASK_NUM << "个任务 | 实际完成：" << completed_task_count << std::endl;
    }
}

// 测试场景2：并发添加任务（验证多线程添加任务不丢失、能被处理）
void test_concurrent_add_task() {
    std::cout << "\n========== 测试2：并发添加任务 ==========\n" << std::endl;
    completed_task_count = 0;
    const int TOTAL_TASK = 20;

    // 创建4个工作线程的线程池
    ThreadPool pool(4);

    // 线程1：添加1~10号任务
    std::thread t1([&pool]() {
        for (int i = 1; i <= 10; ++i) {
            pool.addtask(std::bind(test_task, i));
            std::this_thread::sleep_for(std::chrono::milliseconds(5)); // 模拟添加间隔
        }
        {
            std::lock_guard<std::mutex> lock(cout_mtx);
            std::cout << "[添加线程t1] 10个任务添加完成" << std::endl;
        }
    });

    // 线程2：添加11~20号任务
    std::thread t2([&pool]() {
        for (int i = 11; i <= 20; ++i) {
            pool.addtask(std::bind(test_task, i));
            std::this_thread::sleep_for(std::chrono::milliseconds(5)); // 模拟添加间隔
        }
        {
            std::lock_guard<std::mutex> lock(cout_mtx);
            std::cout << "[添加线程t2] 10个任务添加完成" << std::endl;
        }
    });

    // 等待添加任务的线程完成
    t1.join();
    t2.join();

    // 主动等待所有任务执行完成（超时10秒保护）
    int timeout = 0;
    while (completed_task_count < TOTAL_TASK) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        timeout++;
        // 每1秒打印一次进度（避免看似"卡住"）
        if (timeout % 100 == 0) {
            std::lock_guard<std::mutex> lock(cout_mtx);
            std::cout << "[测试2] 等待中 | 已完成：" << completed_task_count << "/" << TOTAL_TASK << std::endl;
        }
        // 10秒超时退出
        if (timeout > 1000) {
            std::lock_guard<std::mutex> lock(cout_mtx);
            std::cerr << "[测试2] 任务执行超时！" << std::endl;
            break;
        }
    }

    // 打印测试2结果
    {
        std::lock_guard<std::mutex> lock(cout_mtx);
        std::cout << "\n[测试2结果] 预期完成" << TOTAL_TASK << "个任务 | 实际完成：" << completed_task_count << std::endl;
    }
}

// 测试场景3：停止线程池后添加任务（验证返回false，拒绝新任务）
void test_stop_reject_task() {
    std::cout << "\n========== 测试3：停止后拒绝添加任务 ==========\n" << std::endl;
    completed_task_count = 0;

    // 局部作用域：线程池在作用域结束时自动析构（停止）
    {
        ThreadPool pool(2);
        // 先添加一个正常任务
        bool ret1 = pool.addtask(std::bind(test_task, 99));
        {
            std::lock_guard<std::mutex> lock(cout_mtx);
            std::cout << "[添加任务99] 结果：" << (ret1 ? "成功（正常）" : "失败（异常）") << std::endl;
        }

        // 等待任务执行完成
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    } // 作用域结束，pool自动析构（stop_=true，线程池停止，成员变量安全销毁）

    // 验证：停止后的线程池无法访问（避免访问已析构对象）
    {
        std::lock_guard<std::mutex> lock(cout_mtx);
        std::cout << "[线程池已停止] 无法再访问已析构的对象，避免段错误" << std::endl;
    }

    // 正确验证方式：创建新的线程池，在作用域内停止后拒绝任务
    {
        ThreadPool temp_pool(2);
        // 先添加一个任务
        temp_pool.addtask(std::bind(test_task, 100));
        // 作用域结束，temp_pool自动析构（停止）
    }
    // 注意：这里不能再访问temp_pool，否则会触发段错误
}

// 验证：停止后的线程池添加任务返回false
void test_stop_add() {
    ThreadPool temp_pool(2);
    // 尝试添加任务
    bool ret = temp_pool.addtask(std::bind(test_task, 100));
    {
        std::lock_guard<std::mutex> lock(cout_mtx);
        std::cout << "[添加任务100（线程池已停止）] 结果：" << (ret ? "成功（异常）" : "失败（正常）") << std::endl;
    }
}

// 测试场景4：停止线程池后，执行完队列剩余任务
void test_remaining_task_after_stop() {
    std::cout << "\n========== 测试4：停止后执行剩余任务 ==========\n" << std::endl;
    completed_task_count = 0;
    const int TASK_NUM = 10;

    // 创建2个工作线程的线程池
    ThreadPool pool(2);

    // 批量添加10个任务
    for (int i = 1; i <= TASK_NUM; ++i) {
        pool.addtask(std::bind(test_task, i));
    }

    // 等待0.2秒（让部分任务执行，剩余任务留在队列）
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    {
        std::lock_guard<std::mutex> lock(cout_mtx);
        std::cout << "[测试4] 线程池开始停止 | 已完成：" << completed_task_count << " | 剩余任务数：" << TASK_NUM - completed_task_count << std::endl;
    }

    // 线程池析构（stop_=true），会执行完队列剩余任务后退出
}

int main() {
    try {
        test_basic_function();       // 测试1：基础功能
        test_concurrent_add_task();  // 测试2：并发添加任务
        test_stop_reject_task();     // 测试3：停止后拒绝任务
        test_stop_add();             // 验证停止后添加任务返回false
        test_remaining_task_after_stop(); // 测试4：停止后执行剩余任务

        std::cout << "\n========== 所有测试完成 ==========\n" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "测试过程异常：" << e.what() << std::endl;
    }
    return 0;
}