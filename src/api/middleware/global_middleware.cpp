#include "api/middleware/global_middleware.hpp"

// 实现 operator<<
std::ostream& operator<<(std::ostream& os, const crow::HTTPMethod& method)
{
    switch (method)
    {
        case crow::HTTPMethod::GET:
            return os << "GET";
        case crow::HTTPMethod::POST:
            return os << "POST";
        case crow::HTTPMethod::PUT:
            return os << "PUT";
        case crow::HTTPMethod::DELETE:
            return os << "DELETE";
        case crow::HTTPMethod::PATCH:
            return os << "PATCH";
        case crow::HTTPMethod::HEAD:
            return os << "HEAD";
        case crow::HTTPMethod::OPTIONS:
            return os << "OPTIONS";
        default:
            return os << "UNKNOWN";
    }
}

// 实现 GlobalMiddleware 的成员函数
void GlobalMiddleware::before_handle(crow::request& req, crow::response& res, context& /*ctx*/)
{
    // 日志记录请求方法和 URL
    CROW_LOG_INFO << "Request: " << req.method << " " << req.url;
}

void GlobalMiddleware::after_handle(crow::request& req, crow::response& res, context& /*ctx*/)
{
    // 添加统一响应头
    res.add_header("X-Powered-By", "EdgeV3-Server");
    res.add_header("Access-Control-Allow-Origin", "*");
    res.set_header("Content-Type", "application/json");
}