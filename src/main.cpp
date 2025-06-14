#include "api/database/db_initializer.hpp"
#include "api/middleware/global_middleware.hpp"
#include "api/routes/route_manager.hpp"

#include <crow_all.h>
#include <iostream>
#include <sqlite3.h>

int main()
{
    try
    {
        // 初始化数据库
        api::DBInitializer::initialize();

        // 启动Web服务器
        crow::App<GlobalMiddleware> app;
        api::RouteManager::register_routes(app);
        app.port(18080).multithreaded().run();

        return 0;
    }
    catch (const api::DatabaseException& e)
    {
        std::cerr << "Database error: " << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}