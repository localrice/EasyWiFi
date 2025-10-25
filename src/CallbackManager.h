#ifndef CALLBACKMANAGER_H
#define CALLBACKMANAGER_H

#include <Arduino.h>
#include <functional>

/**
 * @brief Manages event callbacks for WiFi connection events
 * 
 * Handles registration and notification of callbacks for connect,
 * disconnect, and credential save events.
 */
class CallbackManager {
public:
    /**
     * @brief Callback type for connection events
     * @param ssid The SSID of the connected network
     * @param ip The local IP address
     */
    using ConnectCallback = std::function<void(const String&, const IPAddress&)>;

    /**
     * @brief Callback type for disconnection events
     * @param ssid The SSID of the disconnected network
     */
    using DisconnectCallback = std::function<void(const String&)>;

    /**
     * @brief Callback type for credential save events
     * @param ssid The SSID that was saved
     * @param password The password that was saved
     */
    using SaveCallback = std::function<void(const String&, const String&)>;

    CallbackManager();

    /**
     * @brief Set the callback for connection events
     * @param callback Function to call when connected
     */
    void setOnConnect(ConnectCallback callback);

    /**
     * @brief Set the callback for disconnection events
     * @param callback Function to call when disconnected
     */
    void setOnDisconnect(DisconnectCallback callback);

    /**
     * @brief Set the callback for credential save events
     * @param callback Function to call when credentials are saved
     */
    void setOnSave(SaveCallback callback);

    /**
     * @brief Notify connect event
     * @param ssid The SSID of the connected network
     * @param ip The local IP address
     */
    void notifyConnect(const String& ssid, const IPAddress& ip);

    /**
     * @brief Notify disconnect event
     * @param ssid The SSID of the disconnected network
     */
    void notifyDisconnect(const String& ssid);

    /**
     * @brief Notify credential save event
     * @param ssid The SSID that was saved
     * @param password The password that was saved
     */
    void notifySave(const String& ssid, const String& password);

    /**
     * @brief Check if connect callback is set
     * @return true if callback is set
     */
    bool hasConnectCallback() const { return onConnectCallback != nullptr; }

    /**
     * @brief Check if disconnect callback is set
     * @return true if callback is set
     */
    bool hasDisconnectCallback() const { return onDisconnectCallback != nullptr; }

    /**
     * @brief Check if save callback is set
     * @return true if callback is set
     */
    bool hasSaveCallback() const { return onSaveCallback != nullptr; }

private:
    ConnectCallback onConnectCallback;
    DisconnectCallback onDisconnectCallback;
    SaveCallback onSaveCallback;
};

#endif
