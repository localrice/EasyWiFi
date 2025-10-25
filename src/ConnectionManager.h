#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "CredentialManager.h"

/**
 * @brief Manages WiFi connections and reconnection logic
 * 
 * Handles connection attempts, automatic reconnection, static IP configuration,
 * and connection state tracking.
 */
class ConnectionManager {
public:
    ConnectionManager(CredentialManager& credManager);

    /**
     * @brief Attempt to connect to WiFi using stored credentials
     * @param preferNext If true, try the next credential in the list first
     * @return true if connected successfully, false otherwise
     */
    bool connect(bool preferNext = false);

    /**
     * @brief Attempt to connect to a specific credential
     * @param cred The credential to use
     * @param timeout Connection timeout in milliseconds
     * @return true if connected successfully, false otherwise
     */
    bool attemptConnection(const CredentialManager::Credential& cred, unsigned long timeout = 10000);

    /**
     * @brief Handle reconnection logic (call from loop)
     * @return true if still connected or reconnected, false if connection lost
     */
    bool handleReconnection();

    /**
     * @brief Check if currently connected to WiFi
     * @return true if connected
     */
    bool isConnected() const;

    /**
     * @brief Check if was previously connected (for state change detection)
     * @return true if was connected
     */
    bool wasConnected() const { return _wasConnected; }

    /**
     * @brief Set the was connected state
     * @param connected The new state
     */
    void setWasConnected(bool connected) { _wasConnected = connected; }

    /**
     * @brief Disconnect from WiFi
     */
    void disconnect();

    /**
     * @brief Configure static IP settings
     * @param ip Static IP address
     * @param gateway Gateway address
     * @param subnet Subnet mask
     * @param dns DNS server address
     */
    void setStaticIP(IPAddress ip, IPAddress gateway, IPAddress subnet, IPAddress dns = IPAddress(8, 8, 8, 8));

    /**
     * @brief Configure reconnection parameters
     * @param maxAttempts Maximum reconnection attempts before cycling networks
     * @param interval Time between reconnection attempts in milliseconds
     */
    void setReconnectParams(int maxAttempts, unsigned long interval);

    /**
     * @brief Reset reconnection attempt counter
     */
    void resetReconnectAttempts();

    /**
     * @brief Get current reconnection attempt count
     * @return Number of reconnection attempts
     */
    int getReconnectAttempts() const { return reconnectAttempts; }

    /**
     * @brief Get maximum reconnection attempts
     * @return Maximum attempts before cycling networks
     */
    int getMaxReconnectAttempts() const { return maxReconnectAttempts; }

    /**
     * @brief Get the current SSID (from active credential)
     * @return SSID string
     */
    String getCurrentSSID() const;

    /**
     * @brief Get the local IP address
     * @return IP address or INADDR_NONE if not connected
     */
    IPAddress getLocalIP() const;

private:
    CredentialManager& credentialManager;

    // Reconnection state
    unsigned long lastReconnectAttempt;
    int reconnectAttempts;
    int maxReconnectAttempts;
    unsigned long reconnectInterval;
    bool _wasConnected;

    // Static IP configuration
    bool useStaticIP;
    IPAddress staticIP;
    IPAddress staticGateway;
    IPAddress staticSubnet;
    IPAddress staticDNS;

    /**
     * @brief Ensure WiFi is in station mode
     */
    void ensureStationMode();

    /**
     * @brief Apply static IP configuration if enabled
     * @return true if configuration successful or not needed
     */
    bool applyStaticIPConfig();
};

#endif
