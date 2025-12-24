#include "ConnectionPool.h"

// 单例获取
ConnectionPool* ConnectionPool::getConnectionPool() {
    static ConnectionPool pool;
    return &pool;
}

// 构造函数：加载配置+初始化连接+启动后台线程
ConnectionPool::ConnectionPool() : _isRunning(true) {
    // 1. 加载配置文件
    if (!loadConfigFile()) {
        std::cerr << "配置文件加载失败！" << std::endl;
        _isRunning = false;
        return;
    }

    // 2. 初始化连接队列
    for (int i = 0; i < _initSize; ++i) {
        Connection* conn = new Connection();
        if (conn->connect(_user, _password, _dbname, _ip, _port)) {
            conn->refreshAliveTime();
            _connectionQue.push(conn);
            std::cout << "初始化连接成功，当前连接数：" << _connectionQue.size() << std::endl;
        } else {
            std::cerr << "初始化连接失败，跳过该连接" << std::endl;
            delete conn; // 失败则释放
        }
    }

    // 3. 启动生产者线程和扫描线程
    _produceThread = std::thread(&ConnectionPool::produceConnectionTask, this);
    _scannerThread = std::thread(&ConnectionPool::scannerConnectionTask, this);
}

// 析构函数：停止线程+释放所有连接
ConnectionPool::~ConnectionPool() {
    _isRunning = false;
    // 等待后台线程退出
    if (_produceThread.joinable()) _produceThread.join();
    if (_scannerThread.joinable()) _scannerThread.join();

    // 释放队列中所有连接
    std::unique_lock<std::mutex> lock(_queueMutex);
    while (!_connectionQue.empty()) {
        Connection* conn = _connectionQue.front();
        _connectionQue.pop();
        delete conn;
    }
    std::cout << "连接池已销毁，所有连接已释放" << std::endl;
}

// 加载配置文件（简易INI解析）
bool ConnectionPool::loadConfigFile() {
    std::ifstream file("config.ini");
    if (!file.is_open()) {
        std::cerr << "配置文件config.ini不存在！" << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        // 跳过空行/注释行
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;
        
        // 解析键值对（key=value）
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        // 赋值到成员变量
        if (key == "ip") _ip = value;
        else if (key == "user") _user = value;
        else if (key == "password") _password = value;
        else if (key == "dbname") _dbname = value;
        else if (key == "port") _port = std::stoi(value);
        else if (key == "initSize") _initSize = std::stoi(value);
        else if (key == "maxSize") _maxSize = std::stoi(value);
        else if (key == "maxIdleTime") _maxIdleTime = std::stoi(value);
        else if (key == "connectionTimeout") _connectionTimeout = std::stoi(value);
    }
    file.close();

    // 校验必要配置
    if (_ip.empty() || _user.empty() || _password.empty() || _dbname.empty() || _port == 0) {
        std::cerr << "配置文件缺少必要项！" << std::endl;
        return false;
    }
    return true;
}

// 生产者线程：补充连接
void ConnectionPool::produceConnectionTask() {
    while (_isRunning) {
        std::unique_lock<std::mutex> lock(_queueMutex);
        
        // 空闲连接数 >= 初始值，无需生产，等待1秒
        while (static_cast<int>(_connectionQue.size()) >= _initSize && _isRunning) {
            _cv.wait_for(lock, std::chrono::seconds(1));
        }

        // 连接数未到上限，生产新连接
        if (static_cast<int>(_connectionQue.size()) < _maxSize && _isRunning) {
            Connection* conn = new Connection();
            if (conn->connect(_user, _password, _dbname, _ip, _port)) {
                conn->refreshAliveTime();
                _connectionQue.push(conn);
                std::cout << "生产者线程：新增连接，当前队列数：" << _connectionQue.size() << std::endl;
                _cv.notify_all(); // 通知消费者
            } else {
                delete conn;
            }
        }
    }
}

// 扫描线程：回收超时连接
void ConnectionPool::scannerConnectionTask() {
    while (_isRunning) {
        // 每3秒扫描一次
        std::this_thread::sleep_for(std::chrono::seconds(3));

        std::unique_lock<std::mutex> lock(_queueMutex);
        // 遍历队列，清理超时连接
        size_t queueSize = _connectionQue.size();
        for (size_t i = 0; i < queueSize && !_connectionQue.empty(); ++i) {
            Connection* conn = _connectionQue.front();
            if (conn->getAliveTime() > _maxIdleTime) {
                // 超时，销毁连接
                _connectionQue.pop();
                delete conn;
                std::cout << "扫描线程：回收超时连接，当前队列数：" << _connectionQue.size() << std::endl;
            } else {
                // 队列是FIFO，前面的没超时，后面的也不会超时
                break;
            }
        }
    }
}

// 获取连接（带超时等待）
std::shared_ptr<Connection> ConnectionPool::getConnection() {
    std::unique_lock<std::mutex> lock(_queueMutex);

    // 队列为空，等待生产者生产（带超时）
    if (_connectionQue.empty()) {
    std::chrono::milliseconds timeout(_connectionTimeout);
    std::cv_status status = _cv.wait_for(lock, timeout);
    if (status == std::cv_status::timeout && _connectionQue.empty()) {
        std::cerr << "获取连接超时！" << std::endl;
        return nullptr;
    }
}

    // 封装智能指针，自定义删除器（归还连接而非销毁）
    std::shared_ptr<Connection> sp(_connectionQue.front(), [this](Connection* conn) {
        std::unique_lock<std::mutex> lock(_queueMutex);
        conn->refreshAliveTime(); // 刷新空闲时间
        _connectionQue.push(conn);
        std::cout << "连接已归还，当前队列数：" << _connectionQue.size() << std::endl;
        _cv.notify_all(); // 通知生产者
    });

    _connectionQue.pop();
    _cv.notify_all(); // 通知生产者（可能需要补充连接）
    return sp;
}