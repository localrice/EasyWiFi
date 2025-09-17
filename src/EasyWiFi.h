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

    private:
        bool portalActive = false;
        String ssid;
        String password;

        void tryConnect();
        void startPortal();
        void handleClient();
};

#endif
