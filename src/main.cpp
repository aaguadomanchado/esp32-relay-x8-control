#include "index_html.h"
#include "pinout.h"
#include "wifi_manager.h"
#include "credentials.h"
#include <Arduino.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <Update.h>
#include <WebServer.h>
#include <WiFi.h>
#include <time.h>

// AP Credentials (definidas en credentials.h)
const char *ssid = DEFAULT_AP_SSID;
const char *password = DEFAULT_AP_PASSWORD;

WebServer server(80);
Preferences preferences;

// Global state
int relayState[8] = {0};
const char *APP_VERSION = "0.8";
char logBuffer[2048] = "";
const char *relayLabels[8] = {"Relay 1", "Relay 2", "Relay 3", "Relay 4",
                             "Relay 5", "Relay 6", "Relay 7", "Relay 8"};
static char labelStore[8][32];
bool isApMode = false;

// Timer Structure
struct Timer {
  int startH = -1;
  int startM = -1;
  int endH = -1;
  int endM = -1;
  bool enabled = false;
  bool isDuration = false;
  int durationSec = 0;
};
Timer timers[8];

// Last on time for duration timers
unsigned long lastOnTime[8] = {0};

// Logging
void logEvent(const char *msg) {
  Serial.println(msg);
  if (strlen(logBuffer) > 1800) {
    logBuffer[0] = '\0';
  }
  strncat(logBuffer, msg, sizeof(logBuffer) - strlen(logBuffer) - 2);
  strncat(logBuffer, "\n", sizeof(logBuffer) - strlen(logBuffer) - 1);
}

// -- WEB HANDLERS --

void handleRoot() { 
  String html = index_html;
  html.replace("{{VERSION}}", APP_VERSION);
  server.send(200, "text/html", html); 
}

void handleToggle() {
  if (server.hasArg("channel") && server.hasArg("state")) {
    int channel = server.arg("channel").toInt();
    int state = server.arg("state").toInt();
    if (channel >= 1 && channel <= 8) {
      int idx = channel - 1;
      relayState[idx] = state;
      digitalWrite(RELAY_PINS[idx], state ? HIGH : LOW);

      // Save state
      preferences.begin("relay-states", false);
      preferences.putInt(("r" + String(idx)).c_str(), state);
      preferences.end();

      char msgBuf[128];
snprintf(msgBuf, sizeof(msgBuf), "Manual: %s %s", relayLabels[idx], state ? "ON" : "OFF");
logEvent(msgBuf);
      server.send(200, "text/plain", "OK");
      return;
    }
  }
  server.send(400, "text/plain", "Bad Req");
}

void handleStatus() {
  String json = "[";
  for (int i = 0; i < 8; i++) {
    json += String(relayState[i]);
    if (i < 7)
      json += ",";
  }
  json += "]";
  server.send(200, "application/json", json);
}

void handleLogs() {
  if (strlen(logBuffer) > 0) {
    server.send(200, "text/plain", logBuffer);
    logBuffer[0] = '\0';
  } else
    server.send(200, "text/plain", "");
}

// Time Sync: /set_time?epoch=1234567890
void handleSetTime() {
  if (server.hasArg("epoch")) {
    long epoch = server.arg("epoch").toInt();
    struct timeval tv;
    tv.tv_sec = epoch;
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);

    time_t now = time(nullptr);
    {
    char buf[128];
    snprintf(buf, sizeof(buf), "Time Synced: %s", ctime(&now));
    // Remove trailing newline from ctime
    size_t len = strlen(buf);
    if (len && buf[len - 1] == '\n') buf[len - 1] = '\0';
    logEvent(buf);
}
    server.send(200, "text/plain", "Time Set");
  } else {
    server.send(400, "text/plain", "No epoch");
  }
}

