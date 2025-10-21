# EasyWiFi
<p align="center">
    <img src="./docs/images/logo.png" alt="EasyWiFi Logo" width="250" border="0">
</p>
 Library for effortless WiFi management. It supports automatic connection with credential storage, captive portal configuration, and optional static IP—all with minimal code required.

(This library doesn't try to replace [tzapu/WiFiManager](https://github.com/tzapu/WiFiManager) but serves as a more simple, barebones and light library)



---
The EasyWiFi library depends on the following external libraries:

- Arduino.h — Core Arduino functions

- LittleFS.h — For file system storage and reading/writing WiFi credentials

- ESP8266WiFi.h — WiFi management on ESP8266 hardware

- ESP8266WebServer.h — Embedded web server for the captive portal

- DNSServer.h — DNS server to redirect to captive portal during setup.

--- 
### API Overview:
- begin() – Starts WiFi with credential storage and provisioning logic.

- loop() – Handles captive portal, reconnection, and HTTP server events.

- reset() – Removes stored credentials and clears configuration.

- saveCredentials(const char* ssid, const char* password) – Explicitly save new SSID and password.

- loadCredentials() – Loads credentials from the filesystem.

- printCredentials() – Prints credentials to serial for review.

- setAP(const char* name, const char* password) – Customizes the name and password of the captive AP.

- setCSS(const char* cssUrl) – Use a custom CSS file for the captive portal.

- setReconnectParams(int maxAttempts, unsigned long interval) – Configure reconnect behavior.

- setStaticIP(IPAddress ip, IPAddress gateway, IPAddress subnet, IPAddress dns) – Enable static IP mode

---
###  Provisioning Flow

<p align="center">
  <img src="./docs/images/easywifi_flow.svg" alt="EasyWiFi Provisioning Flow" width="700">
</p>

This diagram shows the automatic WiFi provisioning process used by EasyWiFi:
1. The device attempts to load saved credentials from **LittleFS**.
2. If found, it tries to connect to WiFi.
3. If not found or connection fails, a **captive portal** is launched.
4. The user connects to the AP, enters credentials, and they’re stored for future automatic reconnection.


---
Thanks to [Holorizu](https://github.com/Holorizu) for the logo.
