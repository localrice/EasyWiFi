#ifndef EASYWIFI_H
#define EASYWIFI_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

class EasyWiFi {
    public:
        EasyWiFi();

        void begin();
        void loop();
        void reset();

        void saveCredentials(const char* ssid, const char* password);
        void loadCredentials();
        void printCredentials();
        
        // to set Access Point (AP) credentials.
        void setAP(const char* name, const char* password = "");

        // to set custom CSS file
        void setCSS(const char* cssUrl);
    private:
        bool portalActive = false;
        String ssid;
        String password;

        // default AP values
        const char* APName = "EasyWiFi setup";
        const char* APPassword = "";

        // optional css by the user
        const char* userCSS = nullptr;

        // reconnect handler
        unsigned long lastReconnectAttempt = 0;
        int reconnectAttempts = 0;
        const int maxReconnectAttempts = 10; // config it
        const unsigned long reconnectInterval = 5000; // 5 seconds

        void tryConnect();
        void startPortal();
        void handleClient();
};

#endif
