#include "PortalManager.h"

const char* defaultCSS = R"rawliteral(
      <style>
      body {
        background: #e0e0e0;
        font-family: Verdana, sans-serif;
        color: #111;
        margin: 0;
        padding: 20px;
      }
      .container {
        background: #fff;
        border: 2px solid #000;
        padding: 15px;
        max-width: 720px;
        margin: auto;
        box-shadow: 4px 4px 0 #000;
      }
      h1 {
        font-size: 22px;
        margin: 0 0;
        text-shadow: 1px 1px 0 #fff;
        text-align: center;
      }
      textarea {
        width: 100%;
        height: 100px;
        border: 2px inset #ccc;
        font-family: monospace;
        padding: 6px;
        box-sizing: border-box;
      }
      button {
        background: #c0c0c0;
        border: 2px outset #fff;
        padding: 6px 12px;
        font-weight: bold;
        cursor: pointer;
        margin-right: 6px;
      }
      button:active {
        border: 2px inset #fff;
        background: #a0a0a0;
      }
      .output {
        margin-top: 12px;
        padding: 10px;
        background: #f9f9f9;
        border: 2px inset #ccc;
        font-family: monospace;
        white-space: pre-wrap;
      }
      .encoded {
        color: #333;
        font-weight: bold;
      }
      /* Center form inputs */
      form {
        display: flex;
        flex-direction: column;
        align-items: center;
      }
      input[type=text],
      input[type=password] {
        width: 80%;
        padding: 8px;
        margin: 5px 0 15px;
        border: 1px solid #ccc;
        border-radius: 4px;
        text-align: center;
      }
      input[type=submit] {
        width: 50%;
        padding: 10px;
        margin-top: 10px;
        background-color: #808080;
        color: white;
        border: none;
        border-radius: 4px;
        cursor: pointer;
      }
      input[type=submit]:hover {
        background-color: #606060;
      }
      ul {
        list-style-type: none;
        padding: 0;
        max-width: 400px;
        margin: 10px auto;
      }
      li {
        background: #fff;
        padding: 8px;
        margin-bottom: 5px;
        border-radius: 4px;
        cursor: pointer;
        box-shadow: 0 1px 3px rgba(0, 0, 0, 0.1);
      }
      li:hover {
        background-color: #e9ecef;
      }
      h2 {
        font-size: 18px;
        margin: 20px 0 10px;
        text-align: center;
      }
      </style>
)rawliteral";

PortalManager::PortalManager()
    : server(nullptr),
      dnsServer(nullptr),
      portalActive(false),
      customCSS(nullptr),
      onSaveCredentialCallback(nullptr) {}

PortalManager::~PortalManager() {
    stop();
}

bool PortalManager::start(const char* apName, const char* apPassword) {
    Serial.println("PortalManager: Starting portal");
    
    // Initialize servers if not already done
    if (!server) {
        server = new ESP8266WebServer(80);
    }
    if (!dnsServer) {
        dnsServer = new DNSServer();
    }

    WiFi.mode(WIFI_AP);

    // Start access point
    if (apPassword && strlen(apPassword) >= 8 && strlen(apPassword) <= 63) {
        WiFi.softAP(apName, apPassword);
        Serial.printf("PortalManager: AP started: %s (secured)\n", apName);
    } else {
        WiFi.softAP(apName);
        Serial.printf("PortalManager: AP started: %s (open)\n", apName);
    }

    // Start DNS server for captive portal
    dnsServer->start(DNS_PORT, "*", WiFi.softAPIP());

    // Setup web server routes
    setupRoutes();
    server->begin();
    
    portalActive = true;

    Serial.printf("PortalManager: Portal active at http://%s/\n",
                 WiFi.softAPIP().toString().c_str());
    
    return true;
}

void PortalManager::stop() {
    if (!portalActive) {
        return;
    }

    Serial.println("PortalManager: Stopping portal");
    
    portalActive = false;
    
    if (dnsServer) {
        dnsServer->stop();
    }
    
    if (server) {
        server->stop();
    }
    
    WiFi.softAPdisconnect(true);
}

