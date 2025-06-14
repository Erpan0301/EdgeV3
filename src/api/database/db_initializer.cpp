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
        // 在这里添加其他表的创建
        createUsersTable();
        createCamerasTable();
        createModelsTable();
    }

    // 初始化用户表
    void DBInitializer::createUsersTable()
    {
        DBManager::getInstance().executeQuery("CREATE TABLE IF NOT EXISTS users ("
                                              "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                              "username TEXT NOT NULL UNIQUE,"
                                              "email TEXT NOT NULL UNIQUE,"
                                              "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
                                              ");");
    }

    // 初始化摄像头表

    void DBInitializer::createCamerasTable()
    {
        DBManager::getInstance().executeQuery("CREATE TABLE IF NOT EXISTS cameras ("
                                              "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                              "name TEXT NOT NULL,"
                                              "ip TEXT NOT NULL,"
                                              "stream_url TEXT,"
                                              "online INTEGER DEFAULT 0,"  // 0: offline, 1: online
                                              "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
                                              "updated_at DATETIME,"
                                              "use_model_id TEXT,"
                                              "coding_type TEXT,"
                                              "detection_types TEXT,"  // 存储 JSON 数组，如 [1,2,3,4]
                                              "danger_types TEXT,"     // 存储 JSON 数组，如 [2,3]
                                              "nms_thresh REAL DEFAULT 0.45,"
                                              "conf_thresh REAL DEFAULT 0.5,"
                                              "point_type_id INTEGER DEFAULT 1,"
                                              "points TEXT"  // 存储 JSON 数组，如 [{"x":100,"y":100},{"x":200,"y":200}]
                                              ");");
    }

    // 初始化模型表

    void DBInitializer::createModelsTable()
    {
        DBManager::getInstance().executeQuery(
            "CREATE TABLE IF NOT EXISTS models ("
            "id TEXT PRIMARY KEY,"
            "name TEXT NOT NULL,"
            "path TEXT NOT NULL,"
            "description TEXT,"
            "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
            "updated_at DATETIME,"
            "version TEXT NOT NULL,"
            "classes TEXT"  // 存储 JSON 数组，如 [{"id":1,"label":"person"},{"id":2,"label":"car","color":"#000000"}]
            ");");
    }

}  // namespace api