#include "EasyWiFi.h"

ESP8266WebServer server(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;

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
  tryConnect();
}

void EasyWiFi::loop() {
  static bool wasConnected = false; //track previous state

  if (portalActive) {
    dnsServer.processNextRequest();
    handleClient();
  } else {
    if (WiFi.status() != WL_CONNECTED) {
      if (wasConnected) {
        Serial.println("EasyWiFi: WiFi lost, attempting reconnect...");
        wasConnected = false;
      }

      unsigned long now = millis();

      if (now - lastReconnectAttempt > reconnectInterval) {
        lastReconnectAttempt = now;
        reconnectAttempts++;

        Serial.println("EasyWiFi: WiFi lost, attempting reconnect...");
        WiFi.begin(ssid.c_str(), password.c_str());

        if (reconnectAttempts > maxReconnectAttempts) {
          Serial.println("EasyWiFi: Too many failures, starting portal");
          startPortal();
          reconnectAttempts = 0;
        }
      }
    } else {
        if (!wasConnected) {
          Serial.println("EasyWiFi: Reconnected to WiFi");
          wasConnected = true; // only print once when it reconnects
        }
        reconnectAttempts = 0; // reset if wifi is connected
    }
  }
}

void EasyWiFi::reset() {
  Serial.println("EasyWiFi: reset()");
  LittleFS.remove("/wifi_credentials.txt");
  ssid = "";
  password = "";
}

void EasyWiFi::tryConnect() {
  if (ssid.length() == 0) {
    Serial.println("No SSID saved, starting portal");
    startPortal();
    return;
  }

  if (useStaticIP) {
    if (!WiFi.config(staticIP, staticGateway, staticSubnet, staticDNS)) {
        Serial.println("EasyWiFi: Failed to configure static IP, falling back to DHCP");
    } else {
        Serial.printf("EasyWiFi: Using static IP %s\n", staticIP.toString().c_str());
    }
  }

  Serial.printf("Attempting to connect to SSID: %s\n", ssid.c_str());
  WiFi.begin(ssid.c_str(), password.c_str());
  unsigned long startAttemptTime = millis();
  const unsigned long timeout = 10000; // 10 seconds timeout

  while ((WiFi.status() != WL_CONNECTED) && (millis() - startAttemptTime < timeout)) {
    delay(100);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nConnected! IP address: %s\n", WiFi.localIP().toString().c_str());
    portalActive = false;
  } else {
    Serial.println("\nFailed to connect, starting portal");
    startPortal();
  }
}
void EasyWiFi::startPortal() {
  Serial.println("EasyWiFi: startPortal()");
  WiFi.mode(WIFI_AP);

  // WPA2 requires passwords between 8 and 63 characters
  if (strlen(APPassword) >= 8 && strlen(APPassword) <= 63) {
    WiFi.softAP(APName, APPassword);
    Serial.printf("AP started: %s (secured) \n",APName);
  } else {
    WiFi.softAP(APName);     // if the password is invalid, start an open AP
    Serial.printf("AP started: %s (open) \n",APName);
  }

  // start DNS server: redirect all domains to our ESP's AP IP
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  String cssBlock;

  if (userCSS && strlen(userCSS) > 0) {
    // register the route to serve the user-provided CSS from LittleFS
    server.serveStatic("/styles.css", LittleFS, userCSS);
    // link the user-provided css
    cssBlock = "<link rel='stylesheet' href='" + String(userCSS) + "'>";
  } else {
      cssBlock = defaultCSS;
  }

  server.on("/", [this, cssBlock]() {
  String html = "<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>EasyWiFi Setup</title>"
            + cssBlock +
            "</head><body><div class='container' style='text-align:center;'>"
            + "<h1>EasyWiFi Setup</h1> </div>"
            + "<div class='container'><form method='POST' action='/save'>"
            + "SSID: <input type='text' id='ssid' name='ssid'><br>"
            + "Password: <input type='password' name='password'><br>"
            + "<input type='submit' value='Save'>"
            + "</form>"
            + "<h2>Available Networks</h2>"
            + "<button onclick=\"scan()\">Scan Networks</button>"
            + "<ul id=\"networks\"></ul>"
            + "<script>"
            + "function scan() {"
            + "fetch('/scan')"
            + ".then(response => response.json())"
            + ".then(data => {"
            + "let list = document.getElementById('networks');"
            + "list.innerHTML = '';"
            + "data.forEach(net => {"
            + "let item = document.createElement('li');"
            + "item.textContent = net.ssid + ' (' + net.rssi + 'dBm)';"
            + "item.style.cursor = 'pointer';"
            + "item.onclick = () => {"
            + "document.getElementById('ssid').value = net.ssid;"
            + "};"
            + "list.appendChild(item);"
            + "});"
            + "});"
            + "}"
            + "</script>"
            + "</div></body></html>";
  server.send(200, "text/html", html);
});

  
  // Handle form submission
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
  
  // Handle scanning for networks
  server.on("/scan", [this]() {
    int networks = WiFi.scanNetworks(); // Returns the number of networks found
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

  // Captive portal detection endpoints → redirect to "/"
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

  // Catch-all → redirect to "/"
  server.onNotFound([]() {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });

  server.begin();
  portalActive = true;

  Serial.printf("Portal active. Connect to WiFi 'EasyWiFi_Setup' and visit http://%s/\n",
                WiFi.softAPIP().toString().c_str()); 
}

void EasyWiFi::handleClient() {
    // placeholder — will handle web requests later
    server.handleClient();
}

void EasyWiFi::saveCredentials(const char* ssid, const char* password) {
  Serial.println("EasyWiFi: saveCredentials()");
  File file = LittleFS.open("/wifi_credentials.txt", "w");
  if (!file) {
    Serial.println("EasyWiFi: Failed to open file for writing");
    return;
  }
  file.println(ssid);
  file.println(password);
  file.close();
  Serial.printf("Saved SSID: %s \n", ssid);
}

void EasyWiFi::loadCredentials() {
  Serial.println("EasyWiFi: loadCredentials()");
  File file = LittleFS.open("/wifi_credentials.txt", "r");
  if (!file) {
    Serial.println("EasyWiFi: No saved credentials found");
    return;
  }
  ssid = file.readStringUntil('\n');
  ssid.trim();
  password = file.readStringUntil('\n');
  password.trim();
  file.close();
  Serial.printf("Loaded SSID: %s \n", ssid.c_str());
}

void EasyWiFi::printCredentials() {
  Serial.println("EasyWiFi: printCredentials()");
  Serial.printf("SSID: %s\n", ssid.c_str());
  Serial.printf("Password: %s\n", password.c_str());
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
