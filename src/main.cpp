#include "index_html.h"
#include "pinout.h"
#include <Arduino.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>
#include <time.h>

// AP Credentials
const char *ssid = "ESP32-Relay-X8";
const char *password = "12345678";

WebServer server(80);
Preferences preferences;

// Global state
int relayState[8] = {0};
const char *APP_VERSION = "0.3";
String logBuffer = "";
String relayLabels[8] = {"Relay 1", "Relay 2", "Relay 3", "Relay 4",
                         "Relay 5", "Relay 6", "Relay 7", "Relay 8"};
bool isApMode = false;

// Timer Structure
struct Timer {
  int startH = -1;
  int startM = -1;
  int endH = -1;
  int endM = -1;
  bool enabled = false;
};
Timer timers[8];

// Logging
void logEvent(String msg) {
  Serial.println(msg);
  if (logBuffer.length() > 2000)
    logBuffer = "";
  logBuffer += msg + "\n";
}

// -- WEB HANDLERS --

void handleRoot() { server.send(200, "text/html", index_html); }

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

      logEvent("Manual: " + relayLabels[idx] + (state ? " ON" : " OFF"));
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
  if (logBuffer.length() > 0) {
    server.send(200, "text/plain", logBuffer);
    logBuffer = "";
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
    logEvent("Time Synced: " + String(ctime(&now)));
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

    // Save to NVS for persistence
    preferences.begin("timers", false);
    String key = "t" + String(ch);
    // Format: startH,startM,endH,endM,enabled
    String value = String(timers[idx].startH) + "," +
                   String(timers[idx].startM) + "," + String(timers[idx].endH) +
                   "," + String(timers[idx].endM) + "," +
                   String(timers[idx].enabled ? 1 : 0);
    preferences.putString(key.c_str(), value);
    preferences.end();

    String stateStr = timers[idx].enabled ? " [ON]" : " [OFF]";
    logEvent("Timer Set " + relayLabels[idx] + ": " + startStr + " - " +
             endStr + stateStr);
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

    logEvent("Timer Cleared " + relayLabels[idx]);
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

  logEvent("Time Check: " + String(info.tm_hour) + ":" + String(info.tm_min));

  for (int i = 0; i < 8; i++) {
    if (!timers[i].enabled)
      continue;

    // Check Start
    if (info.tm_hour == timers[i].startH && info.tm_min == timers[i].startM) {
      if (relayState[i] == 0) {
        relayState[i] = 1;
        digitalWrite(RELAY_PINS[i], HIGH);

        preferences.begin("relay-states", false);
        preferences.putInt(("r" + String(i)).c_str(), 1);
        preferences.end();

        logEvent("Timer Trigger: " + relayLabels[i] + " ON");
      }
    }
    // Check End
    if (info.tm_hour == timers[i].endH && info.tm_min == timers[i].endM) {
      if (relayState[i] == 1) {
        relayState[i] = 0;
        digitalWrite(RELAY_PINS[i], LOW);

        preferences.begin("relay-states", false);
        preferences.putInt(("r" + String(i)).c_str(), 0);
        preferences.end();

        logEvent("Timer Trigger: " + relayLabels[i] + " OFF");
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

void handleScan() {
  int n = WiFi.scanNetworks();
  String json = "[";
  for (int i = 0; i < n; ++i) {
    if (i)
      json += ",";
    json += "{";
    json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
    json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
    json += "\"secure\":" +
            String(WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "true" : "false");
    json += "}";
  }
  json += "]";
  server.send(200, "application/json", json);
}

void handleSaveWiFi() {
  if (server.hasArg("ssid") && server.hasArg("pass")) {
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");

    preferences.begin("wifi-config", false);
    preferences.putString("ssid", ssid);
    preferences.putString("pass", pass);
    preferences.end();

    server.send(200, "text/plain", "Saved. Restarting...");
    delay(1000);
    ESP.restart();
  } else {
    server.send(400, "text/plain", "Missing args");
  }
}

void handleResetWiFi() {
  preferences.begin("wifi-config", false);
  preferences.clear();
  preferences.end();
  server.send(200, "text/plain", "Reset. Restarting...");
  delay(1000);
  ESP.restart();
}

// -- LABELS --
void handleGetLabels() {
  String json = "[";
  for (int i = 0; i < 8; i++) {
    if (i)
      json += ",";
    json += "\"" + relayLabels[i] + "\"";
  }
  json += "]";
  server.send(200, "application/json", json);
}

void handleSetLabel() {
  if (server.hasArg("channel") && server.hasArg("label")) {
    int ch = server.arg("channel").toInt();
    String label = server.arg("label");

    if (ch >= 1 && ch <= 8) {
      relayLabels[ch - 1] = label;

      // Save to Preferences
      preferences.begin("relay-labels", false);
      preferences.putString(("label" + String(ch)).c_str(), label);
      preferences.end();

      logEvent("Label updated: " + relayLabels[ch - 1] + " -> " + label);
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Invalid channel");
    }
  } else {
    server.send(400, "text/plain", "Missing args");
  }
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

        logEvent("HA API: " + relayLabels[idx] + " -> " + stateStr);
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
      relayLabels[i - 1] = saved;
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
        } else {
          // Old format
          timers[idx].endM = value.substring(comma3 + 1).toInt();
          timers[idx].enabled = true;
        }

        String stateStr = timers[idx].enabled ? " [ON]" : " [OFF]";
        logEvent("Timer Loaded " + relayLabels[idx] + ": " +
                 String(timers[idx].startH) + ":" + String(timers[idx].startM) +
                 " - " + String(timers[idx].endH) + ":" +
                 String(timers[idx].endM) + stateStr);
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
  if (savedSSID != "") {
    logEvent("Connecting to " + savedSSID + "...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(savedSSID.c_str(), savedPass.c_str());

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
    logEvent("Connected! IP: " + WiFi.localIP().toString());
    digitalWrite(VIS_STATUS_LED, HIGH); // Solid ON

    // NTP Setup: UTC+1 (3600s offset), Daylight Savings 1h (3600s)
    configTime(3600, 3600, "pool.ntp.org");
    logEvent("NTP Configured (UTC+1)");

    if (MDNS.begin("esp32")) {
      logEvent("mDNS responder started: esp32.local");
    }
    logEvent("System Started - v" + String(APP_VERSION));
  } else {
    logEvent("Connection failed or no config. Starting AP.");
    isApMode = true;
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    logEvent("AP IP: " + IP.toString());
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

  server.begin();
  server.begin();
  if (!isApMode)
    digitalWrite(VIS_STATUS_LED, HIGH); // Ensure solid ON if not AP
}

void loop() {
  server.handleClient();
  checkTimers();
  updateLed();
}
