#include "WiFiManager.h"

int main() {
    WiFiManager manager;

    WiFiEventCallbacks cb;
    cb.onConnect = [](const std::string& ssid){
        std::cout << "[Callback] Connected to " << ssid << std::endl;
    };
    cb.onDisconnect = [](const std::string& ssid){
        std::cout << "[Callback] Disconnected from " << ssid << std::endl;
    };
    cb.onSave = [](const std::string& ssid){
        std::cout << "[Callback] Network saved: " << ssid << std::endl;
    };

    manager.setCallbacks(cb);

    manager.connectToNetwork("Home WiFi");
    manager.saveNetwork("Home WiFi");
    manager.disconnect();

    return 0;
}
