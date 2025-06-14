#include "wifi_manager.hpp"

#include <iostream>
#include <libnm/NetworkManager.h>
#include <libnm/nm-utils.h>

WifiManager::WifiManager()
{
    std::cout << "WifiManager constructor" << std::endl;
}

WifiManager::~WifiManager()
{
    std::cout << "WifiManager destructor" << std::endl;
}

std::vector<WifiManager::WifiItem> WifiManager::scanWifi()
{
    std::cout << "Scanning Wi-Fi networks..." << std::endl;
    std::vector<WifiItem> ssids;

    GError*   error  = nullptr;
    NMClient* client = nm_client_new(nullptr, &error);
    if (!client)
    {
        std::cerr << "Failed to create NMClient: " << error->message << std::endl;
        g_error_free(error);
        return ssids;
    }

    // 获取所有网络设备
    const GPtrArray* devices = nm_client_get_devices(client);
    for (guint i = 0; i < devices->len; i++)
    {
        NMDevice* device = NM_DEVICE(g_ptr_array_index(devices, i));
        if (nm_device_get_device_type(device) != NM_DEVICE_TYPE_WIFI)
            continue;

        // 触发 Wi-Fi 扫描
        NMDeviceWifi* wifi_device = NM_DEVICE_WIFI(device);
        nm_device_wifi_request_scan(wifi_device, nullptr, &error);
        if (error)
        {
            std::cerr << "Scan failed: " << error->message << std::endl;
            g_error_free(error);
            error = nullptr;
            continue;
        }

        // 获取扫描到的接入点
        const GPtrArray* aps = nm_device_wifi_get_access_points(wifi_device);
        for (guint j = 0; j < aps->len; j++)
        {
            NMAccessPoint* ap         = NM_ACCESS_POINT(g_ptr_array_index(aps, j));
            GBytes*        ssid_bytes = nm_access_point_get_ssid(ap);
            if (!ssid_bytes)
                continue;

            const char* ssid =
                nm_utils_ssid_to_utf8(reinterpret_cast<const guint8*>(g_bytes_get_data(ssid_bytes, nullptr)), g_bytes_get_size(ssid_bytes));

            guint8                 strength  = nm_access_point_get_strength(ap);
            guint32                bitrate   = nm_access_point_get_max_bitrate(ap);  // in Kbps
            NM80211ApFlags         flags     = nm_access_point_get_flags(ap);
            NM80211ApSecurityFlags wpa_flags = nm_access_point_get_wpa_flags(ap);
            NM80211ApSecurityFlags rsn_flags = nm_access_point_get_rsn_flags(ap);

            std::string security = "Open";
            if (flags & NM_802_11_AP_FLAGS_PRIVACY)
            {
                if (wpa_flags || rsn_flags)
                    security = "WPA/WPA2";
                else
                    security = "WEP";
            }

            ssids.push_back({ssid, security, static_cast<int>(strength), static_cast<int>(bitrate)});
        }
    }

    g_object_unref(client);
    return ssids;
}