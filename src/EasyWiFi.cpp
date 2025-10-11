#include "EasyWiFi.h"
#include <algorithm>

ESP8266WebServer server(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;

const char* const EasyWiFi::CREDENTIAL_FILE = "/wifi_credentials.txt";

const char *defaultCSS = R"rawliteral(
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

EasyWiFi::EasyWiFi() {}
/*
  EasyWiFi Logic Flow:

  1. begin()
     - Mounts LittleFS.
     - Loads saved WiFi credentials.
     - Attempts to connect to WiFi.
     - If no credentials or connection fails, starts the configuration portal.

  2. loop()
     - If the configuration portal is active, handles incoming web requests.

  3. reset()
     - Removes saved WiFi credentials from LittleFS.
     - Clears ssid and password variables.

  4. saveCredentials()
     - Saves the provided SSID and password to LittleFS.

  5. loadCredentials()
     - Loads SSID and password from LittleFS.

  6. printCredentials()
     - Prints the current SSID and password to Serial.
*/
void EasyWiFi::begin() {
  Serial.println("EasyWiFi: begin()");
  if (!LittleFS.begin()) {
    Serial.println("EasyWiFi: Failed to mount LittleFS");
    return;
  }
  loadCredentials();
  wasConnected = WiFi.status() == WL_CONNECTED;
  tryConnect();
}

void EasyWiFi::loop() {
  if (portalActive) {
    dnsServer.processNextRequest();
    handleClient();
    return;
  }

  const wl_status_t status = WiFi.status();
  if (status == WL_CONNECTED) {
    if (!wasConnected) {
      Serial.println("EasyWiFi: Reconnected to WiFi");
      wasConnected = true;
      notifyConnect();
    }
    reconnectAttempts = 0;
    return;
  }

  if (wasConnected) {
    Serial.println("EasyWiFi: WiFi lost, attempting reconnect...");
    notifyDisconnect();
    wasConnected = false;
  }

  const unsigned long now = millis();
  if (now - lastReconnectAttempt < reconnectInterval) {
    return;
  }

  lastReconnectAttempt = now;
  ++reconnectAttempts;

  if (ssid.length() > 0) {
    Serial.printf("EasyWiFi: Reconnect attempt %d to SSID: %s\n", reconnectAttempts, ssid.c_str());
    ensureStationMode();
    WiFi.begin(ssid.c_str(), password.c_str());
  }

  if (reconnectAttempts > maxReconnectAttempts) {
    Serial.println("EasyWiFi: Too many failures, cycling saved networks");
    reconnectAttempts = 0;
    tryConnect(true);
  }
}

void EasyWiFi::reset() {
  Serial.println("EasyWiFi: reset()");
  LittleFS.remove(CREDENTIAL_FILE);
  ssid = "";
  password = "";
  credentials.clear();
  activeCredentialIndex = -1;
}

void EasyWiFi::tryConnect(bool preferNext) {
  stopPortal();

  reconnectAttempts = 0;
  lastReconnectAttempt = millis();

  if (credentials.empty()) {
    Serial.println("No SSID saved, starting portal");
    startPortal();
    return;
  }

  ensureStationMode();

  if (useStaticIP) {
    if (!WiFi.config(staticIP, staticGateway, staticSubnet, staticDNS)) {
      Serial.println("EasyWiFi: Failed to configure static IP, falling back to DHCP");
    } else {
      Serial.printf("EasyWiFi: Using static IP %s\n", staticIP.toString().c_str());
    }
  }

  const size_t totalNetworks = credentials.size();
  size_t startIndex = 0;
  if (activeCredentialIndex >= 0 && static_cast<size_t>(activeCredentialIndex) < totalNetworks) {
    startIndex = static_cast<size_t>(activeCredentialIndex);
    if (preferNext && totalNetworks > 1) {
      startIndex = (startIndex + 1) % totalNetworks;
    }
  }

  const unsigned long timeout = 10000; 
  for (size_t offset = 0; offset < totalNetworks; ++offset) {
    const size_t index = (startIndex + offset) % totalNetworks;
    applyActiveCredential(index);
    if (attemptConnection(credentials[index], timeout)) {
      return;
    }
  }

  Serial.println("Failed to connect to any saved network, starting portal");
  ssid = "";
  password = "";
  activeCredentialIndex = -1;
  wasConnected = false;
  WiFi.disconnect(false);
  startPortal();
}
void EasyWiFi::startPortal() {
  Serial.println("EasyWiFi: startPortal()");
  WiFi.mode(WIFI_AP);

  if (strlen(APPassword) >= 8 && strlen(APPassword) <= 63) {
    WiFi.softAP(APName, APPassword);
    Serial.printf("AP started: %s (secured) \n",APName);
  } else {
    WiFi.softAP(APName);     
    Serial.printf("AP started: %s (open) \n",APName);
  }

  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  String cssBlock;

  if (userCSS && strlen(userCSS) > 0) {
    server.serveStatic("/styles.css", LittleFS, userCSS);
    cssBlock = "<link rel='stylesheet' href='" + String(userCSS) + "'>";
  } else {
    cssBlock = defaultCSS;
  }

  server.on("/", [this, cssBlock]() {
    String html = buildPortalPage(cssBlock);
    server.send(200, "text/html", html);
  });

  
  server.on("/save", HTTP_POST, [this]() {
    String newSsid = server.arg("ssid");
    String newPassword = server.arg("password");

    if (newSsid.length() > 0) {
      saveCredentials(newSsid.c_str(), newPassword.c_str());
      server.send(200, "text/html", "<h1>Credentials Saved. Rebooting...</h1>");
      delay(2000);
      ESP.restart();
    } else {
      server.send(400, "text/html", "<h1>SSID cannot be empty</h1>");
    }
  });
  
  server.on("/scan", [this]() {
    int networks = WiFi.scanNetworks(); 
    Serial.printf("Scan complete: %d", networks);
    String json = "[";

    for (int i = 0; i < networks; i++) {
      if (i > 0) json += ",";
      json += "{";
      json += "\"ssid\":\"" + WiFi.SSID(i) + "\","; // SSID
      json += "\"rssi\":" + String(WiFi.RSSI(i)) + ","; // Signal strength
      json += "\"encryption\":" + String(WiFi.encryptionType(i)); // Encryption type
      json += "}";
    }
    json += "]";

    server.send(200, "application/json", json);
  });

  server.on("/generate_204", []() {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });
  server.on("/fwlink", []() {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });
  server.on("/hotspot-detect.html", []() {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });
  server.on("/ncsi.txt", []() {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });

  server.onNotFound([]() {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });

  server.begin();
  portalActive = true;

  Serial.printf("Portal active. Connect to WiFi 'EasyWiFi_Setup' and visit http://%s/\n",
                WiFi.softAPIP().toString().c_str());
}

void EasyWiFi::stopPortal() {
  if (!portalActive) {
    return;
  }

  portalActive = false;
  dnsServer.stop();
  server.stop();
  WiFi.softAPdisconnect(true);
}

void EasyWiFi::handleClient() {
    server.handleClient();
}

void EasyWiFi::saveCredentials(const char* networkSsid, const char* networkPassword) {
  Serial.println("EasyWiFi: saveCredentials()");
  String newSsid = String(networkSsid);
  String newPassword = String(networkPassword);

  size_t updatedIndex = 0;
  bool credentialsChanged = true;
  bool reordered = false;
  auto it = std::find_if(credentials.begin(), credentials.end(), [&](const Credential& cred) {
    return cred.ssid == newSsid;
  });

  if (it != credentials.end()) {
    updatedIndex = static_cast<size_t>(it - credentials.begin());
    credentialsChanged = it->password != newPassword;
    it->password = newPassword;
    reordered = updatedIndex != 0;
    if (reordered) {
      std::rotate(credentials.begin(), credentials.begin() + updatedIndex, credentials.begin() + updatedIndex + 1);
      updatedIndex = 0;
    }
    applyActiveCredential(updatedIndex);
  } else {
    credentials.insert(credentials.begin(), {newSsid, newPassword});
    updatedIndex = 0;
    applyActiveCredential(updatedIndex);
  }

  if ((credentialsChanged || reordered) && !persistCredentials()) {
    return;
  }
  Serial.printf("Saved SSID: %s \n", newSsid.c_str());
  if (onSaveCallback) {
    onSaveCallback(newSsid, newPassword);
  }
}

void EasyWiFi::loadCredentials() {
  Serial.println("EasyWiFi: loadCredentials()");
  credentials.clear();
  activeCredentialIndex = -1;
  ssid = "";
  password = "";

  File file = LittleFS.open(CREDENTIAL_FILE, "r");
  if (!file) {
    Serial.println("EasyWiFi: No saved credentials found");
    return;
  }

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) {
      continue;
    }

    int separatorIndex = line.indexOf('\t');

    if (separatorIndex == -1) {
      String legacyPassword = file.readStringUntil('\n');
      legacyPassword.trim();
      if (line.length() > 0) {
        credentials.push_back({line, legacyPassword});
      }
      break; 
    }

    credentials.push_back({line.substring(0, separatorIndex), line.substring(separatorIndex + 1)});
  }

  file.close();

  if (!credentials.empty()) {
    applyActiveCredential(0);
    Serial.printf("Loaded %u saved network(s). First SSID: %s \n", static_cast<unsigned int>(credentials.size()), ssid.c_str());
  } else {
    Serial.println("EasyWiFi: No valid credentials found in file");
  }
}

