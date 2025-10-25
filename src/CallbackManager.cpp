#include "CallbackManager.h"

CallbackManager::CallbackManager()
    : onConnectCallback(nullptr),
      onDisconnectCallback(nullptr),
      onSaveCallback(nullptr) {}

void CallbackManager::setOnConnect(ConnectCallback callback) {
    onConnectCallback = callback;
    Serial.println("CallbackManager: Connect callback registered");
}

void CallbackManager::setOnDisconnect(DisconnectCallback callback) {
    onDisconnectCallback = callback;
    Serial.println("CallbackManager: Disconnect callback registered");
}

void CallbackManager::setOnSave(SaveCallback callback) {
    onSaveCallback = callback;
    Serial.println("CallbackManager: Save callback registered");
}

void CallbackManager::notifyConnect(const String& ssid, const IPAddress& ip) {
    if (onConnectCallback) {
        Serial.printf("CallbackManager: Notifying connect - SSID: %s, IP: %s\n", 
                     ssid.c_str(), ip.toString().c_str());
        onConnectCallback(ssid, ip);
    }
}

void CallbackManager::notifyDisconnect(const String& ssid) {
    if (onDisconnectCallback) {
        Serial.printf("CallbackManager: Notifying disconnect - SSID: %s\n", ssid.c_str());
        onDisconnectCallback(ssid);
    }
}

void CallbackManager::notifySave(const String& ssid, const String& password) {
    if (onSaveCallback) {
        Serial.printf("CallbackManager: Notifying save - SSID: %s\n", ssid.c_str());
        onSaveCallback(ssid, password);
    }
}
