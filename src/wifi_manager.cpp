#include "wifi_manager.h"
#include "pinout.h"
#include <ESPmDNS.h>
#include <time.h>

extern WebServer server;        // Declared in main.cpp
extern Preferences preferences; // Declared in main.cpp
extern void logEvent(const char *msg);

// ---------------------------------------------------------------------------
// Reconexion automatica de WiFi (llamar desde loop())
// ---------------------------------------------------------------------------
static unsigned long lastWifiCheck = 0;
static const unsigned long WIFI_CHECK_INTERVAL = 30000; // 30 s

void checkWifiReconnect() {
  if (millis() - lastWifiCheck < WIFI_CHECK_INTERVAL)
    return;
  lastWifiCheck = millis();

  if (WiFi.status() == WL_CONNECTED)
    return;

  static unsigned long lastRetry = 0;
  // Reintento como mucho cada 15 s
  if (millis() - lastRetry < 15000)
    return;
  lastRetry = millis();

  logEvent("WiFi perdido. Reintentando...");
  WiFi.disconnect();
  WiFi.reconnect();
}

/* -------------------------------------------------------------------------- */
/* Wi-Fi setup: tries to connect to stored STA credentials, falls back to AP   */
/* -------------------------------------------------------------------------- */
void wifiSetup(Preferences &prefs, bool &apMode, const char *apSSID,
               const char *apPassword) {
  prefs.begin("wifi-config", true);
  String savedSSID = prefs.getString("ssid", "");
  String savedPass = prefs.getString("pass", "");
  prefs.end();

  bool connected = false;

  if (savedSSID.length() > 0 && savedSSID.length() <= 32) {
    {
      char buf[128];
      snprintf(buf, sizeof(buf), "Intentando conectar a: %s", savedSSID.c_str());
      logEvent(buf);
    }
    WiFi.mode(WIFI_STA);
    WiFi.begin(savedSSID.c_str(), savedPass.c_str());

    unsigned long startAttempt = millis();
    while (millis() - startAttempt < 20000) {
      if (WiFi.status() == WL_CONNECTED) {
        connected = true;
        break;
      }
      digitalWrite(VIS_STATUS_LED, !digitalRead(VIS_STATUS_LED));
      delay(100);
    }
  }

  if (connected) {
    char buf[128];
    snprintf(buf, sizeof(buf), "Connected! IP: %s",
             WiFi.localIP().toString().c_str());
    logEvent(buf);

    // Zona horaria Espana con DST automatico
    configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.google.com");
    logEvent("NTP Configured (Europe/Madrid)");

    if (MDNS.begin("esp32")) {
      logEvent("mDNS responder started: esp32.local");
    }
  } else {
    logEvent("Connection failed or no config. Starting AP.");
    apMode = true;
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSSID, apPassword);
    IPAddress IP = WiFi.softAPIP();
    char buf[64];
    snprintf(buf, sizeof(buf), "AP IP: %s", IP.toString().c_str());
    logEvent(buf);
  }
}

/* -------------------------------------------------------------------------- */
/* Scan for nearby Wi-Fi networks and return JSON array (bounded buffer)       */
/* -------------------------------------------------------------------------- */
void handleScan() {
  int n = WiFi.scanNetworks();
  const size_t CAP = 4096;
  char *json = (char *)malloc(CAP);
  if (!json) {
    server.send(500, "text/plain", "OOM");
    return;
  }
  size_t pos = 0;
  json[pos++] = '[';
  for (int i = 0; i < n; ++i) {
    const char *ssid = WiFi.SSID(i).c_str();
    int rssi = WiFi.RSSI(i);
    bool secure = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;

    // Escapar comillas dobles y backslashes del SSID para JSON valido
    char esc[65];
    size_t e = 0;
    for (const char *p = ssid; *p && e < sizeof(esc) - 1; ++p) {
      if (*p == '"' || *p == '\\')
        esc[e++] = '\\';
      esc[e++] = *p;
    }
    esc[e] = '\0';

    if (i)
      json[pos++] = ',';
    int written =
        snprintf(json + pos, CAP - pos, "{\"ssid\":\"%s\",\"rssi\":%d,\"secure\":%s}",
                 esc, rssi, secure ? "true" : "false");
    if (written < 0 || (size_t)written >= CAP - pos)
      break; // buffer lleno: parar de anadir redes
    pos += (size_t)written;
  }
  json[pos++] = ']';
  json[pos] = '\0';
  server.send(200, "application/json", json);
  free(json);
}

/* -------------------------------------------------------------------------- */
/* Save Wi-Fi credentials (STA mode) with validation, then restart             */
/* -------------------------------------------------------------------------- */
void handleSaveWiFi() {
  if (server.hasArg("ssid") && server.hasArg("pass")) {
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");

    // Validacion: SSID max 32 chars, pass 8-63 chars (estandar WPA2)
    if (ssid.isEmpty() || ssid.length() > 32 || pass.length() < 8 ||
        pass.length() > 63) {
      server.send(400, "text/plain",
                  "Invalid: SSID 1-32 chars, PASS 8-63 chars");
      return;
    }

    preferences.begin("wifi-config", false);
    preferences.putString("ssid", ssid);
    preferences.putString("pass", pass);
    preferences.end();

    logEvent("WiFi config guardada. Reiniciando...");
    server.send(200, "text/plain", "Saved. Restarting...");
    delay(1000);
    ESP.restart();
  } else {
    server.send(400, "text/plain", "Missing args");
  }
}

/* -------------------------------------------------------------------------- */
/* Reset Wi-Fi configuration and restart                                       */
/* -------------------------------------------------------------------------- */
void handleResetWiFi() {
  preferences.begin("wifi-config", false);
  preferences.clear();
  preferences.end();

  logEvent("WiFi config borrada. Reiniciando...");
  server.send(200, "text/plain", "Reset. Restarting...");
  delay(1000);
  ESP.restart();
}
