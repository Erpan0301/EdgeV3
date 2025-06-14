#pragma once

#include "api/database/db_manager.hpp"

namespace api
{
    class DBInitializer
    {
      public:
        static void initialize();

      private:
        static void createTables();
        static void createUsersTable();
        // 可以添加其他表的创建方法
    };

}  // namespace api