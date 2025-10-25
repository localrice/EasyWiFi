# Contributing to EasyWiFi - Module Guide

This guide helps contributors understand which module to edit for different types of changes.

## Quick Reference

| I want to... | Edit this module |
|-------------|------------------|
| Add a new network credential format | `CredentialManager.*` |
| Change how WiFi connects | `ConnectionManager.*` |
| Modify the web portal UI | `PortalManager.*` |
| Add a new callback event | `CallbackManager.*` |
| Change the public API | `EasyWiFi.*` |
| Fix a bug in file saving | `CredentialManager.*` |
| Improve reconnection logic | `ConnectionManager.*` |
| Add new web endpoints | `PortalManager.*` |
| Change HTML/CSS styling | `PortalManager.*` |

## Module Deep Dive

### CredentialManager

**Edit when:**
- Adding support for new storage backends (EEPROM, SD card)
- Changing credential file format
- Adding credential encryption
- Implementing credential prioritization
- Adding network blacklisting
- Changing how credentials are validated

**Files:** `src/CredentialManager.h`, `src/CredentialManager.cpp`

**Common tasks:**

#### Adding a new credential property
```cpp
// In CredentialManager.h
struct Credential {
    String ssid;
    String password;
    uint8_t priority;  // NEW PROPERTY
};

// In CredentialManager.cpp - update persistCredentials()
file.println(cred.ssid + "\t" + 
             cred.password + "\t" + 
             String(cred.priority));
```

#### Adding credential validation
```cpp
// In CredentialManager.cpp
bool CredentialManager::saveCredential(const String& ssid, const String& password) {
    // Add validation
    if (ssid.length() > 32) {
        Serial.println("SSID too long");
        return false;
    }
    // ... rest of implementation
}
```

---

### ConnectionManager

**Edit when:**
- Changing connection timeout values
- Implementing new reconnection strategies
- Adding support for WPA3 or other security protocols
- Implementing connection quality monitoring
- Adding bandwidth testing
- Changing how static IP is configured

**Files:** `src/ConnectionManager.h`, `src/ConnectionManager.cpp`

**Common tasks:**

#### Adding connection timeout configuration
```cpp
// In ConnectionManager.h
class ConnectionManager {
    // ...
    void setConnectionTimeout(unsigned long timeout);
private:
    unsigned long connectionTimeout;
};

// In ConnectionManager.cpp
void ConnectionManager::setConnectionTimeout(unsigned long timeout) {
    connectionTimeout = timeout;
    Serial.printf("Connection timeout set to %lu ms\n", timeout);
}
```

#### Implementing smart reconnection
```cpp
// In ConnectionManager.cpp
bool ConnectionManager::handleReconnection() {
    // Add exponential backoff
    static unsigned long backoffMultiplier = 1;
    
    const unsigned long dynamicInterval = reconnectInterval * backoffMultiplier;
    
    if (now - lastReconnectAttempt < dynamicInterval) {
        return false;
    }
    
    backoffMultiplier = min(backoffMultiplier * 2, 16); // Cap at 16x
    // ... rest of reconnection logic
}
```

---

### PortalManager

**Edit when:**
- Changing the captive portal web interface
- Adding new web endpoints
- Modifying HTML/CSS styling
- Adding network filtering/sorting
- Implementing AJAX updates
- Adding password strength indicators
- Implementing QR code scanning

**Files:** `src/PortalManager.h`, `src/PortalManager.cpp`

**Common tasks:**

#### Adding a new web endpoint
```cpp
// In PortalManager.cpp - setupRoutes()
void PortalManager::setupRoutes() {
    // ... existing routes
    
    // NEW ENDPOINT
    server->on("/status", [this]() { handleStatus(); });
}

void PortalManager::handleStatus() {
    String json = "{";
    json += "\"connected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false");
    json += ",\"ip\":\"" + WiFi.localIP().toString() + "\"";
    json += "}";
    server->send(200, "application/json", json);
}
```

#### Customizing the HTML page
```cpp
// In PortalManager.cpp - buildPortalPage()
String PortalManager::buildPortalPage(const String& cssBlock) const {
    String html;
    html.reserve(1024);
    html += F("<!DOCTYPE html><html>");
    html += F("<head>");
    html += F("<meta charset='UTF-8'>");
    html += F("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
    html += F("<title>EasyWiFi Setup</title>");
    html += cssBlock;
    html += F("</head>");
    // ... add your custom HTML
    return html;
}
```

---

### CallbackManager

**Edit when:**
- Adding new event types
- Implementing event filtering
- Adding callback priorities
- Implementing async callbacks
- Adding event history/logging

**Files:** `src/CallbackManager.h`, `src/CallbackManager.cpp`

**Common tasks:**

