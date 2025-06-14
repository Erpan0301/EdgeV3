#pragma once
#include <crow_all.h>

namespace api
{
    struct Response
    {
        int                code;
        std::string        message;
        crow::json::wvalue data;

        crow::json::wvalue to_json() const;

        static crow::response success(crow::json::wvalue data = {});
        static crow::response error(int code, const std::string& message);
    };
}  // namespace api