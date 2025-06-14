#pragma once

#include <libnm/NetworkManager.h>
#include <string>
#include <vector>

class WifiManager
{
  public:
    WifiManager();
    ~WifiManager();

    struct WifiItem
    {
        std::string ssid;      // wifi名称
        std::string security;  // 安全类型
        int         strength;  // 信号强度
        int         bitrate;   // 比特率
    };

    // 扫描wifi列表
    std::vector<WifiItem> scanWifi();
};