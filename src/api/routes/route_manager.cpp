#include "api/routes/route_manager.hpp"

#include "api/models/response_model.hpp"
#include "api/routes/camera_routes.hpp"
#include "api/routes/system_routes.hpp"
#include "api/routes/user_routes.hpp"

namespace api
{
    void RouteManager::register_routes(crow::App<GlobalMiddleware>& app)
    {
        // 注册系统路由
        SystemRoutes::register_routes(app);

        // 注册用户路由
        UserRoutes::register_routes(app);

        // 注册摄像头路由
        CameraRoutes::register_routes(app);

        CROW_ROUTE(app, "/api")([]() { return Response::success(crow::json::wvalue {{"message", "Hello from API!"}}); });

        CROW_ROUTE(app, "/api/example")([]() { return Response::success(crow::json::wvalue {{"example", "data"}}); });

        CROW_ROUTE(app, "/api/error")([]() { return Response::error(400, "Bad Request"); });
    }
}  // namespace api