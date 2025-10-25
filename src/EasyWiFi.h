#ifndef EASYWIFI_H
#define EASYWIFI_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <functional>

// Include modularized components
#include "CredentialManager.h"
#include "ConnectionManager.h"
#include "PortalManager.h"
#include "CallbackManager.h"

/**
 * @brief Main EasyWiFi class - Simplified WiFi management for ESP8266
 * 
 * Provides easy WiFi connection management with automatic reconnection,
 * captive portal configuration, and credential storage.
 * 
 * This class coordinates between modularized components:
 * - CredentialManager: Handles credential storage and persistence
 * - ConnectionManager: Manages WiFi connections and reconnection
 * - PortalManager: Handles the configuration web portal
 * - CallbackManager: Manages event callbacks
 */
class EasyWiFi {
public:
    EasyWiFi();

    /**
     * @brief Initialize EasyWiFi and attempt to connect
     * 
     * Mounts LittleFS, loads saved credentials, and attempts connection.
     * If no credentials exist or connection fails, starts the configuration portal.
     */
    void begin();

    /**
     * @brief Main loop handler - must be called regularly
     * 
     * Handles portal requests if active, otherwise manages reconnection logic.
     */
    void loop();

    /**
     * @brief Reset all saved credentials
     * 
     * Removes credential file and clears all stored networks.
     */
    void reset();

    /**
     * @brief Save WiFi credentials
     * @param networkSsid The network SSID
     * @param networkPassword The network password
     */
    void saveCredentials(const char* networkSsid, const char* networkPassword);

    /**
     * @brief Load credentials from storage
     */
    void loadCredentials();

    /**
     * @brief Print all stored credentials to Serial
     */
    void printCredentials();
    
    /**
     * @brief Set Access Point credentials for configuration portal
     * @param name AP name
     * @param password AP password (empty for open network)
     */
    void setAP(const char* name, const char* password = "");

    /**
     * @brief Set custom CSS file for portal
     * @param cssUrl URL to CSS file in LittleFS
     */
    void setCSS(const char* cssUrl);

    /**
     * @brief Set WiFi reconnection parameters
     * @param maxAttempts Maximum attempts before cycling networks
     * @param interval Time between attempts in milliseconds
     */
    void setReconnectParams(int maxAttempts, unsigned long interval);

    /**
     * @brief Configure static IP for STA mode
     * @param ip Static IP address
     * @param gateway Gateway address
     * @param subnet Subnet mask
     * @param dns DNS server (default: 8.8.8.8)
     */
    void setStaticIP(IPAddress ip, IPAddress gateway, IPAddress subnet, IPAddress dns = IPAddress(8,8,8,8));

    // Callback type definitions (maintained for backward compatibility)
    using ConnectCallback = std::function<void(const String&, const IPAddress&)>;
    using DisconnectCallback = std::function<void(const String&)>;
    using SaveCallback = std::function<void(const String&, const String&)>;

    /**
     * @brief Set callback for connection events
     * @param cb Callback function
     */
    void setOnConnect(ConnectCallback cb);

    /**
     * @brief Set callback for disconnection events
     * @param cb Callback function
     */
    void setOnDisconnect(DisconnectCallback cb);

    /**
     * @brief Set callback for credential save events
     * @param cb Callback function
     */
    void setOnSave(SaveCallback cb);

private:
    // Modularized components
    CredentialManager credentialManager;
    ConnectionManager connectionManager;
    PortalManager portalManager;
    CallbackManager callbackManager;

    // Configuration
    const char* apName;
    const char* apPassword;

    /**
     * @brief Try to connect to WiFi
     * @param preferNext If true, try next network in list
     */
    void tryConnect(bool preferNext = false);

    /**
     * @brief Start the configuration portal
     */
    void startPortal();

    /**
     * @brief Handle credential save from portal
     * @param ssid SSID to save
     * @param password Password to save
     */
    void handleCredentialSave(const String& ssid, const String& password);
};

#endif
