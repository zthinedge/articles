#include "Connection.h"
#include <iostream>

// 构造函数：初始化MySQL句柄
Connection::Connection() {
    _conn = mysql_init(nullptr);
    // 设置字符集（防止中文乱码）
    mysql_options(_conn, MYSQL_SET_CHARSET_NAME, "utf8mb4");
}

// 析构函数：关闭连接
Connection::~Connection() {
    if (_conn != nullptr) {
        mysql_close(_conn);
    }
}

// 连接数据库（封装mysql_real_connect）
bool Connection::connect(std::string user, std::string passwd, std::string dbName, std::string ip, unsigned short port) {
    MYSQL* p = mysql_real_connect(
        _conn, 
        ip.c_str(), 
        user.c_str(), 
        passwd.c_str(), 
        dbName.c_str(), 
        port, 
        nullptr, 
        0
    );
    if (p == nullptr) {
        std::cerr << "MySQL连接失败: " << mysql_error(_conn) << std::endl;
        return false;
    }
    return true;
}

// 执行更新类SQL（insert/update/delete）
bool Connection::update(std::string sql) {
    if (mysql_query(_conn, sql.c_str())) {
        std::cerr << "SQL执行失败: " << mysql_error(_conn) << " | SQL: " << sql << std::endl;
        return false;
    }
    return true;
}

// 执行查询类SQL（select）
MYSQL_RES* Connection::query(std::string sql) {
    // 释放上一次残留的结果集，避免内存泄漏
    MYSQL_RES* res = mysql_store_result(_conn);
    if (res) mysql_free_result(res);

    // 执行新查询
    if (mysql_query(_conn, sql.c_str())) {
        std::cerr << "SQL查询失败: " << mysql_error(_conn) << " | SQL: " << sql << std::endl;
        return nullptr;
    }

    // 拉取结果集到客户端内存
    return mysql_store_result(_conn);
}