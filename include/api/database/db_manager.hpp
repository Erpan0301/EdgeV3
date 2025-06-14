#pragma once

#include <memory>
#include <sqlite3.h>
#include <stdexcept>
#include <string>

namespace api
{
    class DatabaseException : public std::runtime_error
    {
      public:
        explicit DatabaseException(const std::string& message) : std::runtime_error(message) {}
    };

    class DBManager
    {
      public:
        static DBManager& getInstance();

        // 初始化数据库连接
        void initialize(const std::string& db_path);

        // 执行SQL查询
        void executeQuery(const std::string& sql);

        // 获取数据库连接
        sqlite3* getConnection() { return db_; }

        // 关闭数据库连接
        void close();

      private:
        DBManager() = default;
        ~DBManager();

        // 禁止拷贝和赋值
        DBManager(const DBManager&) = delete;
        DBManager& operator=(const DBManager&) = delete;

        sqlite3* db_ = nullptr;
    };

}  // namespace api