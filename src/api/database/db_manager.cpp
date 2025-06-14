#include "api/database/db_manager.hpp"

#include "crow_all.h"

namespace api
{
    DBManager& DBManager::getInstance()
    {
        static DBManager instance;
        return instance;
    }

    void DBManager::initialize(const std::string& db_path)
    {
        if (db_ != nullptr)
        {
            close();
        }

        int rc = sqlite3_open(db_path.c_str(), &db_);
        if (rc != SQLITE_OK)
        {
            std::string error = sqlite3_errmsg(db_);
            sqlite3_close(db_);
            db_ = nullptr;
            throw DatabaseException("Failed to open database: " + error);
        }

        // 启用外键约束
        executeQuery("PRAGMA foreign_keys = ON;");
    }

    std::vector<std::map<std::string, std::string>> DBManager::executeQuery(const std::string& sql)
    {
        if (db_ == nullptr)
        {
            throw DatabaseException("Database not initialized");
        }

        std::vector<std::map<std::string, std::string>> results;
        char*                                           errMsg = nullptr;

        int rc = sqlite3_exec(
            db_,
            sql.c_str(),
            [](void* data, int argc, char** argv, char** azColName) -> int {
                auto*                              results = static_cast<std::vector<std::map<std::string, std::string>>*>(data);
                std::map<std::string, std::string> row;
                for (int i = 0; i < argc; i++)
                {
                    row[azColName[i]] = argv[i] ? argv[i] : "";
                }
                results->push_back(row);
                return 0;
            },
            &results,
            &errMsg);

        if (rc != SQLITE_OK)
        {
            std::string error = errMsg;
            sqlite3_free(errMsg);
            throw DatabaseException("SQL error: " + error);
        }

        return results;
    }

    void DBManager::close()
    {
        if (db_ != nullptr)
        {
            sqlite3_close(db_);
            db_ = nullptr;
        }
    }

    DBManager::~DBManager() { close(); }

}  // namespace api