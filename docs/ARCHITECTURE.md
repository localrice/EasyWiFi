# EasyWiFi - Modular Architecture

## Overview

The EasyWiFi library has been refactored into a modular architecture to improve maintainability, collaboration, and code organization. The library is now split into the following components:

## Module Structure

```
src/
├── EasyWiFi.h              # Main class (backward compatible API)
├── EasyWiFi.cpp            # Main class implementation
├── CredentialManager.h     # Credential storage and persistence
├── CredentialManager.cpp
├── ConnectionManager.h     # WiFi connection and reconnection logic
├── ConnectionManager.cpp
├── PortalManager.h         # Captive portal and web interface
├── PortalManager.cpp
├── CallbackManager.h       # Event callback management
└── CallbackManager.cpp
```

## Modules

### 1. CredentialManager
**Purpose:** Manages WiFi credential storage and persistence

**Responsibilities:**
- Load credentials from LittleFS
- Save new credentials
- Manage multiple network credentials
- Handle credential file I/O
- Track active credential
- Support legacy credential format

**Key Methods:**
- `loadCredentials()` - Load from file system
- `saveCredential(ssid, password)` - Save/update credential
- `getCredentials()` - Get all stored credentials
- `getActiveCredential()` - Get currently active credential
- `clearCredentials()` - Remove all credentials

### 2. ConnectionManager
**Purpose:** Handles WiFi connections and reconnection logic

**Responsibilities:**
- Attempt WiFi connections
- Handle automatic reconnection
- Manage static IP configuration
- Track connection state
- Cycle through saved networks on failure

**Key Methods:**
- `connect(preferNext)` - Attempt connection to saved networks
- `attemptConnection(cred, timeout)` - Try specific credential
- `handleReconnection()` - Manage reconnection attempts
- `setStaticIP()` - Configure static IP
- `setReconnectParams()` - Configure retry behavior

### 3. PortalManager
**Purpose:** Manages the captive portal web interface

**Responsibilities:**
- Start/stop access point
- Run web server
- Handle DNS captive portal detection
- Serve configuration page
- Handle network scanning
- Process credential submissions

**Key Methods:**
- `start(apName, apPassword)` - Start captive portal
- `stop()` - Stop portal
- `handleClient()` - Process web requests
- `setCustomCSS()` - Set custom styling
- `setOnSaveCredential()` - Register save callback

### 4. CallbackManager
**Purpose:** Manages event callbacks

**Responsibilities:**
- Register user callbacks
- Notify on connection events
- Notify on disconnection events
- Notify on credential save events

**Key Methods:**
- `setOnConnect(callback)` - Register connect callback
- `setOnDisconnect(callback)` - Register disconnect callback
- `setOnSave(callback)` - Register save callback
- `notifyConnect()` - Trigger connect event
- `notifyDisconnect()` - Trigger disconnect event
- `notifySave()` - Trigger save event

### 5. EasyWiFi (Main Class)
**Purpose:** Coordinate between modules and provide backward-compatible API

**Responsibilities:**
- Initialize all modules
- Coordinate module interactions
- Maintain backward compatibility
- Provide simplified public API

## Benefits of Modularization

### 1. **Improved Maintainability**
- Each module has a single, clear responsibility
- Easier to understand and debug
- Smaller, more focused code files

### 2. **Better Collaboration**
- Multiple developers can work on different modules simultaneously
- Reduced merge conflicts
- Clear module boundaries

### 3. **Easier Testing**
- Modules can be tested independently
- Mock implementations can be created for each module
- Unit tests are more focused

### 4. **Enhanced Extensibility**
- New features can be added to specific modules
- Alternative implementations can be swapped in
- Future features won't bloat a single file

### 5. **Backward Compatibility**
- Existing code using EasyWiFi continues to work
- Public API remains unchanged
- `#include <EasyWiFi.h>` still works as before

## Usage

The public API remains **100% backward compatible**. Existing code requires no changes:

```cpp
#include <EasyWiFi.h>

EasyWiFi wifi;

void setup() {
    Serial.begin(115200);
    
    wifi.setAP("MyESP8266_AP", "password");
    wifi.setReconnectParams(5, 3000);
    wifi.begin();
}

void loop() {
    wifi.loop();
}
```

## Development Guidelines

### Working on a Specific Feature

1. **Credential Management** → Edit `CredentialManager.*`
2. **Connection Logic** → Edit `ConnectionManager.*`
3. **Web Portal** → Edit `PortalManager.*`
4. **Callbacks** → Edit `CallbackManager.*`
5. **API Changes** → Edit `EasyWiFi.*`

### Adding a New Feature

1. Identify which module owns the feature
2. Add methods to that module
3. Update `EasyWiFi` class if public API needs to change
4. Maintain backward compatibility when possible

### Module Communication

- Modules communicate through the main `EasyWiFi` class
- Avoid direct module-to-module dependencies
- Use callbacks for event notification
- Pass references/pointers when modules need to collaborate

## File Descriptions

| File | Lines | Purpose |
|------|-------|---------|
| `CredentialManager.h` | ~120 | Credential management interface |
| `CredentialManager.cpp` | ~140 | Credential implementation |
| `ConnectionManager.h` | ~130 | Connection management interface |
| `ConnectionManager.cpp` | ~150 | Connection implementation |
| `PortalManager.h` | ~90 | Portal management interface |
| `PortalManager.cpp` | ~220 | Portal implementation (includes HTML/CSS) |
| `CallbackManager.h` | ~80 | Callback management interface |
| `CallbackManager.cpp` | ~40 | Callback implementation |
| `EasyWiFi.h` | ~120 | Main public API |
| `EasyWiFi.cpp` | ~130 | Main coordination logic |

**Total:** ~1,220 lines (previously ~680 lines in 2 files)

The increased line count comes from:
- Better documentation (Doxygen comments)
- Separation of concerns
- Improved error handling
- More modular structure

## Migration Notes

### For Library Users
**No changes required!** Your existing code will work as-is.

### For Library Contributors

**Before (monolithic):**
```cpp
// Everything in EasyWiFi.cpp
void EasyWiFi::saveCredentials(const char* ssid, const char* pass) {
    // 50 lines of implementation
}
```

**After (modular):**
```cpp
// In EasyWiFi.cpp (coordination)
void EasyWiFi::saveCredentials(const char* ssid, const char* pass) {
    if (credentialManager.saveCredential(ssid, pass)) {
        callbackManager.notifySave(ssid, pass);
    }
}

// In CredentialManager.cpp (implementation)
bool CredentialManager::saveCredential(const String& ssid, const String& pass) {
    // Implementation details here
}
```

## Future Improvements

Potential enhancements enabled by this architecture:

1. **Pluggable Storage** - Swap LittleFS for SPIFFS or EEPROM
2. **Advanced Portal** - Add more features without bloating main class
3. **Connection Strategies** - Different reconnection algorithms
4. **Multiple Portal Themes** - Easy to add theme support
5. **Testing Framework** - Mock modules for comprehensive testing
6. **Alternative Transports** - Support WiFi alternatives (Ethernet, etc.)

## Build Size Impact

The modular architecture has **minimal impact** on compiled binary size:
- Unused code is still eliminated by the linker
- Header-only code is inlined where appropriate
- Module overhead is negligible (<100 bytes)

## Conclusion

This refactoring transforms EasyWiFi from a monolithic library into a well-organized, modular codebase that's easier to:
- Understand
- Maintain
- Test
- Extend
- Collaborate on

All while maintaining 100% backward compatibility with existing user code.
