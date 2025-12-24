#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <string>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iostream>
#include "Connection.h"

class ConnectionPool {
public:
    // 获取单例实例（懒汉+局部静态，线程安全）
    static ConnectionPool* getConnectionPool();

    // 外部接口：从池中获取一个可用连接（带超时）
    std::shared_ptr<Connection> getConnection();

private:
    // 私有化构造/拷贝/赋值
    ConnectionPool();
    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;
    ~ConnectionPool();

    // 加载配置文件
    bool loadConfigFile();

    // 生产者线程：补充连接（当空闲连接不足时）
    void produceConnectionTask();

    // 扫描线程：回收超时空闲连接
    void scannerConnectionTask();

    // 配置项
    std::string _ip;
    std::string _user;
    std::string _password;
    std::string _dbname;
    unsigned short _port;

    int _initSize;      // 初始连接数
    int _maxSize;       // 最大连接数
    int _maxIdleTime;   // 最大空闲时间（毫秒）
    int _connectionTimeout; // 获取连接的超时时间（毫秒）

    // 连接队列+线程安全
    std::queue<Connection*> _connectionQue;
    std::mutex _queueMutex;
    std::condition_variable _cv; // 生产/消费条件变量

    // 线程控制
    std::atomic<bool> _isRunning; // 线程运行标志
    std::thread _produceThread;   // 生产者线程
    std::thread _scannerThread;   // 扫描线程
};