#### Adding a new callback type
```cpp
// In CallbackManager.h
class CallbackManager {
public:
    using PortalStartedCallback = std::function<void()>;  // NEW
    
    void setOnPortalStarted(PortalStartedCallback callback);
    void notifyPortalStarted();
    
private:
    PortalStartedCallback onPortalStartedCallback;
};

// In CallbackManager.cpp
void CallbackManager::setOnPortalStarted(PortalStartedCallback callback) {
    onPortalStartedCallback = callback;
    Serial.println("CallbackManager: Portal started callback registered");
}

void CallbackManager::notifyPortalStarted() {
    if (onPortalStartedCallback) {
        Serial.println("CallbackManager: Notifying portal started");
        onPortalStartedCallback();
    }
}
```

---

### EasyWiFi (Main Class)

**Edit when:**
- Adding new public API methods
- Changing initialization sequence
- Modifying module coordination
- Adding backward compatibility wrappers
- Changing loop() behavior

**Files:** `src/EasyWiFi.h`, `src/EasyWiFi.cpp`

**Common tasks:**

#### Adding a new public method
```cpp
// In EasyWiFi.h
class EasyWiFi {
public:
    // ... existing methods
    void forgetNetwork(const char* ssid);  // NEW
};

// In EasyWiFi.cpp
void EasyWiFi::forgetNetwork(const char* ssid) {
    Serial.printf("EasyWiFi: Forgetting network %s\n", ssid);
    
    // Use existing module functionality
    int index = credentialManager.findCredentialIndex(String(ssid));
    if (index >= 0) {
        credentialManager.removeCredential(index);
    }
}
```

## Testing Your Changes

### Testing a Single Module

Create a minimal test sketch:

```cpp
#include <Arduino.h>
#include <LittleFS.h>
#include "CredentialManager.h"  // Test specific module

CredentialManager credManager;

void setup() {
    Serial.begin(115200);
    LittleFS.begin();
    
    // Test your changes
    credManager.saveCredential("TestNetwork", "password123");
    credManager.printCredentials();
}

void loop() {}
```

### Testing Integration

Use the existing example to verify everything works together:

```bash
# Compile the BasicConnection example
pio run -e esp12e

# Upload to device
pio run -e esp12e -t upload

# Monitor serial output
pio device monitor
```

## Pull Request Checklist

- [ ] Changed only the relevant module(s)
- [ ] Added Doxygen comments to new public methods
- [ ] Tested with existing examples
- [ ] Verified backward compatibility
- [ ] Updated ARCHITECTURE.md if adding new responsibilities
- [ ] No compiler warnings
- [ ] Code follows existing style

## Common Patterns

### Module Communication

**DON'T** create direct dependencies:
```cpp
// ❌ BAD - ConnectionManager directly using PortalManager
class ConnectionManager {
    PortalManager portalManager;  // NO!
};
```

**DO** communicate through main class:
```cpp
// ✅ GOOD - EasyWiFi coordinates modules
void EasyWiFi::tryConnect(bool preferNext) {
    if (!connectionManager.connect(preferNext)) {
        portalManager.start(apName, apPassword);
    }
}
```

### Error Handling

**DO** return status and log errors:
```cpp
bool CredentialManager::saveCredential(const String& ssid, const String& password) {
    if (ssid.length() == 0) {
        Serial.println("CredentialManager: Cannot save empty SSID");
        return false;  // Return status
    }
    // ... implementation
    return true;
}
```

### Memory Management

**DO** use `String.reserve()` for large strings:
```cpp
String PortalManager::buildPortalPage(const String& cssBlock) const {
    String html;
    html.reserve(1024);  // Pre-allocate
    html += F("<!DOCTYPE html>");  // Use F() for constants
    // ... build page
    return html;
}
```

## Getting Help

- Check existing modules for similar functionality
- Read the ARCHITECTURE.md for module responsibilities
- Look at git history to see how similar changes were made
- Open an issue for design questions before implementing

## Code Style

Follow the existing style:
- Class names: `PascalCase`
- Methods: `camelCase`
- Private members: `camelCase`
- Constants: `UPPER_SNAKE_CASE`
- Indentation: 4 spaces
- Braces: K&R style (opening brace on same line)

## Example: Adding a Feature End-to-End

Let's say you want to add "forget network" functionality:

### 1. CredentialManager (storage layer)
```cpp
// CredentialManager.h
bool removeCredential(int index);

// CredentialManager.cpp
bool CredentialManager::removeCredential(int index) {
    if (index < 0 || index >= credentials.size()) {
        return false;
    }
    credentials.erase(credentials.begin() + index);
    return persistCredentials();
}
```

### 2. EasyWiFi (public API)
```cpp
// EasyWiFi.h
void forgetNetwork(const char* ssid);

// EasyWiFi.cpp
void EasyWiFi::forgetNetwork(const char* ssid) {
    int idx = credentialManager.findCredentialIndex(String(ssid));
    if (idx >= 0) {
        credentialManager.removeCredential(idx);
    }
}
```

### 3. PortalManager (web interface - optional)
```cpp
// Add endpoint in setupRoutes()
server->on("/forget", HTTP_POST, [this]() {
    String ssid = server->arg("ssid");
    // Trigger callback to main class
    if (onForgetNetworkCallback) {
        onForgetNetworkCallback(ssid);
    }
});
```

That's it! Each module handles its own responsibility.
