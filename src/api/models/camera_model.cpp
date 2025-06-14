#include "api/models/camera_model.hpp"

#include "api/database/db_manager.hpp"

#include <iostream>
#include <sstream>

namespace api
{
    Camera::Camera(const crow::json::rvalue& json)
    {
        id           = json["id"].i();
        name         = json["name"].s();
        ip           = json["ip"].s();
        stream_url   = json["stream_url"].s();
        online       = json["online"].b();
        created_at   = json["created_at"].s();
        updated_at   = json["updated_at"].s();
        use_model_id = json["use_model_id"].s();
        coding_type  = json["coding_type"].s();

        if (json.has("detection_types"))
        {
            for (const auto& type : json["detection_types"])
            {
                detection_types.push_back(type.i());
            }
        }

        if (json.has("danger_types"))
        {
            for (const auto& type : json["danger_types"])
            {
                danger_types.push_back(type.i());
            }
        }

        nms_thresh    = json["nms_thresh"].d();
        conf_thresh   = json["conf_thresh"].d();
        point_type_id = json["point_type_id"].i();

        if (json.has("points"))
        {
            for (const auto& point : json["points"])
            {
                crow::json::wvalue p;
                p["x"] = point["x"].i();  // Use .i() for integer coordinates
                p["y"] = point["y"].i();
                points.emplace_back(std::move(p));  // Use emplace_back to avoid copy
            }
        }
    }

    crow::json::wvalue Camera::toJson() const
    {
        crow::json::wvalue json;
        json["id"]           = id;
        json["name"]         = name;
        json["ip"]           = ip;
        json["stream_url"]   = stream_url;
        json["online"]       = online;
        json["created_at"]   = created_at;
        json["updated_at"]   = updated_at;
        json["use_model_id"] = use_model_id;
        json["coding_type"]  = coding_type;

        crow::json::wvalue detectionTypes = crow::json::wvalue::list();
        for (size_t i = 0; i < detection_types.size(); i++)
        {
            detectionTypes[i] = detection_types[i];
        }
        json["detection_types"] = std::move(detectionTypes);

        crow::json::wvalue dangerTypes = crow::json::wvalue::list();
        for (size_t i = 0; i < danger_types.size(); i++)
        {
            dangerTypes[i] = danger_types[i];
        }
        json["danger_types"] = std::move(dangerTypes);

        json["nms_thresh"]    = nms_thresh;
        json["conf_thresh"]   = conf_thresh;
        json["point_type_id"] = point_type_id;

        crow::json::wvalue pointsJson = crow::json::wvalue::list();
        for (size_t i = 0; i < points.size(); i++)
        {
            pointsJson[i]      = crow::json::wvalue::object();
            pointsJson[i]["x"] = points[i]["x"].i();
            pointsJson[i]["y"] = points[i]["y"].i();
        }
        json["points"] = std::move(pointsJson);

        return json;
    }

    Camera Camera::findById(int id)
    {
        try
        {
            auto result = DBManager::getInstance().executeQuery("SELECT * FROM cameras WHERE id = " + std::to_string(id));
            if (result.empty())
            {
                return Camera();
            }

            crow::json::wvalue json;
            json["id"]           = std::stoi(result[0]["id"]);
            json["name"]         = result[0]["name"];
            json["ip"]           = result[0]["ip"];
            json["stream_url"]   = result[0]["stream_url"];
            json["online"]       = std::stoi(result[0]["online"]) != 0;
            json["created_at"]   = result[0]["created_at"];
            json["updated_at"]   = result[0]["updated_at"];
            json["use_model_id"] = result[0]["use_model_id"];
            json["coding_type"]  = result[0]["coding_type"];

            auto detectionTypes = crow::json::load(result[0]["detection_types"]);
            if (detectionTypes)
            {
                json["detection_types"] = std::move(detectionTypes);
            }

            auto dangerTypes = crow::json::load(result[0]["danger_types"]);
            if (dangerTypes)
            {
                json["danger_types"] = std::move(dangerTypes);
            }

            json["nms_thresh"]    = std::stof(result[0]["nms_thresh"]);
            json["conf_thresh"]   = std::stof(result[0]["conf_thresh"]);
            json["point_type_id"] = std::stoi(result[0]["point_type_id"]);

            auto points = crow::json::load(result[0]["points"]);
            if (points)
            {
                json["points"] = std::move(points);
            }

            std::string jsonStr = json.dump();
            auto        rvalue  = crow::json::load(jsonStr);
            return Camera(rvalue);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error finding camera by ID: " << e.what() << std::endl;
            return Camera();
        }
    }

