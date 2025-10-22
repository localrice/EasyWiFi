# TODO: Fix EasyWiFi Security and Reliability Issues

## Tasks
- [x] Edit EasyWiFi.h: Add private members for server, dnsServer, portal timer, restart timer, encryption key
- [x] Edit EasyWiFi.cpp: Remove global server and dnsServer declarations
- [x] Update /save handler: Add input validation, remove delay, set restart flag and timer
- [x] Update loop(): Handle non-blocking restart and portal timeout
- [x] Improve error handling in saveCredentials for persist failures
