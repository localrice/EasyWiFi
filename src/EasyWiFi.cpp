#include "EasyWiFi.h"

EasyWiFi::EasyWiFi() {}

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
}

void EasyWiFi::startPortal() {
  Serial.println("EasyWiFi: startPortal() (stub)");
  portalActive = true;
}

void EasyWiFi::handleClient() {
    // placeholder — will handle web requests later
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