#include "CredentialManager.h"
#include <algorithm>

const char* const CredentialManager::CREDENTIAL_FILE = "/wifi_credentials.txt";

CredentialManager::CredentialManager() : activeCredentialIndex(-1) {}

bool CredentialManager::loadCredentials() {
    Serial.println("CredentialManager: Loading credentials");
    credentials.clear();
    activeCredentialIndex = -1;

    File file = LittleFS.open(CREDENTIAL_FILE, "r");
    if (!file) {
        Serial.println("CredentialManager: No saved credentials found");
        return false;
    }

    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) {
            continue;
        }

        int separatorIndex = line.indexOf('\t');

        // Support legacy format (SSID on one line, password on next)
        if (separatorIndex == -1) {
            String legacyPassword = file.readStringUntil('\n');
            legacyPassword.trim();
            if (line.length() > 0) {
                credentials.push_back({line, legacyPassword});
            }
            break; 
        }

        // New format: SSID\tPassword
        credentials.push_back({
            line.substring(0, separatorIndex),
            line.substring(separatorIndex + 1)
        });
    }

    file.close();

    if (!credentials.empty()) {
        activeCredentialIndex = 0;
        Serial.printf("CredentialManager: Loaded %u network(s). First SSID: %s\n", 
                     static_cast<unsigned int>(credentials.size()), 
                     credentials[0].ssid.c_str());
        return true;
    }

    Serial.println("CredentialManager: No valid credentials found in file");
    return false;
}

bool CredentialManager::saveCredential(const String& ssid, const String& password) {
    Serial.println("CredentialManager: Saving credential");
    
    if (ssid.length() == 0) {
        Serial.println("CredentialManager: Cannot save empty SSID");
        return false;
    }

    int existingIndex = findCredentialIndex(ssid);
    
    if (existingIndex != -1) {
        // Update existing credential and move to front (most recently used)
        credentials[existingIndex].password = password;
        if (existingIndex != 0) {
            std::rotate(credentials.begin(), 
                       credentials.begin() + existingIndex, 
                       credentials.begin() + existingIndex + 1);
        }
        activeCredentialIndex = 0;
    } else {
        // Add new credential at the beginning
        credentials.insert(credentials.begin(), {ssid, password});
        activeCredentialIndex = 0;
    }

    if (!persistCredentials()) {
        Serial.println("CredentialManager: Failed to persist credentials");
        return false;
    }

    Serial.printf("CredentialManager: Saved SSID: %s\n", ssid.c_str());
    return true;
}

void CredentialManager::clearCredentials() {
    Serial.println("CredentialManager: Clearing all credentials");
    LittleFS.remove(CREDENTIAL_FILE);
    credentials.clear();
    activeCredentialIndex = -1;
}

void CredentialManager::printCredentials() const {
    Serial.println("CredentialManager: Printing credentials");
    if (credentials.empty()) {
        Serial.println("No credentials stored");
        return;
    }

    for (size_t i = 0; i < credentials.size(); ++i) {
        const auto &cred = credentials[i];
        Serial.printf("[%u] SSID: %s\n", static_cast<unsigned>(i), cred.ssid.c_str());
        Serial.printf("    Password: %s\n", cred.password.c_str());
        if (static_cast<int>(i) == activeCredentialIndex) {
            Serial.println("    [ACTIVE]");
        }
    }
}

const CredentialManager::Credential* CredentialManager::getCredential(size_t index) const {
    if (index >= credentials.size()) {
        return nullptr;
    }
    return &credentials[index];
}

const CredentialManager::Credential* CredentialManager::getActiveCredential() const {
    if (activeCredentialIndex < 0 || static_cast<size_t>(activeCredentialIndex) >= credentials.size()) {
        return nullptr;
    }
    return &credentials[activeCredentialIndex];
}

bool CredentialManager::setActiveCredential(size_t index) {
    if (index >= credentials.size()) {
        activeCredentialIndex = -1;
        return false;
    }
    activeCredentialIndex = static_cast<int>(index);
    return true;
}

bool CredentialManager::persistCredentials() const {
    File file = LittleFS.open(CREDENTIAL_FILE, "w");
    if (!file) {
        Serial.println("CredentialManager: Failed to open file for writing");
        return false;
    }

    for (const auto &cred : credentials) {
        file.println(cred.ssid + "\t" + cred.password);
    }

    file.close();
    return true;
}

int CredentialManager::findCredentialIndex(const String& ssid) const {
    for (size_t i = 0; i < credentials.size(); ++i) {
        if (credentials[i].ssid == ssid) {
            return static_cast<int>(i);
        }
    }
    return -1;
}
