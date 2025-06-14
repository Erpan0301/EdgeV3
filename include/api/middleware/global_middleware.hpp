#pragma once
#include <crow_all.h>
#include <sstream>

// 为 crow::HTTPMethod 声明 operator<<
std::ostream& operator<<(std::ostream& os, const crow::HTTPMethod& method);

struct GlobalMiddleware
{
    struct context
    {
    };

    void before_handle(crow::request& req, crow::response& res, context& ctx);
    void after_handle(crow::request& req, crow::response& res, context& ctx);
};