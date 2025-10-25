#ifndef CREDENTIALMANAGER_H
#define CREDENTIALMANAGER_H

#include <Arduino.h>
#include <LittleFS.h>
#include <vector>

/**
 * @brief Manages WiFi credentials storage and persistence
 * 
 * Handles loading, saving, and managing multiple WiFi network credentials.
 * Credentials are stored in LittleFS with tab-separated SSID and password pairs.
 */
class CredentialManager {
public:
    /**
     * @brief Represents a single WiFi credential
     */
    struct Credential {
        String ssid;
        String password;
    };

    CredentialManager();

    /**
     * @brief Load credentials from file system
     * @return true if credentials were loaded successfully, false otherwise
     */
    bool loadCredentials();

    /**
     * @brief Save a new credential or update existing one
     * @param ssid The network SSID
     * @param password The network password
     * @return true if save was successful, false otherwise
     */
    bool saveCredential(const String& ssid, const String& password);

    /**
     * @brief Remove all stored credentials
     */
    void clearCredentials();

    /**
     * @brief Print all stored credentials to Serial
     */
    void printCredentials() const;

    /**
     * @brief Get the list of all credentials
     * @return Reference to the credentials vector
     */
    const std::vector<Credential>& getCredentials() const { return credentials; }

    /**
     * @brief Get a specific credential by index
     * @param index The index of the credential
     * @return Pointer to credential or nullptr if index is invalid
     */
    const Credential* getCredential(size_t index) const;

    /**
     * @brief Get the currently active credential
     * @return Pointer to active credential or nullptr if none is active
     */
    const Credential* getActiveCredential() const;

    /**
     * @brief Set the active credential by index
     * @param index The index to set as active
     * @return true if index was valid, false otherwise
     */
    bool setActiveCredential(size_t index);

    /**
     * @brief Get the active credential index
     * @return The active index or -1 if none is active
     */
    int getActiveCredentialIndex() const { return activeCredentialIndex; }

    /**
     * @brief Check if credentials list is empty
     * @return true if no credentials are stored
     */
    bool isEmpty() const { return credentials.empty(); }

    /**
     * @brief Get the number of stored credentials
     * @return Number of credentials
     */
    size_t getCredentialCount() const { return credentials.size(); }

private:
    static const char* const CREDENTIAL_FILE;
    std::vector<Credential> credentials;
    int activeCredentialIndex;

    /**
     * @brief Persist current credentials to file system
     * @return true if successful, false otherwise
     */
    bool persistCredentials() const;

    /**
     * @brief Find credential index by SSID
     * @param ssid The SSID to search for
     * @return Index of the credential or -1 if not found
     */
    int findCredentialIndex(const String& ssid) const;
};

#endif
