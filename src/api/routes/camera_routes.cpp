#include "api/routes/camera_routes.hpp"

#include "api/models/camera_model.hpp"
#include "api/models/response_model.hpp"

#include <regex>
#include <sstream>

namespace api
{
    bool isValidIpAddress(const std::string& ip)
    {
        std::regex ipv4_pattern("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
        return std::regex_match(ip, ipv4_pattern);
    }

    void CameraRoutes::register_routes(crow::App<GlobalMiddleware>& app)
    {
        // Create a new camera
        CROW_ROUTE(app, "/api/camera").methods("POST"_method)([](const crow::request& req) {
            try
            {
                auto x = crow::json::load(req.body);
                if (!x)
                {
                    return Response::error(400, "Invalid JSON");
                }

                // Validate required fields
                std::vector<std::string> required_fields = {
                    "name",
                    "ip",
                    "stream_url",
                    "use_model_id",
                    "detection_types",
                    "danger_types",
                    "nms_thresh",
                    "conf_thresh",
                };

                for (const auto& field : required_fields)
                {
                    if (!x.has(field))
                    {
                        return Response::error(400, "Missing required field: " + field);
                    }
                }

                // Validate field types and formats
                try
                {
                    // Validate name (non-empty string)
                    if (x["name"].t() != crow::json::type::String || x["name"].s().size() == 0)
                    {
                        return Response::error(400, "Name must be a non-empty string");
                    }

                    // Validate IP address format
                    if (x["ip"].t() != crow::json::type::String || !isValidIpAddress(x["ip"].s()))
                    {
                        return Response::error(400, "Invalid IP address format");
                    }

                    // Validate stream_url (non-empty string)
                    if (x["stream_url"].t() != crow::json::type::String || x["stream_url"].s().size() == 0)
                    {
                        return Response::error(400, "Stream URL must be a non-empty string");
                    }

                    // Validate use_model_id (non-empty string)
                    if (x["use_model_id"].t() != crow::json::type::String || x["use_model_id"].s().size() == 0)
                    {
                        return Response::error(400, "Use model ID must be a non-empty string");
                    }

                    // Validate detection_types (array of integers)
                    if (x["detection_types"].t() != crow::json::type::List)
                    {
                        return Response::error(400, "Detection types must be an array");
                    }
                    for (const auto& type : x["detection_types"])
                    {
                        if (type.t() != crow::json::type::Number)
                        {
                            return Response::error(400, "Detection types must be integers");
                        }
                    }

                    // Validate danger_types (array of integers)
                    if (x["danger_types"].t() != crow::json::type::List)
                    {
                        return Response::error(400, "Danger types must be an array");
                    }
                    for (const auto& type : x["danger_types"])
                    {
                        if (type.t() != crow::json::type::Number)
                        {
                            return Response::error(400, "Danger types must be integers");
                        }
                    }

                    // Validate nms_thresh (float between 0 and 1)
                    if (x["nms_thresh"].t() != crow::json::type::Number)
                    {
                        return Response::error(400, "NMS threshold must be a number");
                    }
                    double nms_thresh = x["nms_thresh"].d();
                    if (nms_thresh < 0.0 || nms_thresh > 1.0)
                    {
                        return Response::error(400, "NMS threshold must be between 0 and 1");
                    }

                    // Validate conf_thresh (float between 0 and 1)
                    if (x["conf_thresh"].t() != crow::json::type::Number)
                    {
                        return Response::error(400, "Confidence threshold must be a number");
                    }
                    double conf_thresh = x["conf_thresh"].d();
                    if (conf_thresh < 0.0 || conf_thresh > 1.0)
                    {
                        return Response::error(400, "Confidence threshold must be between 0 and 1");
                    }

                    // Validate point_type_id (positive integer, optional)
                    int point_type_id = 0;
                    if (x.has("point_type_id"))
                    {
                        if (x["point_type_id"].t() != crow::json::type::Number)
                        {
                            return Response::error(400, "Point type ID must be a number");
                        }
                        point_type_id = x["point_type_id"].i();
                        if (point_type_id < 0)
                        {
                            return Response::error(400, "Point type ID must be a non-negative integer");
                        }
                    }

                    // Validate points (array of objects with x, y coordinates, optional)
                    if (x.has("points"))
                    {
                        if (x["points"].t() != crow::json::type::List)
                        {
                            return Response::error(400, "Points must be an array");
                        }
                        for (const auto& point : x["points"])
                        {
                            if (point.t() != crow::json::type::Object || !point.has("x") || !point.has("y"))
                            {
                                return Response::error(400, "Each point must be an object with x and y coordinates");
                            }
                            if (point["x"].t() != crow::json::type::Number || point["y"].t() != crow::json::type::Number)
                            {
                                return Response::error(400, "Point coordinates must be numbers");
                            }
                        }
                    }

                    // Create camera with validated data
                    try
                    {
                        Camera camera(x);
                        if (Camera::create(camera))
                        {
                            return Response::success(crow::json::wvalue {{"message", "Camera created successfully"}});
                        }
                        return Response::error(500, "Failed to create camera");
                    }
                    catch (const std::exception& e)
                    {
                        return Response::error(500, std::string("Error creating camera: ") + e.what());
                    }
                }
                catch (const std::exception& e)
                {
                    return Response::error(400, std::string("Validation error: ") + e.what());
                }
            }
            catch (const std::exception& e)
            {
                return Response::error(500, std::string("Server error: ") + e.what());
            }
        });

        // Get all cameras
        CROW_ROUTE(app, "/api/camera").methods("GET"_method)([](const crow::request&) {
            try
            {
                auto               cameras = Camera::findAll();
                crow::json::wvalue camerasJson;
                camerasJson["cameras"] = crow::json::wvalue::list();
                size_t index           = 0;
                for (const auto& camera : cameras)
                {
                    camerasJson["cameras"][index++] = camera.toJson();
                }
                return Response::success(camerasJson);
            }
            catch (const std::exception& e)
            {
                return Response::error(500, std::string("Server error: ") + e.what());
            }
        });

        // Get camera by ID
        CROW_ROUTE(app, "/api/camera/<int>").methods("GET"_method)([](const crow::request&, int id) {
            try
            {
                auto camera = Camera::findById(id);
                if (camera.getId() == 0)
                {
                    return Response::error(404, "Camera not found");
                }
                return Response::success(camera.toJson());
            }
            catch (const std::exception& e)
            {
                return Response::error(500, std::string("Server error: ") + e.what());
            }
        });

        // Update camera
        CROW_ROUTE(app, "/api/camera/<int>").methods("PUT"_method)([](const crow::request& req, int id) {
            try
            {
                auto x = crow::json::load(req.body);
                if (!x)
                {
                    return Response::error(400, "Invalid JSON");
                }

                auto camera = Camera::findById(id);
                if (camera.getId() == 0)
                {
                    return Response::error(404, "Camera not found");
                }

                // Update fields if provided
                if (x.has("name") && x["name"].t() == crow::json::type::String && x["name"].s().size() > 0)
                    camera.setName(x["name"].s());
                if (x.has("ip") && x["ip"].t() == crow::json::type::String && isValidIpAddress(x["ip"].s()))
                    camera.setIp(x["ip"].s());
                if (x.has("stream_url") && x["stream_url"].t() == crow::json::type::String && x["stream_url"].s().size() > 0)
                    camera.setStreamUrl(x["stream_url"].s());
                if (x.has("online") && x["online"].t() == crow::json::type::True)
                    camera.setOnline(true);
                else if (x.has("online") && x["online"].t() == crow::json::type::False)
                    camera.setOnline(false);
                if (x.has("use_model_id") && x["use_model_id"].t() == crow::json::type::String && x["use_model_id"].s().size() > 0)
                    camera.setUseModelId(x["use_model_id"].s());
                if (x.has("coding_type") && x["coding_type"].t() == crow::json::type::String)
                    camera.setCodingType(x["coding_type"].s());
                if (x.has("detection_types") && x["detection_types"].t() == crow::json::type::List)
                {
                    std::vector<int> types;
                    for (const auto& type : x["detection_types"])
                    {
                        if (type.t() == crow::json::type::Number)
                            types.push_back(type.i());
                    }
                    camera.setDetectionTypes(types);
                }
                if (x.has("danger_types") && x["danger_types"].t() == crow::json::type::List)
                {
                    std::vector<int> types;
                    for (const auto& type : x["danger_types"])
                    {
                        if (type.t() == crow::json::type::Number)
                            types.push_back(type.i());
                    }
                    camera.setDangerTypes(types);
                }
                if (x.has("nms_thresh") && x["nms_thresh"].t() == crow::json::type::Number)
                {
                    double nms = x["nms_thresh"].d();
                    if (nms >= 0.0 && nms <= 1.0)
                        camera.setNmsThresh(nms);
                }
                if (x.has("conf_thresh") && x["conf_thresh"].t() == crow::json::type::Number)
                {
                    double conf = x["conf_thresh"].d();
                    if (conf >= 0.0 && conf <= 1.0)
                        camera.setConfThresh(conf);
                }
                if (x.has("point_type_id") && x["point_type_id"].t() == crow::json::type::Number && x["point_type_id"].i() >= 0)
                    camera.setPointTypeId(x["point_type_id"].i());
                if (x.has("points") && x["points"].t() == crow::json::type::List)
                {
                    std::vector<crow::json::wvalue> points;
                    for (const auto& point : x["points"])
                    {
                        if (point.t() == crow::json::type::Object && point.has("x") && point.has("y") && point["x"].t() == crow::json::type::Number &&
                            point["y"].t() == crow::json::type::Number)
                        {
                            crow::json::wvalue p;
                            p["x"] = point["x"].d();
                            p["y"] = point["y"].d();
                            points.push_back(p);
                        }
                    }
                    camera.setPoints(points);
                }

                if (Camera::update(camera))
                {
                    return Response::success(crow::json::wvalue {{"message", "Camera updated successfully"}});
                }
                return Response::error(500, "Failed to update camera");
            }
            catch (const std::exception& e)
            {
                return Response::error(500, std::string("Server error: ") + e.what());
            }
        });

        // Delete camera
        CROW_ROUTE(app, "/api/camera/<int>").methods("DELETE"_method)([](const crow::request&, int id) {
            try
            {
                auto camera = Camera::findById(id);
                if (camera.getId() == 0)
                {
                    return Response::error(404, "Camera not found");
                }

                if (Camera::remove(id))
                {
                    return Response::success(crow::json::wvalue {{"message", "Camera deleted successfully"}});
                }
                return Response::error(500, "Failed to delete camera");
            }
            catch (const std::exception& e)
            {
                return Response::error(500, std::string("Server error: ") + e.what());
            }
        });
    }
}  // namespace api