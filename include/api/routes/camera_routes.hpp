#pragma once

#include "api/middleware/global_middleware.hpp"
#include "crow_all.h"

namespace api
{
    class CameraRoutes
    {
      public:
        static void register_routes(crow::App<GlobalMiddleware>& app);
    };
}  // namespace api