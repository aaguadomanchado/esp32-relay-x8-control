#include "wifi_manager.h"
#include "pinout.h"
#include <WebServer.h>
#include <ESPmDNS.h>
 

extern WebServer server;          // Declared in main.cpp
extern Preferences preferences;   // Declared in main.cpp
extern bool isApMode;             // Declared in main.cpp
extern const char *APP_VERSION;   // Declared in main.cpp

// Helper to send a plain‑text response
static void sendPlain(const char *msg, int code = 200) {
    server.send(code, "text/plain", msg);
}

/* -------------------------------------------------------------------------- */
/* Wi‑Fi setup: tries to connect to stored STA credentials, falls back to AP   */
/* -------------------------------------------------------------------------- */
void wifiSetup(Preferences &prefs, bool &apMode, const char *apSSID, const char *apPassword) {
    prefs.begin("wifi-config", true);
    String savedSSID = prefs.getString("ssid", "");
    String savedPass = prefs.getString("pass", "");
    prefs.end();

    bool connected = false;
    if (savedSSID.length() > 0) {
        Serial.println("Connecting to " + savedSSID + "...");
        WiFi.mode(WIFI_STA);
        WiFi.begin(savedSSID.c_str(), savedPass.c_str());

        unsigned long startAttempt = millis();
        while (millis() - startAttempt < 20000) {
            if (WiFi.status() == WL_CONNECTED) {
                connected = true;
                break;
            }
            // Fast blink 100 ms while waiting for connection
            digitalWrite(VIS_STATUS_LED, !digitalRead(VIS_STATUS_LED));
            delay(100);
        }
    }

    if (connected) {
        Serial.println("Connected! IP: " + WiFi.localIP().toString());
        digitalWrite(VIS_STATUS_LED, HIGH); // Solid ON

        // NTP Setup: UTC+1 (3600 s offset), DST +1 h (3600 s)
        configTime(3600, 3600, "pool.ntp.org");
        Serial.println("NTP Configured (UTC+1)");

        if (MDNS.begin("esp32")) {
            Serial.println("mDNS responder started: esp32.local");
        }
        Serial.println("System Started - v" + String(APP_VERSION));
    } else {
        Serial.println("Connection failed or no config. Starting AP.");
        apMode = true;
        WiFi.mode(WIFI_AP);
        WiFi.softAP(apSSID, apPassword);
        IPAddress IP = WiFi.softAPIP();
        Serial.println("AP IP: " + IP.toString());
    }
}

/* -------------------------------------------------------------------------- */
/* Scan for nearby Wi‑Fi networks and return JSON array                       */
/* -------------------------------------------------------------------------- */
void handleScan() {
    int n = WiFi.scanNetworks();
    // Estimate maximum size: each entry ~100 bytes, plus brackets
    char json[4096];
    size_t pos = 0;
    json[pos++] = '[';
    for (int i = 0; i < n; ++i) {
        if (i) {
            json[pos++] = ',';
        }
        const char *ssid = WiFi.SSID(i).c_str();
        int rssi = WiFi.RSSI(i);
        bool secure = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;

        // Build object: {"ssid":"...", "rssi":-70, "secure":true}
        int written = snprintf(json + pos, sizeof(json) - pos,
                               "{\"ssid\":\"%s\",\"rssi\":%d,\"secure\":%s}",
                               ssid,
                               rssi,
                               secure ? "true" : "false");
        if (written < 0) break; // encoding error
        pos += (size_t)written;
    }
    json[pos++] = ']';
    json[pos] = '\0';
    server.send(200, "application/json", json);
}

/* -------------------------------------------------------------------------- */
/* Save Wi‑Fi credentials (STA mode) and restart                              */
/* -------------------------------------------------------------------------- */
void handleSaveWiFi() {
    if (server.hasArg("ssid") && server.hasArg("pass")) {
        const char *ssid = server.arg("ssid").c_str();
        const char *pass = server.arg("pass").c_str();

        preferences.begin("wifi-config", false);
        preferences.putString("ssid", ssid);
        preferences.putString("pass", pass);
        preferences.end();

        sendPlain("Saved. Restarting...", 200);
        delay(1000);
        ESP.restart();
    } else {
        sendPlain("Missing args", 400);
    }
}

/* -------------------------------------------------------------------------- */
/* Reset Wi‑Fi configuration and restart                                      */
/* -------------------------------------------------------------------------- */
void handleResetWiFi() {
    preferences.begin("wifi-config", false);
    preferences.clear();
    preferences.end();

    sendPlain("Reset. Restarting...", 200);
    delay(1000);
    ESP.restart();
}
