cmake_minimum_required(VERSION 3.12)

# 交叉编译环境
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 交叉编译器
set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

# Sysroot
set(SYSROOT_PATH "$ENV{HOME}/rv1126-rootfs")
set(CMAKE_SYSROOT ${SYSROOT_PATH})
set(CMAKE_FIND_ROOT_PATH ${SYSROOT_PATH})

# 查找路径限制
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# 避免编译检测
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

# pkg-config
set(ENV{PKG_CONFIG_SYSROOT_DIR} ${SYSROOT_PATH})
set(ENV{PKG_CONFIG_PATH} "${SYSROOT_PATH}/usr/lib/pkgconfig:${SYSROOT_PATH}/usr/share/pkgconfig:${SYSROOT_PATH}/usr/lib/arm-linux-gnueabihf/pkgconfig")

# 项目名
get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${CURRENT_DIR_NAME} VERSION 0.1 LANGUAGES CXX)

# 输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SYSROOT}/userdata/Demo/${CURRENT_DIR_NAME})

# 编译选项
add_compile_options(-Wno-psabi -g3 -O0)

# 源文件
file(GLOB SRC_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/routes/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/routes/camera/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/models/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/middleware/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/database/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/*.cpp
)

# 可执行文件
add_executable(${PROJECT_NAME} ${SRC_FILES})

# 查找依赖
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBNM REQUIRED libnm)
pkg_check_modules(DBUS REQUIRED dbus-1)

# 查找并链接 nlohmann_json 库
find_package(nlohmann_json CONFIG REQUIRED)

# Boost
set(BOOST_ROOT ${SYSROOT_PATH}/usr)
find_package(Boost REQUIRED COMPONENTS system)

# ASIO
find_path(ASIO_INCLUDE_DIR NAMES asio.hpp HINTS ${SYSROOT_PATH}/usr/include ${CMAKE_CURRENT_SOURCE_DIR}/include)
if(NOT ASIO_INCLUDE_DIR)
    message(FATAL_ERROR "ASIO not found")
endif()

# SQLite
find_path(SQLITE_INCLUDE_DIR NAMES sqlite3.h HINTS ${SYSROOT_PATH}/usr/include ${CMAKE_CURRENT_SOURCE_DIR}/include)
if(NOT SQLITE_INCLUDE_DIR)
    message(FATAL_ERROR "SQLite not found")
endif()


# 头文件
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/api/routes
    ${CMAKE_CURRENT_SOURCE_DIR}/include/api/routes/camera
    ${CMAKE_CURRENT_SOURCE_DIR}/include/api/models
    ${CMAKE_CURRENT_SOURCE_DIR}/include/api/middleware
    ${CMAKE_CURRENT_SOURCE_DIR}/include/api/database
    ${CMAKE_CURRENT_SOURCE_DIR}/include/utils
    ${LIBNM_INCLUDE_DIRS}
    ${DBUS_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${ASIO_INCLUDE_DIR}
    ${SQLITE_INCLUDE_DIR}
    ${nlohmann_json_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${LIBNM_LIBRARIES}
    ${DBUS_LIBRARIES}
    Boost::system
    -lpthread
    -lsqlite3
    nlohmann_json::nlohmann_json
)

# 预编译头（加速）
target_precompile_headers(${PROJECT_NAME} PRIVATE <crow_all.h>)