// Set Timer: /set_timer?channel=1&start=20:00&end=21:00
void handleSetTimer() {
  if (server.hasArg("channel") && server.hasArg("start") &&
      server.hasArg("end")) {
    int ch = server.arg("channel").toInt();
    if (ch < 1 || ch > 8) {
      server.send(400, "text/plain", "Bad CH");
      return;
    }

    String startStr = server.arg("start"); // "HH:MM"
    String endStr = server.arg("end");

    int idx = ch - 1;
    // Simple Parse
    timers[idx].startH = startStr.substring(0, 2).toInt();
    timers[idx].startM = startStr.substring(3, 5).toInt();
    timers[idx].endH = endStr.substring(0, 2).toInt();
    timers[idx].endM = endStr.substring(3, 5).toInt();

    // Check enabled param (default true if missing for backward compat)
    if (server.hasArg("enabled")) {
      timers[idx].enabled = server.arg("enabled") == "1";
    } else {
      timers[idx].enabled = true;
    }

    // New: isDuration and durationSec
    timers[idx].isDuration = server.hasArg("isDuration") && server.arg("isDuration") == "1";
    if (server.hasArg("duration")) {
      timers[idx].durationSec = server.arg("duration").toInt();
    } else {
      timers[idx].durationSec = 0;
    }

    // Save to NVS for persistence
    preferences.begin("timers", false);
    String key = "t" + String(ch);
    // Format: startH,startM,endH,endM,enabled,isDuration,durationSec
    String value = String(timers[idx].startH) + "," +
                   String(timers[idx].startM) + "," + String(timers[idx].endH) +
                   "," + String(timers[idx].endM) + "," +
                   String(timers[idx].enabled ? 1 : 0) + "," +
                   String(timers[idx].isDuration ? 1 : 0) + "," +
                   String(timers[idx].durationSec);
    preferences.putString(key.c_str(), value);
    preferences.end();

    String stateStr = timers[idx].enabled ? " [ON]" : " [OFF]";
    char timerMsg[256];
snprintf(timerMsg, sizeof(timerMsg), "Timer Set %s: %s - %s%s", relayLabels[idx], startStr.c_str(), endStr.c_str(), stateStr.c_str());
logEvent(timerMsg);
    server.send(200, "text/plain", "OK");
  } else
    server.send(400, "text/plain", "Missing args");
}

// -- MAIN LOOP --

// Clear Timer: /clear_timer?channel=X
void handleClearTimer() {
  if (server.hasArg("channel")) {
    int ch = server.arg("channel").toInt();
    if (ch < 1 || ch > 8) {
      server.send(400, "text/plain", "Bad CH");
      return;
    }

    int idx = ch - 1;
    timers[idx].enabled = false;
    timers[idx].startH = -1;
    timers[idx].startM = -1;
    timers[idx].endH = -1;
    timers[idx].endM = -1;

    // Remove from NVS
    preferences.begin("timers", false);
    String key = "t" + String(ch);
    preferences.remove(key.c_str());
    preferences.end();

    char clearMsg[128];
snprintf(clearMsg, sizeof(clearMsg), "Timer Cleared %s", relayLabels[idx]);
logEvent(clearMsg);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing args");
  }
}

// Get Timers: /get_timers -> JSON Array
void handleGetTimers() {
  String json = "[";
  for (int i = 0; i < 8; i++) {
    json += "{";
    json += "\"enabled\":" + String(timers[i].enabled ? "true" : "false");

    // Always return times if they are valid (not -1)
    if (timers[i].startH != -1) {
      char buf[16];
      sprintf(buf, ",\"start\":\"%02d:%02d\"", timers[i].startH,
              timers[i].startM);
      json += String(buf);
      sprintf(buf, ",\"end\":\"%02d:%02d\"", timers[i].endH, timers[i].endM);
      json += String(buf);
    } else {
      json += ",\"start\":\"\",\"end\":\"\"";
    }
    // New fields
    json += ",\"isDuration\":" + String(timers[i].isDuration ? "true" : "false");
    json += ",\"duration\":" + String(timers[i].durationSec);
    json += "}";
    if (i < 7)
      json += ",";
  }
  json += "]";
  server.send(200, "application/json", json);
}

