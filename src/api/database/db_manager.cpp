#include "api/database/db_manager.hpp"

#include <iostream>

namespace api
{
    DBManager& DBManager::getInstance()
    {
        static DBManager instance;
        return instance;
    }

    void DBManager::initialize(const std::string& db_path)
    {
        int rc = sqlite3_open(db_path.c_str(), &db_);
        if (rc != SQLITE_OK)
        {
            std::string error = sqlite3_errmsg(db_);
            sqlite3_close(db_);
            throw DatabaseException("Failed to open database: " + error);
        }

        // 启用外键约束
        executeQuery("PRAGMA foreign_keys = ON;");
    }

    void DBManager::executeQuery(const std::string& sql)
    {
        char* errMsg = nullptr;
        int   rc     = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);

        if (rc != SQLITE_OK)
        {
            std::string error = errMsg;
            sqlite3_free(errMsg);
            throw DatabaseException("SQL error: " + error);
        }
    }

    void DBManager::close()
    {
        if (db_)
        {
            sqlite3_close(db_);
            db_ = nullptr;
        }
    }

    DBManager::~DBManager() { close(); }

}  // namespace api