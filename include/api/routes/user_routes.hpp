#pragma once

#include "api/middleware/global_middleware.hpp"
#include "api/models/response_model.hpp"
#include "crow_all.h"

namespace api
{
    class UserRoutes
    {
      public:
        static void register_routes(crow::App<GlobalMiddleware>& app);
    };
}  // namespace api