void EasyWiFi::printCredentials() {
  Serial.println("EasyWiFi: printCredentials()");
  if (credentials.empty()) {
    Serial.println("No credentials stored");
    return;
  }

  for (size_t i = 0; i < credentials.size(); ++i) {
    const auto &cred = credentials[i];
    Serial.printf("[%u] SSID: %s\n", static_cast<unsigned>(i), cred.ssid.c_str());
    Serial.printf("    Password: %s\n", cred.password.c_str());
  }
}

void EasyWiFi::setAP(const char* name, const char* password) {
  APName = name;
  APPassword = password;
}

void EasyWiFi::setCSS(const char* cssUrl) {
  userCSS = cssUrl;
}

void EasyWiFi::setReconnectParams(int maxAttempts, unsigned long interval) {
  maxReconnectAttempts = maxAttempts;
  reconnectInterval = interval;
}

void EasyWiFi::setStaticIP(IPAddress ip, IPAddress gateway, IPAddress subnet, IPAddress dns) {
  staticIP = ip;
  staticGateway = gateway;
  staticSubnet = subnet;
  staticDNS = dns;
  useStaticIP = true;
}

void EasyWiFi::setOnConnect(ConnectCallback cb) {
  onConnectCallback = cb;
}

void EasyWiFi::setOnDisconnect(DisconnectCallback cb) {
  onDisconnectCallback = cb;
}

