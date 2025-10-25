# EasyWiFi Modular Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                         USER CODE                               │
│                      #include <EasyWiFi.h>                      │
│                    EasyWiFi wifi; wifi.begin();                 │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             │ Public API (100% backward compatible)
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                      EasyWiFi (Main Class)                      │
│                    - Coordinates all modules                    │
│                    - Maintains public API                       │
│                    - begin(), loop(), reset()                   │
│                    - setAP(), setCSS(), etc.                    │
└─────┬─────────┬──────────────┬──────────────┬───────────────────┘
      │         │              │              │
      │         │              │              │
      ▼         ▼              ▼              ▼
┌─────────┐ ┌──────────┐ ┌──────────┐ ┌──────────────┐
│Credential│ │Connection│ │  Portal  │ │   Callback   │
│ Manager  │ │ Manager  │ │ Manager  │ │   Manager    │
└─────────┘ └──────────┘ └──────────┘ └──────────────┘
      │         │              │              │
      │         │              │              │
┌─────▼─────────▼──────────────▼──────────────▼───────────────────┐
│              ESP8266 Hardware / Arduino Core                    │
│        WiFi, LittleFS, WebServer, DNSServer                     │
└─────────────────────────────────────────────────────────────────┘
```

## Module Responsibilities

```
┌────────────────────────────────────────────────────────────────┐
│                     CredentialManager                          │
├────────────────────────────────────────────────────────────────┤
│ • Load credentials from LittleFS                               │
│ • Save/update credentials                                      │
│ • Manage multiple networks                                     │
│ • Track active credential                                      │
│ • File I/O operations                                          │
└────────────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────────────┐
│                     ConnectionManager                          │
├────────────────────────────────────────────────────────────────┤
│ • WiFi connection attempts                                     │
│ • Automatic reconnection                                       │
│ • Static IP configuration                                      │
│ • Connection state tracking                                    │
│ • Network cycling on failure                                   │
└────────────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────────────┐
│                       PortalManager                            │
├────────────────────────────────────────────────────────────────┤
│ • Start/stop Access Point                                      │
│ • Web server management                                        │
│ • DNS captive portal                                           │
│ • Network scanning                                             │
│ • HTML/CSS generation                                          │
│ • Handle credential submission                                 │
└────────────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────────────┐
│                      CallbackManager                           │
├────────────────────────────────────────────────────────────────┤
│ • Register user callbacks                                      │
│ • Notify: onConnect                                            │
│ • Notify: onDisconnect                                         │
│ • Notify: onSave                                               │
└────────────────────────────────────────────────────────────────┘
```

## Data Flow Example: Connecting to WiFi

```
┌──────────┐
│   User   │ calls wifi.begin()
└────┬─────┘
     │
     ▼
┌────────────────┐
│   EasyWiFi     │ Initializes LittleFS
└────┬───────────┘
     │
     ▼
┌────────────────────┐
│ CredentialManager  │ Loads saved credentials
└────┬───────────────┘
     │
     ▼
┌────────────────────┐
│  ConnectionManager │ Attempts connection
└────┬───────────────┘
     │
     ├─── Success ──────────┐
     │                      ▼
     │              ┌───────────────┐
     │              │CallbackManager│ Notifies onConnect
     │              └───────────────┘
     │
     └─── Failure ──────────┐
                            ▼
                    ┌──────────────┐
                    │PortalManager │ Starts captive portal
                    └──────────────┘
```

## State Transitions

```
                    ┌─────────┐
                    │  START  │
                    └────┬────┘
                         │
                         ▼
              ┌──────────────────┐
              │ Load Credentials │
              └────┬─────────────┘
                   │
         ┌─────────┴─────────┐
         │                   │
         ▼                   ▼
    Has Creds?          No Creds
         │                   │
         ▼                   ▼
   Try Connect         Start Portal
         │                   │
    ┌────┴────┐              │
    │         │              │
Success   Failure            │
    │         │              │
    ▼         └──────────────┘
Connected              │
    │                  ▼
    │           ┌────────────┐
    │           │Save Creds? │
    │           └──────┬─────┘
    │                  │
    │                  ▼
    │              Restart
    │                  │
    └──────────────────┘
             │
             ▼
       ┌──────────┐
       │Connected?│◄───── Loop checking
       └──────────┘
             │
             │ Lost
             ▼
       Reconnect Logic
```

## File Dependencies

```
EasyWiFi.h
    ├── includes CredentialManager.h
    ├── includes ConnectionManager.h
    ├── includes PortalManager.h
    └── includes CallbackManager.h

CredentialManager.h
    ├── Arduino.h
    └── LittleFS.h

ConnectionManager.h
    ├── Arduino.h
    ├── ESP8266WiFi.h
    └── CredentialManager.h (reference)

PortalManager.h
    ├── Arduino.h
    ├── ESP8266WiFi.h
    ├── ESP8266WebServer.h
    ├── DNSServer.h
    └── LittleFS.h

CallbackManager.h
    └── Arduino.h
```

## Comparison: Before vs After

### Before (Monolithic)
```
EasyWiFi.h (100 lines)
  └── All class definitions

EasyWiFi.cpp (580 lines)
  ├── Credential management
  ├── Connection logic
  ├── Portal management
  ├── Callback handling
  ├── HTML/CSS
  └── Everything else
```

### After (Modular)
```
CredentialManager.h/cpp (260 lines)
  └── Credential storage

ConnectionManager.h/cpp (280 lines)
  └── Connection logic

PortalManager.h/cpp (310 lines)
  └── Web portal

CallbackManager.h/cpp (120 lines)
  └── Event callbacks

EasyWiFi.h/cpp (250 lines)
  └── Coordination & API
```

## Benefits Visualization

```
┌─────────────────────────────────────────────────────────────┐
│                    DEVELOPER BENEFITS                       │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  Smaller Files         Easy to Navigate    Clear Structure │
│       ▼                       ▼                    ▼        │
│   ┌───────┐             ┌─────────┐          ┌─────────┐   │
│   │  150  │             │  Quick  │          │ Module  │   │
│   │ Lines │             │  Find   │          │  Name = │   │
│   │  Max  │             │  Code   │          │Purpose  │   │
│   └───────┘             └─────────┘          └─────────┘   │
│                                                             │
│  Parallel Work      Isolated Testing    Easy Debugging     │
│       ▼                    ▼                    ▼           │
│   ┌───────┐          ┌─────────┐          ┌─────────┐      │
│   │ No    │          │  Mock   │          │  One    │      │
│   │Merge  │          │ Modules │          │ Module  │      │
│   │Conflict│         │  Easy   │          │at Time  │      │
│   └───────┘          └─────────┘          └─────────┘      │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```
