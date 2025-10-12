#pragma once
#include <string>
#include <iostream>
#include "WiFiCallbacks.h"

class WiFiManager {
private:
    WiFiEventCallbacks callbacks;

public:
    void setCallbacks(const WiFiEventCallbacks& cb) { callbacks = cb; }

    void connectToNetwork(const std::string& ssid) {
        std::cout << "Connecting to: " << ssid << std::endl;
        if (callbacks.onConnect) callbacks.onConnect(ssid);
    }

    void disconnect() {
        std::string ssid = "CurrentNetwork";
        std::cout << "Disconnecting from: " << ssid << std::endl;
        if (callbacks.onDisconnect) callbacks.onDisconnect(ssid);
    }

    void saveNetwork(const std::string& ssid) {
        std::cout << "Saving network: " << ssid << std::endl;
        if (callbacks.onSave) callbacks.onSave(ssid);
    }
};
