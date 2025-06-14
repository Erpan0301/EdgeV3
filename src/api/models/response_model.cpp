#include "api/models/response_model.hpp"

namespace api
{
    crow::json::wvalue Response::to_json() const
    {
        return crow::json::wvalue {
            {"code", code},
            {"message", message},
            {"data", std::move(data)},
        };
    }

    crow::response Response::success(crow::json::wvalue data)
    {
        Response res {0, "Success", std::move(data)};
        return crow::response {res.to_json()};
    }

    crow::response Response::error(int code, const std::string& message)
    {
        Response res {code, message, {}};
        return crow::response {res.to_json()};
    }
}  // namespace api