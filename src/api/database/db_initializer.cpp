#include "api/database/db_initializer.hpp"

namespace api
{
    void DBInitializer::initialize()
    {
        // 初始化数据库连接
        DBManager::getInstance().initialize("database.db");

        // 创建所有表
        createTables();
    }

    void DBInitializer::createTables()
    {
        createUsersTable();
        // 在这里添加其他表的创建
    }

    void DBInitializer::createUsersTable()
    {
        DBManager::getInstance().executeQuery("CREATE TABLE IF NOT EXISTS users ("
                                              "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                              "username TEXT NOT NULL UNIQUE,"
                                              "email TEXT NOT NULL UNIQUE,"
                                              "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
                                              ");");
    }

}  // namespace api