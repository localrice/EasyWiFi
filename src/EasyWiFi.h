#ifndef EASYWIFI_H
#define EASYWIFI_H

#include <Arduino.h>

class EasyWiFi {
    public:
        EasyWiFi();

        void begin();
        void loop();
        void reset();

    private:
        bool portalActive = false;

        void tryConnect();
        void startPortal();
        void handleClient();
};

#endif
