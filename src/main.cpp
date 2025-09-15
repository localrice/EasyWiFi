#include <Arduino.h>
#include "EasyWiFi.h"

EasyWiFi wifi;

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n=== WiFiThing skeleton starting ===");
  wifi.begin();

  wifi.saveCredentials("TestSSID", "TestPassword");
  wifi.loadCredentials();

  wifi.printCredentials();
}

void loop() {
  wifi.loop();
  delay(10);
}
