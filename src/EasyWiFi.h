#ifndef EASYWIFI_H
#define EASYWIFI_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <vector>
#include <functional>

class EasyWiFi {
    public:
        EasyWiFi();

        void begin();
        void loop();
        void reset();

        void saveCredentials(const char* networkSsid, const char* networkPassword);
        void loadCredentials();
        void printCredentials();
        
        // to set Access Point (AP) credentials.
        void setAP(const char* name, const char* password = "");

        // to set custom CSS file
        void setCSS(const char* cssUrl);

        // to set maximum WiFi reconnect attempts and interval (ms)
        void setReconnectParams(int maxAttempts, unsigned long interval);

        // STA mode static IP
        void setStaticIP(IPAddress ip, IPAddress gateway, IPAddress subnet, IPAddress dns = IPAddress(8,8,8,8));

        using ConnectCallback = std::function<void(const String&, const IPAddress&)>;
        using DisconnectCallback = std::function<void(const String&)>;
        using SaveCallback = std::function<void(const String&, const String&)>;

        void setOnConnect(ConnectCallback cb);
        void setOnDisconnect(DisconnectCallback cb);
        void setOnSave(SaveCallback cb);
    private:
        bool portalActive = false;
        String ssid;
        String password;

        bool wasConnected = false;

        // default AP values
        const char* APName = "EasyWiFi setup";
        const char* APPassword = "";

        // optional css by the user
        const char* userCSS = nullptr;

        // reconnect handler
        unsigned long lastReconnectAttempt = 0;
        int reconnectAttempts = 0;
        int maxReconnectAttempts = 10; // config it
        unsigned long reconnectInterval = 5000; // 5 seconds

        bool useStaticIP = false;
        IPAddress staticIP;
        IPAddress staticGateway;
        IPAddress staticSubnet;
        IPAddress staticDNS;

        struct Credential {
            String ssid;
            String password;
        };

        static const char* const CREDENTIAL_FILE;

        std::vector<Credential> credentials;
        int activeCredentialIndex = -1;

        void tryConnect(bool preferNext = false);
        void startPortal();
        void stopPortal();
        void handleClient();

        void applyActiveCredential(size_t index);
        bool attemptConnection(const Credential& cred, unsigned long timeout);
        bool persistCredentials() const;
        String buildPortalPage(const String& cssBlock) const;
        void ensureStationMode();

        ConnectCallback onConnectCallback = nullptr;
        DisconnectCallback onDisconnectCallback = nullptr;
        SaveCallback onSaveCallback = nullptr;

        void notifyConnect();
        void notifyDisconnect();
};

#endif
