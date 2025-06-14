#pragma once

#include "api/database/db_manager.hpp"
#include "crow_all.h"

#include <string>
#include <vector>

namespace api
{
    class Camera
    {
      public:
        // Constructors
        Camera() = default;
        Camera(const crow::json::rvalue& json);

        // Getters
        int                                    getId() const { return id; }
        const std::string&                     getName() const { return name; }
        const std::string&                     getIp() const { return ip; }
        const std::string&                     getStreamUrl() const { return stream_url; }
        bool                                   isOnline() const { return online; }
        const std::string&                     getCreatedAt() const { return created_at; }
        const std::string&                     getUpdatedAt() const { return updated_at; }
        const std::string&                     getUseModelId() const { return use_model_id; }
        const std::string&                     getCodingType() const { return coding_type; }
        const std::vector<int>&                getDetectionTypes() const { return detection_types; }
        const std::vector<int>&                getDangerTypes() const { return danger_types; }
        float                                  getNmsThresh() const { return nms_thresh; }
        float                                  getConfThresh() const { return conf_thresh; }
        int                                    getPointTypeId() const { return point_type_id; }
        const std::vector<crow::json::wvalue>& getPoints() const { return points; }

        // Setters
        void setName(const std::string& value) { name = value; }
        void setIp(const std::string& value) { ip = value; }
        void setStreamUrl(const std::string& value) { stream_url = value; }
        void setOnline(bool value) { online = value; }
        void setUseModelId(const std::string& value) { use_model_id = value; }
        void setCodingType(const std::string& value) { coding_type = value; }
        void setDetectionTypes(const std::vector<int>& value) { detection_types = value; }
        void setDangerTypes(const std::vector<int>& value) { danger_types = value; }
        void setNmsThresh(float value) { nms_thresh = value; }
        void setConfThresh(float value) { conf_thresh = value; }
        void setPointTypeId(int value) { point_type_id = value; }
        void setPoints(std::vector<crow::json::wvalue>&& value) { points = std::move(value); }
        void setPoints(const std::vector<crow::json::wvalue>& value)
        {
            points.clear();
            for (const auto& point : value)
            {
                points.push_back(crow::json::wvalue(point));
            }
        }

        // Convert to JSON
        crow::json::wvalue toJson() const;

        // Static methods for database operations
        static Camera              findById(int id);
        static std::vector<Camera> findAll();
        static bool                create(const Camera& camera);
        static bool                update(const Camera& camera);
        static bool                remove(int id);

      private:
        int                             id {0};
        std::string                     name;
        std::string                     ip;
        std::string                     stream_url;
        bool                            online {false};
        std::string                     created_at;
        std::string                     updated_at;
        std::string                     use_model_id;
        std::string                     coding_type;
        std::vector<int>                detection_types;
        std::vector<int>                danger_types;
        float                           nms_thresh {0.45f};
        float                           conf_thresh {0.5f};
        int                             point_type_id {1};
        std::vector<crow::json::wvalue> points;
    };
}  // namespace api