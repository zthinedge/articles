#pragma once
#include <mysql/mysql.h>
#include <string>
#include <chrono>

class Connection {
public:
    Connection();
    ~Connection();

    // 连接数据库
    bool connect(std::string user, std::string passwd, std::string dbName, std::string ip, unsigned short port = 3306);
    // 更新操作（insert, update, delete）
    bool update(std::string sql);
    // 查询操作
    MYSQL_RES* query(std::string sql);

    // 刷新连接的空闲起始时间（改用系统时间，更准确）
    void refreshAliveTime() { _aliveTime = std::chrono::steady_clock::now(); }
    // 计算连接已空闲的时间（毫秒）
    int getAliveTime() {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - _aliveTime);
        return duration.count();
    }

private:
    MYSQL* _conn;       // MySQL 句柄
    std::chrono::steady_clock::time_point _aliveTime; // 空闲起始时间（高精度）
};