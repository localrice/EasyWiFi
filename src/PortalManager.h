#ifndef PORTALMANAGER_H
#define PORTALMANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>

/**
 * @brief Manages the captive portal for WiFi configuration
 * 
 * Handles the web server, DNS server, network scanning, and HTML page generation
 * for the WiFi configuration portal.
 */
class PortalManager {
public:
    /**
     * @brief Callback type for when credentials are saved via portal
     */
    using SaveCredentialCallback = std::function<void(const String&, const String&)>;

    PortalManager();
    ~PortalManager();

    /**
     * @brief Start the captive portal
     * @param apName Access Point name
     * @param apPassword Access Point password (empty for open network)
     * @return true if portal started successfully
     */
    bool start(const char* apName, const char* apPassword = "");

    /**
     * @brief Stop the captive portal
     */
    void stop();

    /**
     * @brief Check if portal is currently active
     * @return true if portal is active
     */
    bool isActive() const { return portalActive; }

    /**
     * @brief Handle incoming client requests (call from loop)
     */
    void handleClient();

    /**
     * @brief Set custom CSS file URL for portal page
     * @param cssUrl URL to CSS file (served from LittleFS)
     */
    void setCustomCSS(const char* cssUrl);

    /**
     * @brief Set callback for when credentials are saved
     * @param callback Function to call when credentials are saved
     */
    void setOnSaveCredential(SaveCredentialCallback callback);

    /**
     * @brief Get the portal IP address
     * @return IP address of the access point
     */
    IPAddress getPortalIP() const;

private:
    ESP8266WebServer* server;
    DNSServer* dnsServer;
    bool portalActive;
    const char* customCSS;

    SaveCredentialCallback onSaveCredentialCallback;

    static const byte DNS_PORT = 53;

    /**
     * @brief Build the HTML page for the portal
     * @param cssBlock CSS content to include in the page
     * @return Complete HTML page as String
     */
    String buildPortalPage(const String& cssBlock) const;

    /**
     * @brief Get the default CSS styling
     * @return Default CSS as const char*
     */
    const char* getDefaultCSS() const;

    /**
     * @brief Setup web server routes and handlers
     */
    void setupRoutes();

    /**
     * @brief Handle root page request
     */
    void handleRoot();

    /**
     * @brief Handle save credentials request
     */
    void handleSave();

    /**
     * @brief Handle network scan request
     */
    void handleScan();

    /**
     * @brief Handle captive portal detection requests
     */
    void handleCaptivePortalDetect();
};

#endif
