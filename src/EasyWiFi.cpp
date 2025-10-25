#include "EasyWiFi.h"

/**
 * EasyWiFi - Modularized Implementation
 * 
 * This implementation coordinates between the following modules:
 * - CredentialManager: Credential storage and persistence
 * - ConnectionManager: WiFi connection and reconnection logic
 * - PortalManager: Captive portal web interface
 * - CallbackManager: Event callbacks
 */

EasyWiFi::EasyWiFi()
    : connectionManager(credentialManager),
      apName("EasyWiFi setup"),
      apPassword("") {}

void EasyWiFi::begin() {
    Serial.println("EasyWiFi: begin()");
    
    // Initialize LittleFS
    if (!LittleFS.begin()) {
        Serial.println("EasyWiFi: Failed to mount LittleFS");
        return;
    }
    
    // Load saved credentials
    loadCredentials();
    
    // Set initial connection state
    connectionManager.setWasConnected(WiFi.status() == WL_CONNECTED);
    
    // Attempt connection
    tryConnect();
}

void EasyWiFi::loop() {
    // Handle portal if active
    if (portalManager.isActive()) {
        portalManager.handleClient();
        return;
    }

    // Handle connection state changes
    const bool wasConnectedBefore = connectionManager.wasConnected();
    const bool isConnectedNow = connectionManager.isConnected();

    if (isConnectedNow && !wasConnectedBefore) {
        // Just connected
        Serial.println("EasyWiFi: Reconnected to WiFi");
        connectionManager.setWasConnected(true);
        const auto* activeCred = credentialManager.getActiveCredential();
        if (activeCred) {
            callbackManager.notifyConnect(activeCred->ssid, connectionManager.getLocalIP());
        }
        return;
    }

    if (!isConnectedNow && wasConnectedBefore) {
        // Just disconnected
        Serial.println("EasyWiFi: WiFi lost, attempting reconnect...");
        const auto* activeCred = credentialManager.getActiveCredential();
        if (activeCred) {
            callbackManager.notifyDisconnect(activeCred->ssid);
        }
        connectionManager.setWasConnected(false);
    }

    // Handle reconnection attempts
    if (!isConnectedNow) {
        connectionManager.handleReconnection();
        
        // If too many failures, try next network
        if (connectionManager.getReconnectAttempts() > connectionManager.getMaxReconnectAttempts()) {
            Serial.println("EasyWiFi: Too many failures, cycling saved networks");
            connectionManager.resetReconnectAttempts();
            tryConnect(true);
        }
    }
}

void EasyWiFi::reset() {
    Serial.println("EasyWiFi: reset()");
    credentialManager.clearCredentials();
}

void EasyWiFi::saveCredentials(const char* networkSsid, const char* networkPassword) {
    Serial.println("EasyWiFi: saveCredentials()");
    
    if (credentialManager.saveCredential(String(networkSsid), String(networkPassword))) {
        // Notify callbacks
        callbackManager.notifySave(String(networkSsid), String(networkPassword));
    }
}

void EasyWiFi::loadCredentials() {
    Serial.println("EasyWiFi: loadCredentials()");
    credentialManager.loadCredentials();
}

void EasyWiFi::printCredentials() {
    credentialManager.printCredentials();
}

void EasyWiFi::setAP(const char* name, const char* password) {
    apName = name;
    apPassword = password;
}

void EasyWiFi::setCSS(const char* cssUrl) {
    portalManager.setCustomCSS(cssUrl);
}

void EasyWiFi::setReconnectParams(int maxAttempts, unsigned long interval) {
    connectionManager.setReconnectParams(maxAttempts, interval);
}

void EasyWiFi::setStaticIP(IPAddress ip, IPAddress gateway, IPAddress subnet, IPAddress dns) {
    connectionManager.setStaticIP(ip, gateway, subnet, dns);
}

void EasyWiFi::setOnConnect(ConnectCallback cb) {
    callbackManager.setOnConnect(cb);
}

void EasyWiFi::setOnDisconnect(DisconnectCallback cb) {
    callbackManager.setOnDisconnect(cb);
}

void EasyWiFi::setOnSave(SaveCallback cb) {
    callbackManager.setOnSave(cb);
}

void EasyWiFi::tryConnect(bool preferNext) {
    // Stop portal if active
    portalManager.stop();

    // If no credentials, start portal
    if (credentialManager.isEmpty()) {
        Serial.println("EasyWiFi: No SSID saved, starting portal");
        startPortal();
        return;
    }

    // Attempt connection
    if (connectionManager.connect(preferNext)) {
        // Successfully connected
        const auto* activeCred = credentialManager.getActiveCredential();
        if (activeCred) {
            callbackManager.notifyConnect(activeCred->ssid, connectionManager.getLocalIP());
        }
    } else {
        // Failed to connect to any network, start portal
        Serial.println("EasyWiFi: Failed to connect to any saved network, starting portal");
        startPortal();
    }
}

void EasyWiFi::startPortal() {
    Serial.println("EasyWiFi: startPortal()");
    
    // Set up portal callback
    portalManager.setOnSaveCredential([this](const String& ssid, const String& password) {
        handleCredentialSave(ssid, password);
    });
    
    // Start the portal
    portalManager.start(apName, apPassword);
}

void EasyWiFi::handleCredentialSave(const String& ssid, const String& password) {
    Serial.println("EasyWiFi: handleCredentialSave()");
    
    // Save credentials
    if (credentialManager.saveCredential(ssid, password)) {
        // Notify save callback
        callbackManager.notifySave(ssid, password);
    }
}