    std::vector<Camera> Camera::findAll()
    {
        std::vector<Camera> cameras;
        auto                result = DBManager::getInstance().executeQuery("SELECT * FROM cameras;");

        for (const auto& row : result)
        {
            crow::json::wvalue json;
            json["id"]           = std::stoi(row.at("id"));
            json["name"]         = row.at("name");
            json["ip"]           = row.at("ip");
            json["stream_url"]   = row.at("stream_url");
            json["online"]       = std::stoi(row.at("online")) != 0;
            json["created_at"]   = row.at("created_at");
            json["updated_at"]   = row.at("updated_at");
            json["use_model_id"] = row.at("use_model_id");
            json["coding_type"]  = row.at("coding_type");

            auto detectionTypes = crow::json::load(row.at("detection_types"));
            if (detectionTypes)
            {
                json["detection_types"] = std::move(detectionTypes);
            }

            auto dangerTypes = crow::json::load(row.at("danger_types"));
            if (dangerTypes)
            {
                json["danger_types"] = std::move(dangerTypes);
            }

            json["nms_thresh"]    = std::stof(row.at("nms_thresh"));
            json["conf_thresh"]   = std::stof(row.at("conf_thresh"));
            json["point_type_id"] = std::stoi(row.at("point_type_id"));

            auto points = crow::json::load(row.at("points"));
            if (points)
            {
                json["points"] = std::move(points);
            }

            std::string jsonStr = json.dump();
            auto        rvalue  = crow::json::load(jsonStr);
            cameras.emplace_back(rvalue);
        }

        return cameras;
    }

    bool Camera::create(const Camera& camera)
    {
        try
        {
            std::stringstream ss;
            ss << "INSERT INTO cameras (name, ip, stream_url, online, use_model_id, coding_type, "
               << "detection_types, danger_types, nms_thresh, conf_thresh, point_type_id, points) "
               << "VALUES ('" << camera.name << "', '" << camera.ip << "', '" << camera.stream_url << "', " << (camera.online ? 1 : 0) << ", '"
               << camera.use_model_id << "', '" << camera.coding_type << "', '" << camera.toJson()["detection_types"].dump() << "', '"
               << camera.toJson()["danger_types"].dump() << "', " << camera.nms_thresh << ", " << camera.conf_thresh << ", " << camera.point_type_id
               << ", '" << camera.toJson()["points"].dump() << "');";

            DBManager::getInstance().executeQuery(ss.str());
            return true;
        }
        catch (const std::exception& e)
        {
            CROW_LOG_ERROR << "Error creating camera: " << e.what();
            return false;
        }
    }

    bool Camera::update(const Camera& camera)
    {
        try
        {
            std::stringstream ss;
            ss << "UPDATE cameras SET "
               << "name = '" << camera.name << "', "
               << "ip = '" << camera.ip << "', "
               << "stream_url = '" << camera.stream_url << "', "
               << "online = " << (camera.online ? 1 : 0) << ", "
               << "use_model_id = '" << camera.use_model_id << "', "
               << "coding_type = '" << camera.coding_type << "', "
               << "detection_types = '" << camera.toJson()["detection_types"].dump() << "', "
               << "danger_types = '" << camera.toJson()["danger_types"].dump() << "', "
               << "nms_thresh = " << camera.nms_thresh << ", "
               << "conf_thresh = " << camera.conf_thresh << ", "
               << "point_type_id = " << camera.point_type_id << ", "
               << "points = '" << camera.toJson()["points"].dump() << "', "
               << "updated_at = CURRENT_TIMESTAMP "
               << "WHERE id = " << camera.id << ";";

            DBManager::getInstance().executeQuery(ss.str());
            return true;
        }
        catch (const std::exception& e)
        {
            CROW_LOG_ERROR << "Error updating camera: " << e.what();
            return false;
        }
    }

    bool Camera::remove(int id)
    {
        try
        {
            std::stringstream ss;
            ss << "DELETE FROM cameras WHERE id = " << id << ";";
            DBManager::getInstance().executeQuery(ss.str());
            return true;
        }
        catch (const std::exception& e)
        {
            CROW_LOG_ERROR << "Error deleting camera: " << e.what();
            return false;
        }
    }
}  // namespace api