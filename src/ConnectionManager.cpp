#include "ConnectionManager.h"

ConnectionManager::ConnectionManager(CredentialManager& credManager)
    : credentialManager(credManager),
      lastReconnectAttempt(0),
      reconnectAttempts(0),
      maxReconnectAttempts(10),
      reconnectInterval(5000),
      _wasConnected(false),
      useStaticIP(false),
      staticIP(0, 0, 0, 0),
      staticGateway(0, 0, 0, 0),
      staticSubnet(255, 255, 255, 0),
      staticDNS(8, 8, 8, 8) {}

bool ConnectionManager::connect(bool preferNext) {
    reconnectAttempts = 0;
    lastReconnectAttempt = millis();

    if (credentialManager.isEmpty()) {
        Serial.println("ConnectionManager: No credentials available");
        return false;
    }

    ensureStationMode();
    
    if (!applyStaticIPConfig()) {
        Serial.println("ConnectionManager: Warning - Static IP configuration failed");
    }

    const size_t totalNetworks = credentialManager.getCredentialCount();
    size_t startIndex = 0;
    
    int activeIndex = credentialManager.getActiveCredentialIndex();
    if (activeIndex >= 0 && static_cast<size_t>(activeIndex) < totalNetworks) {
        startIndex = static_cast<size_t>(activeIndex);
        if (preferNext && totalNetworks > 1) {
            startIndex = (startIndex + 1) % totalNetworks;
        }
    }

    const unsigned long timeout = 10000;
    for (size_t offset = 0; offset < totalNetworks; ++offset) {
        const size_t index = (startIndex + offset) % totalNetworks;
        credentialManager.setActiveCredential(index);
        
        const auto* cred = credentialManager.getCredential(index);
        if (cred && attemptConnection(*cred, timeout)) {
            return true;
        }
    }

    Serial.println("ConnectionManager: Failed to connect to any saved network");
    credentialManager.setActiveCredential(0); // Reset to first credential
    _wasConnected = false;
    WiFi.disconnect(false);
    return false;
}

bool ConnectionManager::attemptConnection(const CredentialManager::Credential& cred, unsigned long timeout) {
    Serial.printf("ConnectionManager: Attempting to connect to SSID: %s\n", cred.ssid.c_str());
    ensureStationMode();
    WiFi.begin(cred.ssid.c_str(), cred.password.c_str());

    const wl_status_t result = static_cast<wl_status_t>(WiFi.waitForConnectResult(timeout));

    if (result == WL_CONNECTED) {
        Serial.printf("ConnectionManager: Connected! IP address: %s\n", WiFi.localIP().toString().c_str());
        _wasConnected = true;
        reconnectAttempts = 0;
        return true;
    }

    Serial.println("ConnectionManager: Connection failed");
    WiFi.disconnect(false);
    return false;
}

bool ConnectionManager::handleReconnection() {
    const wl_status_t status = WiFi.status();
    
    // Check if connected
    if (status == WL_CONNECTED) {
        if (!_wasConnected) {
            Serial.println("ConnectionManager: Reconnected to WiFi");
            _wasConnected = true;
        }
        reconnectAttempts = 0;
        return true;
    }

    // Connection lost
    if (_wasConnected) {
        Serial.println("ConnectionManager: WiFi lost, attempting reconnect...");
        _wasConnected = false;
    }

    // Check if it's time to attempt reconnection
    const unsigned long now = millis();
    if (now - lastReconnectAttempt < reconnectInterval) {
        return false;
    }

    lastReconnectAttempt = now;
    ++reconnectAttempts;

    const auto* activeCred = credentialManager.getActiveCredential();
    if (activeCred) {
        Serial.printf("ConnectionManager: Reconnect attempt %d to SSID: %s\n", 
                     reconnectAttempts, activeCred->ssid.c_str());
        ensureStationMode();
        WiFi.begin(activeCred->ssid.c_str(), activeCred->password.c_str());
    }

    // If too many failures, try next network
    if (reconnectAttempts > maxReconnectAttempts) {
        Serial.println("ConnectionManager: Too many failures, cycling saved networks");
        reconnectAttempts = 0;
        connect(true); // Try next network
    }

    return false;
}

bool ConnectionManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

void ConnectionManager::disconnect() {
    WiFi.disconnect(false);
    _wasConnected = false;
}

void ConnectionManager::setStaticIP(IPAddress ip, IPAddress gateway, IPAddress subnet, IPAddress dns) {
    staticIP = ip;
    staticGateway = gateway;
    staticSubnet = subnet;
    staticDNS = dns;
    useStaticIP = true;
    Serial.printf("ConnectionManager: Static IP configured: %s\n", ip.toString().c_str());
}

void ConnectionManager::setReconnectParams(int maxAttempts, unsigned long interval) {
    maxReconnectAttempts = maxAttempts;
    reconnectInterval = interval;
    Serial.printf("ConnectionManager: Reconnect params set - Max attempts: %d, Interval: %lu ms\n", 
                 maxAttempts, interval);
}

void ConnectionManager::resetReconnectAttempts() {
    reconnectAttempts = 0;
    lastReconnectAttempt = millis();
}

String ConnectionManager::getCurrentSSID() const {
    const auto* activeCred = credentialManager.getActiveCredential();
    return activeCred ? activeCred->ssid : String("");
}

IPAddress ConnectionManager::getLocalIP() const {
    return WiFi.localIP();
}

void ConnectionManager::ensureStationMode() {
    const WiFiMode_t mode = WiFi.getMode();
    if (mode == WIFI_STA) {
        return;
    }

    if (mode == WIFI_AP || mode == WIFI_AP_STA) {
        WiFi.softAPdisconnect(true);
    }

    WiFi.mode(WIFI_STA);
}

bool ConnectionManager::applyStaticIPConfig() {
    if (!useStaticIP) {
        return true;
    }

    if (!WiFi.config(staticIP, staticGateway, staticSubnet, staticDNS)) {
        Serial.println("ConnectionManager: Failed to configure static IP, falling back to DHCP");
        return false;
    }
    
    Serial.printf("ConnectionManager: Using static IP %s\n", staticIP.toString().c_str());
    return true;
}