void PortalManager::handleClient() {
    if (!portalActive || !server || !dnsServer) {
        return;
    }
    
    dnsServer->processNextRequest();
    server->handleClient();
}

void PortalManager::setCustomCSS(const char* cssUrl) {
    customCSS = cssUrl;
}

void PortalManager::setOnSaveCredential(SaveCredentialCallback callback) {
    onSaveCredentialCallback = callback;
}

IPAddress PortalManager::getPortalIP() const {
    return WiFi.softAPIP();
}

void PortalManager::setupRoutes() {
    if (!server) return;

    // Root page
    server->on("/", [this]() { handleRoot(); });

    // Save credentials
    server->on("/save", HTTP_POST, [this]() { handleSave(); });

    // Network scan
    server->on("/scan", [this]() { handleScan(); });

    // Captive portal detection endpoints
    server->on("/generate_204", [this]() { handleCaptivePortalDetect(); });
    server->on("/fwlink", [this]() { handleCaptivePortalDetect(); });
    server->on("/hotspot-detect.html", [this]() { handleCaptivePortalDetect(); });
    server->on("/ncsi.txt", [this]() { handleCaptivePortalDetect(); });

    // Catch-all for other requests
    server->onNotFound([this]() { handleCaptivePortalDetect(); });
}

void PortalManager::handleRoot() {
    String cssBlock;

    if (customCSS && strlen(customCSS) > 0) {
        server->serveStatic("/styles.css", LittleFS, customCSS);
        cssBlock = "<link rel='stylesheet' href='" + String(customCSS) + "'>";
    } else {
        cssBlock = defaultCSS;
    }

    String html = buildPortalPage(cssBlock);
    server->send(200, "text/html", html);
}

void PortalManager::handleSave() {
    String newSsid = server->arg("ssid");
    String newPassword = server->arg("password");

    if (newSsid.length() > 0) {
        Serial.printf("PortalManager: Credentials received - SSID: %s\n", newSsid.c_str());
        
        // Trigger callback if set
        if (onSaveCredentialCallback) {
            onSaveCredentialCallback(newSsid, newPassword);
        }
        
        server->send(200, "text/html", "<h1>Credentials Saved. Rebooting...</h1>");
        delay(2000);
        ESP.restart();
    } else {
        server->send(400, "text/html", "<h1>SSID cannot be empty</h1>");
    }
}

void PortalManager::handleScan() {
    int networks = WiFi.scanNetworks();
    Serial.printf("PortalManager: Scan complete - found %d networks\n", networks);
    
    String json = "[";
    for (int i = 0; i < networks; i++) {
        if (i > 0) json += ",";
        json += "{";
        json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
        json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
        json += "\"encryption\":" + String(WiFi.encryptionType(i));
        json += "}";
    }
    json += "]";

    server->send(200, "application/json", json);
}

void PortalManager::handleCaptivePortalDetect() {
    server->sendHeader("Location", "/", true);
    server->send(302, "text/plain", "");
}

String PortalManager::buildPortalPage(const String& cssBlock) const {
    String html;
    html.reserve(1024);
    html += F("<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>EasyWiFi Setup</title>");
    html += cssBlock;
    html += F("</head><body><div class='container' style='text-align:center;'><h1>EasyWiFi Setup</h1></div>");
    html += F("<div class='container'><form method='POST' action='/save'>");
    html += F("SSID: <input type='text' id='ssid' name='ssid'><br>");
    html += F("Password: <input type='password' name='password'><br>");
    html += F("<input type='submit' value='Save'>");
    html += F("</form><h2>Available Networks</h2><button onclick=\"scan()\">Scan Networks</button><ul id='networks'></ul>");
    html += F("<script>function scan(){fetch('/scan').then(response=>response.json()).then(data=>{let list=document.getElementById('networks');list.innerHTML='';data.forEach(net=>{let item=document.createElement('li');item.textContent=net.ssid+' ('+net.rssi+'dBm)';item.style.cursor='pointer';item.onclick=()=>{document.getElementById('ssid').value=net.ssid;};list.appendChild(item);});});}</script></div></body></html>");
    return html;
}

const char* PortalManager::getDefaultCSS() const {
    return defaultCSS;
}