// Get Time: /get_time -> JSON
void handleGetTime() {
  time_t now;
  time(&now);
  struct tm info;
  localtime_r(&now, &info);

  String json = "{";
  json += "\"epoch\":" + String(now);
  json += ",\"year\":" + String(info.tm_year + 1900);
  String timeStr = String(ctime(&now));
  timeStr.replace("\n", ""); // Remove newline
  json += ",\"str\":\"" + timeStr + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

// -- MAIN LOOP --

void checkTimers() {
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck < 1000)
    return;
  lastCheck = millis();

  time_t now;
  struct tm info;
  if (time(&now) < 100000)
    return; // Time not set yet
  localtime_r(&now, &info);

  static int lastMin = -1;
  if (info.tm_min == lastMin)
    return; // Already checked this minute
  lastMin = info.tm_min;

  {
    char buf[64];
    snprintf(buf, sizeof(buf), "Time Check: %d:%02d", info.tm_hour, info.tm_min);
    logEvent(buf);
}

  for (int i = 0; i < 8; i++) {
    if (!timers[i].enabled)
      continue;

    // Check Start
    if (info.tm_hour == timers[i].startH && info.tm_min == timers[i].startM) {
      if (timers[i].isDuration) {
        // Duration mode: turn on if off, and start duration timer
        if (relayState[i] == 0) {
          relayState[i] = 1;
          digitalWrite(RELAY_PINS[i], HIGH);
          lastOnTime[i] = millis();

          preferences.begin("relay-states", false);
          preferences.putInt(("r" + String(i)).c_str(), 1);
          preferences.end();

          {
    char buf[128];
    snprintf(buf, sizeof(buf), "Timer Duration Start: %s ON for %d s", relayLabels[i], timers[i].durationSec);
    logEvent(buf);
}
        }
      } else {
        // Daily mode: turn on if off
        if (relayState[i] == 0) {
          relayState[i] = 1;
          digitalWrite(RELAY_PINS[i], HIGH);

          preferences.begin("relay-states", false);
          preferences.putInt(("r" + String(i)).c_str(), 1);
          preferences.end();

          {
    char buf[64];
    snprintf(buf, sizeof(buf), "Timer Trigger: %s ON", relayLabels[i]);
    logEvent(buf);
}
        }
      }
    }
    // Check End (only for daily mode)
    if (!timers[i].isDuration && info.tm_hour == timers[i].endH && info.tm_min == timers[i].endM) {
      if (relayState[i] == 1) {
        relayState[i] = 0;
        digitalWrite(RELAY_PINS[i], LOW);

        preferences.begin("relay-states", false);
        preferences.putInt(("r" + String(i)).c_str(), 0);
        preferences.end();

        {
    char buf[64];
    snprintf(buf, sizeof(buf), "Timer Trigger: %s OFF", relayLabels[i]);
    logEvent(buf);
}
      }
    }
  }
}

void checkDurations() {
  for (int i = 0; i < 8; i++) {
    if (timers[i].enabled && timers[i].isDuration && relayState[i] == 1 && lastOnTime[i] > 0) {
      if (millis() - lastOnTime[i] >= (unsigned long)timers[i].durationSec * 1000) {
        relayState[i] = 0;
        digitalWrite(RELAY_PINS[i], LOW);
        lastOnTime[i] = 0; // Reset

        preferences.begin("relay-states", false);
        preferences.putInt(("r" + String(i)).c_str(), 0);
        preferences.end();

        {
    char buf[64];
    snprintf(buf, sizeof(buf), "Timer Duration End: %s OFF", relayLabels[i]);
    logEvent(buf);
}
      }
    }
  }
}

void updateLed() {
  if (isApMode) {
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 1000) {
      lastBlink = millis();
      digitalWrite(VIS_STATUS_LED, !digitalRead(VIS_STATUS_LED));
    }
  }
}

// -- WIFI MANAGER --




// -- LABELS --
void handleGetLabels() {
  String json = "[";
  for (int i = 0; i < 8; i++) {
    if (i)
      json += ",";
    json += "\"";
    json += relayLabels[i];
    json += "\"";
  }
  json += "]";
  server.send(200, "application/json", json);
}

void handleSetLabel() {
  if (server.hasArg("channel") && server.hasArg("label")) {
    int ch = server.arg("channel").toInt();
    String label = server.arg("label");

    if (ch >= 1 && ch <= 8) {
      {
    snprintf(labelStore[ch - 1], sizeof(labelStore[ch - 1]), "%s", label.c_str());
    relayLabels[ch - 1] = labelStore[ch - 1];
}

      // Save to Preferences
      preferences.begin("relay-labels", false);
      preferences.putString(("label" + String(ch)).c_str(), label);
      preferences.end();

      char labelMsg[256];
snprintf(labelMsg, sizeof(labelMsg), "Label updated: %s -> %s", relayLabels[ch - 1], label.c_str());
logEvent(labelMsg);
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Invalid channel");
    }
  } else {
    server.send(400, "text/plain", "Missing args");
  }
}

