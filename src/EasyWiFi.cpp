#include "EasyWiFi.h"

ESP8266WebServer server(80);

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
  if (portalActive) {
    handleClient();
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
  WiFi.softAP("EasyWiFi-Setup");

  server.on("/", [this]() {
      String html = "<h1>EasyWiFi Setup</h1>"
                  "<form method='POST' action='/save'>"
                  "SSID: <input name='ssid' value='" + ssid + "'><br>"
                  "Password: <input type='password' name='password' value='" + password +"'><br>"
                  "<input type='submit' value='Save'>"
                  "</form>";
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