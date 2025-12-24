#include "ConnectionPool.h"
#include <iostream>
#include <thread>
#include <vector>

// 测试函数：多线程获取连接执行SQL
void testConnectionPool(int threadId) {
    ConnectionPool* pool = ConnectionPool::getConnectionPool();
    for (int i = 0; i < 2; ++i) { // 每个线程执行2次操作
        std::shared_ptr<Connection> conn = pool->getConnection();
        if (conn) {
            // 插入测试数据（需先创建testdb库和user表：CREATE TABLE user(name VARCHAR(20), age INT);）
            std::string sql = "INSERT INTO user(name, age) VALUES('test_" + std::to_string(threadId) + "_" + std::to_string(i) + "', " + std::to_string(20 + threadId) + ")";
            if (conn->update(sql)) {
                std::cout << "线程" << threadId << "：SQL执行成功 | " << sql << std::endl;
            }
        } else {
            std::cerr << "线程" << threadId << "：获取连接失败" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 模拟业务耗时
    }
}

int main() {
    // 多线程测试（5个线程）
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(testConnectionPool, i);
    }

    // 等待所有线程结束
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    std::cout << "所有测试线程执行完毕" << std::endl;
    return 0;
}