// -- SYSTEM INFO & OTA --
void handleSystemInfo() {
  unsigned long uptimeSeconds = millis() / 1000;
  unsigned long days = uptimeSeconds / 86400;
  unsigned long hours = (uptimeSeconds % 86400) / 3600;
  unsigned long minutes = (uptimeSeconds % 3600) / 60;
  unsigned long seconds = uptimeSeconds % 60;

  String json = "{";
  json += "\"version\":\"" + String(APP_VERSION) + "\"";
  json += ",\"freeHeap\":" + String(ESP.getFreeHeap());
  json += ",\"totalHeap\":" + String(ESP.getHeapSize());
  json += ",\"chipModel\":\"" + String(ESP.getChipModel()) + "\"";
  json += ",\"chipRevision\":" + String(ESP.getChipRevision());
  json += ",\"cpuFreqMHz\":" + String(ESP.getCpuFreqMHz());
  json += ",\"flashSize\":" + String(ESP.getFlashChipSize());
  json += ",\"sketchSize\":" + String(ESP.getSketchSize());
  json += ",\"freeSketchSpace\":" + String(ESP.getFreeSketchSpace());
  json += ",\"uptimeSeconds\":" + String(uptimeSeconds);
  json += ",\"uptimeStr\":\"" + String(days) + "d " + String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s\"";
  json += ",\"wifiRSSI\":" + String(WiFi.RSSI());
  json += ",\"wifiSSID\":\"" + WiFi.SSID() + "\"";
  json += ",\"ipAddress\":\"" + WiFi.localIP().toString() + "\"";
  json += ",\"macAddress\":\"" + WiFi.macAddress() + "\"";
  json += ",\"apMode\":" + String(isApMode ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

void handleUpdatePage() {
  // Landing page after update attempt
  server.send(200, "text/html", 
    "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<style>body{font-family:Arial;background:#121212;color:#e0e0e0;display:flex;"
    "justify-content:center;align-items:center;height:100vh;margin:0;}"
    ".box{background:#1e1e1e;padding:40px;border-radius:12px;text-align:center;}"
    "h2{color:#bb86fc;margin-bottom:20px;}"
    "a{color:#bb86fc;}</style></head><body>"
    "<div class='box'><h2>Firmware Update</h2>"
    "<p>Use the System tab in the main interface to upload firmware.</p>"
    "<p><a href='/'>Go to Control Panel</a></p></div></body></html>");
}

void handleDoUpdate() {
  // This is called when the upload is complete
  server.sendHeader("Connection", "close");
  if (Update.hasError()) {
    server.send(500, "application/json", "{\"success\":false,\"error\":\"Update failed\"}");
  } else {
    server.send(200, "application/json", "{\"success\":true,\"message\":\"Update successful! Rebooting...\"}");
    delay(1000);
    ESP.restart();
  }
}

void handleDoUpdateUpload() {
  HTTPUpload& upload = server.upload();
  
  if (upload.status == UPLOAD_FILE_START) {
    {
    char buf[128];
    snprintf(buf, sizeof(buf), "OTA Update Start: %s", upload.filename.c_str());
    logEvent(buf);
}
    
    // Start with max available size
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      {
    char buf[128];
    snprintf(buf, sizeof(buf), "OTA Error: %s", Update.errorString());
    logEvent(buf);
}
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    // Write chunked data to flash
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      {
    char buf[128];
    snprintf(buf, sizeof(buf), "OTA Write Error: %s", Update.errorString());
    logEvent(buf);
}
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      {
    char buf[128];
    snprintf(buf, sizeof(buf), "OTA Update Success: %d bytes", upload.totalSize);
    logEvent(buf);
}
    } else {
      {
    char buf[128];
    snprintf(buf, sizeof(buf), "OTA Error: %s", Update.errorString());
    logEvent(buf);
}
    }
  }
}

void handleReboot() {
  server.send(200, "application/json", "{\"success\":true,\"message\":\"Rebooting...\"}");
  delay(1000);
  ESP.restart();
}

// -- HOME ASSISTANT API --
// GET /api/ha -> Returns {"r1":"OFF", "r2":"ON", ...}
// POST /api/ha?channel=1&state=ON -> Sets state and returns current json
void handleHA() {
  // Handle POST/GET Action
  if (server.hasArg("channel") && server.hasArg("state")) {
    int ch = server.arg("channel").toInt();
    String stateStr = server.arg("state");
    stateStr.toUpperCase();

    if (ch >= 1 && ch <= 8 && (stateStr == "ON" || stateStr == "OFF")) {
      int idx = ch - 1;
      int newState = (stateStr == "ON") ? 1 : 0;

      if (relayState[idx] != newState) {
        relayState[idx] = newState;
        digitalWrite(RELAY_PINS[idx], newState ? HIGH : LOW);

        // Save state
        preferences.begin("relay-states", false);
        preferences.putInt(("r" + String(idx)).c_str(), newState);
        preferences.end();

        {
    char buf[128];
    snprintf(buf, sizeof(buf), "HA API: %s -> %s", relayLabels[idx], stateStr.c_str());
    logEvent(buf);
}
      }
    } else {
      // Invalid request
    }
  }

  // Build JSON Response
  String json = "{";
  for (int i = 0; i < 8; i++) {
    if (i > 0)
      json += ",";
    json +=
        "\"r" + String(i + 1) + "\":\"" + (relayState[i] ? "ON" : "OFF") + "\"";
  }
  json += "}";
  server.send(200, "application/json", json);
}

// -- SETUP --
void setup() {
  Serial.begin(115200);

  // Init Pins & Restore State
  preferences.begin("relay-states", true);
  pinMode(VIS_STATUS_LED, OUTPUT);
  digitalWrite(VIS_STATUS_LED, LOW);
  for (int i = 0; i < NUM_RELAYS; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
    // Load saved state (default 0/OFF)
    int savedState = preferences.getInt(("r" + String(i)).c_str(), 0);
    relayState[i] = savedState;
    digitalWrite(RELAY_PINS[i], savedState ? HIGH : LOW);
  }
  preferences.end();

  // Load saved labels
  preferences.begin("relay-labels", true);
  for (int i = 1; i <= 8; i++) {
    String saved = preferences.getString(("label" + String(i)).c_str(), "");
    if (saved != "") {
      snprintf(labelStore[i - 1], sizeof(labelStore[i - 1]), "%s", saved.c_str());
      relayLabels[i - 1] = labelStore[i - 1];
    }
  }
  preferences.end();

  // Load saved timers
  preferences.begin("timers", true);
  for (int i = 1; i <= 8; i++) {
    String key = "t" + String(i);
    String value = preferences.getString(key.c_str(), "");
    if (value != "") {
      int idx = i - 1;
      // Parse "startH,startM,endH,endM"
      int comma1 = value.indexOf(',');
      int comma2 = value.indexOf(',', comma1 + 1);
      int comma3 = value.indexOf(',', comma2 + 1);

      if (comma1 > 0 && comma2 > 0 && comma3 > 0) {
        timers[idx].startH = value.substring(0, comma1).toInt();
        timers[idx].startM = value.substring(comma1 + 1, comma2).toInt();
        timers[idx].endH = value.substring(comma2 + 1, comma3).toInt();

        // Handle new format with 5th element (enabled)
        int comma4 = value.indexOf(',', comma3 + 1);
        if (comma4 > 0) {
          timers[idx].endM = value.substring(comma3 + 1, comma4).toInt();
          timers[idx].enabled = value.substring(comma4 + 1).toInt() == 1;

          // New: check for isDuration and durationSec (6th and 7th)
          int comma5 = value.indexOf(',', comma4 + 1);
          if (comma5 > 0) {
            timers[idx].isDuration = value.substring(comma4 + 1, comma5).toInt() == 1;
            timers[idx].durationSec = value.substring(comma5 + 1).toInt();
          } else {
            // Old format, assume false, 0
            timers[idx].isDuration = false;
            timers[idx].durationSec = 0;
          }
        } else {
          // Old format
          timers[idx].endM = value.substring(comma3 + 1).toInt();
          timers[idx].enabled = true;
          timers[idx].isDuration = false;
          timers[idx].durationSec = 0;
        }

{
    const char *stateStr = timers[idx].enabled ? " [ON]" : " [OFF]";
    char timerBuf[128];
    snprintf(timerBuf, sizeof(timerBuf), "Timer Loaded %s: %02d:%02d - %02d:%02d%s",
             relayLabels[idx],
             timers[idx].startH, timers[idx].startM,
             timers[idx].endH, timers[idx].endM,
             stateStr);
    logEvent(timerBuf);
}
      }
    }
  }
  preferences.end();

  // WiFi Logic
  preferences.begin("wifi-config", true);
  String savedSSID = preferences.getString("ssid", "");
  String savedPass = preferences.getString("pass", "");
  preferences.end();

  bool connected = false;
  String ssidToUse;
  String passToUse;

  if (savedSSID != "") {
    ssidToUse = savedSSID;
    passToUse = savedPass;
  } else {
    // Usar credenciales por defecto para STA
    ssidToUse = DEFAULT_STA_SSID;
    passToUse = DEFAULT_STA_PASSWORD;
  }

  if (ssidToUse != "") {
    {
    char buf[128];
    snprintf(buf, sizeof(buf), "Intentando conectar a: %s", ssidToUse.c_str());
    logEvent(buf);
}
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssidToUse.c_str(), passToUse.c_str());

    // Wait for connection (20s timeout)
    unsigned long startAttempt = millis();
    while (millis() - startAttempt < 20000) {
      if (WiFi.status() == WL_CONNECTED) {
        connected = true;
        break;
      }
      // Fast blink 100ms
      digitalWrite(VIS_STATUS_LED, !digitalRead(VIS_STATUS_LED));
      delay(100);
    }
  }

  if (connected) {
    char connMsg[128];
snprintf(connMsg, sizeof(connMsg), "Connected! IP: %s", WiFi.localIP().toString().c_str());
logEvent(connMsg);
    digitalWrite(VIS_STATUS_LED, HIGH); // Solid ON

    // NTP Setup: UTC+1 (3600s offset), Daylight Savings 1h (3600s)
    configTime(3600, 3600, "pool.ntp.org");
    logEvent("NTP Configured (UTC+1)");

    if (MDNS.begin("esp32")) {
      logEvent("mDNS responder started: esp32.local");
    }
    {
    char buf[64];
    snprintf(buf, sizeof(buf), "System Started - v%s", APP_VERSION);
    logEvent(buf);
}
  } else {
    logEvent("Connection failed or no config. Starting AP.");
    isApMode = true;
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    char apMsg[128];
snprintf(apMsg, sizeof(apMsg), "AP IP: %s", IP.toString().c_str());
logEvent(apMsg);
  }

  // Routes
  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);
  server.on("/status", handleStatus);
  server.on("/logs", handleLogs);
  server.on("/set_time", handleSetTime);
  server.on("/get_time", handleGetTime);
  server.on("/set_timer", handleSetTimer);
  server.on("/clear_timer", handleClearTimer);
  server.on("/get_timers", handleGetTimers);
  // WiFi
  server.on("/scan", handleScan);
  server.on("/save_wifi", handleSaveWiFi);
  server.on("/reset_wifi", handleResetWiFi);
  // Labels
  server.on("/get_labels", handleGetLabels);
  server.on("/set_label", handleSetLabel);
  // Home Assistant
  server.on("/api/ha", handleHA);
  // System & OTA
  server.on("/system_info", HTTP_GET, handleSystemInfo);
  server.on("/update", HTTP_GET, handleUpdatePage);
  server.on("/do_update", HTTP_POST, handleDoUpdate, handleDoUpdateUpload);
  server.on("/reboot", HTTP_POST, handleReboot);

  server.begin();
  if (!isApMode)
    digitalWrite(VIS_STATUS_LED, HIGH); // Ensure solid ON if not AP
}

void loop() {
  server.handleClient();
  checkTimers();
  checkDurations();
  updateLed();
}
