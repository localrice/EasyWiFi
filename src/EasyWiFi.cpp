#include "EasyWiFi.h"

EasyWiFi::EasyWiFi() {}

void EasyWiFi::begin() {
  Serial.println("EasyWiFi: begin()");
}

void EasyWiFi::loop() {
  if (portalActive) {
    handleClient();
  }
}

void EasyWiFi::reset() {
  Serial.println("EasyWiFi: reset() (stub)");
}

void EasyWiFi::tryConnect() {
  Serial.println("EasyWiFi: tryConnect() (stub)");
}

void EasyWiFi::startPortal() {
  Serial.println("EasyWiFi: startPortal() (stub)");
  portalActive = true;
}

void EasyWiFi::handleClient() {
  // placeholder — will handle web requests later
}
