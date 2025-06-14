#include "api/routes/system_routes.hpp"

#include "api/models/response_model.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>

namespace api
{
    void SystemRoutes::register_routes(crow::App<GlobalMiddleware>& app)
    {
        // 获取系统时间的API
        CROW_ROUTE(app, "/api/system/time")
        ([]() {
            // 获取当前时间
            auto now  = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);

            // 格式化时间
            std::stringstream ss;
            ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");

            // 返回JSON响应
            return Response::success(crow::json::wvalue {
                {"timestamp", static_cast<int64_t>(time)},
                {"datetime", ss.str()},
            });
        });
    }
}  // namespace api