void EasyWiFi::setOnSave(SaveCallback cb) {
  onSaveCallback = cb;
}

void EasyWiFi::notifyConnect() {
  if (onConnectCallback && WiFi.status() == WL_CONNECTED) {
    onConnectCallback(ssid, WiFi.localIP());
  }
}

void EasyWiFi::notifyDisconnect() {
  if (onDisconnectCallback) {
    onDisconnectCallback(ssid);
  }
}

void EasyWiFi::applyActiveCredential(size_t index) {
  if (index >= credentials.size()) {
    activeCredentialIndex = -1;
    ssid = "";
    password = "";
    return;
  }

  activeCredentialIndex = static_cast<int>(index);
  ssid = credentials[index].ssid;
  password = credentials[index].password;
}

bool EasyWiFi::attemptConnection(const Credential& cred, unsigned long timeout) {
  Serial.printf("Attempting to connect to SSID: %s\n", cred.ssid.c_str());
  ensureStationMode();
  WiFi.begin(cred.ssid.c_str(), cred.password.c_str());

  const wl_status_t result = static_cast<wl_status_t>(WiFi.waitForConnectResult(timeout));

  if (result == WL_CONNECTED) {
    Serial.printf("Connected! IP address: %s\n", WiFi.localIP().toString().c_str());
    stopPortal();
    wasConnected = true;
    reconnectAttempts = 0;
    notifyConnect();
    return true;
  }

  Serial.println("Failed to connect");
  WiFi.disconnect(false);
  return false;
}

bool EasyWiFi::persistCredentials() const {
  File file = LittleFS.open(CREDENTIAL_FILE, "w");
  if (!file) {
    Serial.println("EasyWiFi: Failed to open file for writing");
    return false;
  }

  for (const auto &cred : credentials) {
    file.println(cred.ssid + "\t" + cred.password);
  }

  file.close();
  return true;
}

String EasyWiFi::buildPortalPage(const String& cssBlock) const {
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

void EasyWiFi::ensureStationMode() {
  const WiFiMode_t mode = WiFi.getMode();
  if (mode == WIFI_STA) {
    return;
  }

  if (mode == WIFI_AP || mode == WIFI_AP_STA) {
    WiFi.softAPdisconnect(true);
  }

  WiFi.mode(WIFI_STA);
}
