# Potential Issues and Improvements in EasyWiFi Library

## Description
This issue outlines several potential issues and areas for improvement identified in the EasyWiFi library project. Addressing these points will enhance the security, robustness, and user experience of the library.

## Issues

1. **Security Concern: Plaintext Password Storage**  
   WiFi passwords are currently stored in plaintext within the LittleFS filesystem. This could pose a security risk if unauthorized access to the device's storage occurs. Consider implementing encryption or obfuscation for stored credentials.

2. **Blocking Delay in Captive Portal**  
   The captive portal uses a blocking delay (`delay(2000)`) before restarting the device after saving credentials. This may block other tasks or processes running on the device and could be replaced with a non-blocking approach.

3. **Minimal Input Validation**  
   There is minimal validation of SSID and password inputs beyond checking that the SSID is not empty. Additional validation could prevent invalid or problematic credentials from being saved.

4. **Lack of Error Handling on Credential Save Failure**  
   The code does not explicitly handle errors when saving credentials fails, which could result in silent failures. Adding error handling and user feedback would improve reliability.

5. **Captive Portal URL Handling**  
   The captive portal redirects all unknown URLs to the root page, which may mask errors or unexpected requests. This behavior could be refined to better handle invalid requests.

6. **Concurrent Client Handling in Captive Portal**  
   The captive portal may not handle multiple concurrent clients or heavy load gracefully. Testing and improvements in this area could enhance usability.

7. **Global Server and DNS Server Instances**  
   The use of global `server` and `dnsServer` objects may cause issues if multiple instances of EasyWiFi are created. While typically only one instance is expected, this could be documented or refactored.

8. **No Recovery Mechanism for Portal Mode**  
   There is no watchdog or recovery mechanism if the device becomes stuck in captive portal mode indefinitely. Implementing such a mechanism could improve device reliability.

## Suggested Actions
- Evaluate and implement encryption for stored credentials.
- Replace blocking delays with non-blocking alternatives.
- Add comprehensive input validation for SSID and password.
- Implement error handling and user feedback for credential saving.
- Refine captive portal URL handling and error responses.
- Test and improve captive portal handling of concurrent clients.
- Document or refactor global server instances to support multiple EasyWiFi instances if needed.
- Add a watchdog or timeout mechanism for captive portal mode.

## Additional Notes
These improvements aim to make the EasyWiFi library more secure, robust, and user-friendly. Prioritization can be based on user feedback and usage scenarios.

---

Please consider these points for future updates to the EasyWiFi library.
