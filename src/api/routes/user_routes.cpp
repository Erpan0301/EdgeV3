#include "api/routes/user_routes.hpp"

#include "api/database/db_manager.hpp"

#include <sstream>

namespace api
{
    // 回调函数用于处理查询结果
    static int callback(void* data, int argc, char** argv, char** azColName)
    {
        auto*              result = static_cast<crow::json::wvalue*>(data);
        crow::json::wvalue user;

        for (int i = 0; i < argc; i++)
        {
            user[azColName[i]] = argv[i] ? argv[i] : "NULL";
        }

        // 直接使用数组赋值
        static int index            = 0;
        (*result)["users"][index++] = std::move(user);
        return 0;
    }

    // 检查用户是否存在的回调函数
    static int check_user_callback(void* data, int argc, char** argv, char** azColName)
    {
        auto* exists = static_cast<bool*>(data);
        *exists      = true;
        return 0;
    }

    void UserRoutes::register_routes(crow::App<GlobalMiddleware>& app)
    {
        // 获取所有用户
        CROW_ROUTE(app, "/api/users")
        ([]() {
            auto&              db = DBManager::getInstance();
            crow::json::wvalue result;

            // 初始化users数组
            result["users"] = crow::json::wvalue::list();

            char* errMsg = nullptr;
            int   rc     = sqlite3_exec(db.getConnection(), "SELECT * FROM users ORDER BY created_at DESC;", callback, &result, &errMsg);

            if (rc != SQLITE_OK)  // 数据库错误
            {
                std::string error = errMsg;
                sqlite3_free(errMsg);
                return Response::error(500, "数据库错误: " + error);
            }

            return Response::success(result);
        });

        // 创建新用户
        CROW_ROUTE(app, "/api/users").methods("POST"_method)([](const crow::request& req) {
            auto x = crow::json::load(req.body);
            if (!x)
            {
                return Response::error(400, "无效的JSON");
            }

            if (!x.has("username") || !x.has("email"))
            {
                return Response::error(400, "缺少必填字段");
            }

            // 查询用户是否存在
            bool              user_exists = false;
            std::stringstream ss;
            ss << "SELECT 1 FROM users WHERE username = '" << x["username"].s() << "';";

            char* errMsg = nullptr;
            int   rc     = sqlite3_exec(DBManager::getInstance().getConnection(), ss.str().c_str(), check_user_callback, &user_exists, &errMsg);

            if (rc != SQLITE_OK)
            {
                std::string error = errMsg;
                sqlite3_free(errMsg);
                return Response::error(500, "数据库错误: " + error);
            }

            if (user_exists)
            {
                return Response::error(400, "用户已存在");
            }

            try
            {
                ss.str("");
                ss << "INSERT INTO users (username, email) VALUES ('" << x["username"].s() << "', '" << x["email"].s() << "');";
                CROW_LOG_INFO << ss.str();
                DBManager::getInstance().executeQuery(ss.str());
                return Response::success(crow::json::wvalue {{"message", "用户创建成功"}});
            }
            catch (const DatabaseException& e)
            {
                return Response::error(500, e.what());
            }
        });
    }

}  // namespace api