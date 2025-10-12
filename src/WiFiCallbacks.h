#pragma once
#include <functional>
#include <string>

using WiFiCallback = std::function<void(const std::string& ssid)>;

struct WiFiEventCallbacks {
    WiFiCallback onConnect = nullptr;
    WiFiCallback onDisconnect = nullptr;
    WiFiCallback onSave = nullptr;
};
