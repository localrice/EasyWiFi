#include <Arduino.h>
#include <EasyWiFi.h>

EasyWiFi wifi;

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Change AP name/password for the captive portal (optional)
    wifi.setAP("MyESP8266_AP", "supersecret");  

    // Optional: configure reconnect parameters (maxAttempts, interval ms)
    wifi.setReconnectParams(5, 3000);

    // Optional: set a static IP (uncomment if needed)
    // wifi.setStaticIP(IPAddress(192,168,1,50),
    //                  IPAddress(192,168,1,1),
    //                  IPAddress(255,255,255,0),
    //                  IPAddress(8,8,8,8));

    // Start EasyWiFi
    wifi.begin();

    Serial.println("EasyWiFi example started.");
}

void loop() {
    // required: keep WiFi manager running
    wifi.loop();

    // your main code can go here
    Serial.println("Looping...");
}
