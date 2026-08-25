#include "index_html.h"
#include "pinout.h"
#include "wifi_manager.h"
#include "credentials.h"
#include <Arduino.h>
#include <Preferences.h>
#include <Update.h>
#include <WebServer.h>
#include <WiFi.h>
#include <time.h>

// ---------------------------------------------------------------------------
// Config
// ---------------------------------------------------------------------------
const char *APP_VERSION = "0.9";

// Token de autenticacion para endpoints sensibles (OTA, reboot, wifi config).
// Cambialo o dejalo vacio para desactivar la comprobacion.
const char *ADMIN_TOKEN = "cambia-este-token";

// ---------------------------------------------------------------------------
// Global state
// ---------------------------------------------------------------------------
WebServer server(80);
Preferences preferences;

bool relayState[NUM_RELAYS] = {false};
char relayLabels[NUM_RELAYS][32];
bool isApMode = false;

struct Timer {
  int startH = -1;
  int startM = -1;
  int endH = -1;
  int endM = -1;
  bool enabled = false;
  bool isDuration = false;
  uint32_t durationSec = 0;   // duracion en segundos
};
Timer timers[NUM_RELAYS];

uint64_t lastOnTimeMs[NUM_RELAYS] = {0}; // millis() de 64 bits

// millis() extendido a 64 bits: no desborda a los 49 dias
static uint64_t millis64() {
  static uint32_t low32 = 0;
  static uint32_t high32 = 0;
  uint32_t now = millis();
  if (now < low32)
    high32++;
  low32 = now;
  return ((uint64_t)high32 << 32) | low32;
}

// ---------------------------------------------------------------------------
// Log circular no destructivo
// ---------------------------------------------------------------------------
static const size_t LOG_LINES = 40;
static const size_t LOG_LINE_LEN = 96;
static char logStore[LOG_LINES][LOG_LINE_LEN];
static size_t logHead = 0;   // siguiente posicion a escribir
static size_t logCount = 0;  // lineas validas

void logEvent(const char *msg) {
  Serial.println(msg);
  snprintf(logStore[logHead], LOG_LINE_LEN, "%s", msg);
  logHead = (logHead + 1) % LOG_LINES;
  if (logCount < LOG_LINES)
    logCount++;
}

// Construye el contenido del log (mas antiguo primero) en un buffer estatico
String getLogText() {
  String out;
  out.reserve(logCount * LOG_LINE_LEN);
  size_t start = (logCount == LOG_LINES) ? logHead : 0;
  for (size_t i = 0; i < logCount; i++) {
    out += logStore[(start + i) % LOG_LINES];
    out += '\n';
  }
  return out;
}

// ---------------------------------------------------------------------------
// Auth helper para rutas sensibles
// ---------------------------------------------------------------------------
bool checkAuth() {
  if (ADMIN_TOKEN[0] == '\0')
    return true; // auth desactivada
  if (server.header("X-Auth-Token") != nullptr &&
      strcmp(server.header("X-Auth-Token").c_str(), ADMIN_TOKEN) == 0)
    return true;
  server.send(401, "application/json",
              "{\"success\":false,\"error\":\"unauthorized\"}");
  return false;
}

void setRelay(int idx, bool on, const char *source) {
  if (idx < 0 || idx >= NUM_RELAYS || on == relayState[idx])
    return;
  relayState[idx] = on;
  digitalWrite(RELAY_PINS[idx], on ? HIGH : LOW);

  preferences.begin("relay-states", false);
  preferences.putInt(("r" + String(idx)).c_str(), on ? 1 : 0);
  preferences.end();

  // Encendido manual con timer de duracion activo: arrancar cuenta atras
  if (on && timers[idx].enabled && timers[idx].isDuration)
    lastOnTimeMs[idx] = millis64();
  if (!on)
    lastOnTimeMs[idx] = 0;

  char buf[LOG_LINE_LEN];
  snprintf(buf, sizeof(buf), "%s: %s %s", source, relayLabels[idx],
           on ? "ON" : "OFF");
  logEvent(buf);
}

// ---------------------------------------------------------------------------
// Web handlers
// ---------------------------------------------------------------------------
void handleRoot() {
  String html = index_html;
  html.replace("{{VERSION}}", APP_VERSION);
  server.send(200, "text/html", html);
}

void handleToggle() {
  if (!server.hasArg("channel") || !server.hasArg("state")) {
    server.send(400, "text/plain", "Missing args");
    return;
  }
  int ch = server.arg("channel").toInt();
  String stateStr = server.arg("state");
  if (ch < 1 || ch > NUM_RELAYS ||
      !(stateStr == "0" || stateStr == "1")) {
    server.send(400, "text/plain", "Bad args");
    return;
  }
  setRelay(ch - 1, stateStr == "1", "Manual");
  server.send(200, "text/plain", "OK");
}

void handleStatus() {
  char json[NUM_RELAYS * 4 + 4];
  size_t pos = 0;
  json[pos++] = '[';
  for (int i = 0; i < NUM_RELAYS; i++) {
    if (i)
      json[pos++] = ',';
    pos += snprintf(json + pos, sizeof(json) - pos, "%d", relayState[i]);
  }
  json[pos++] = ']';
  json[pos] = '\0';
  server.send(200, "application/json", json);
}

void handleLogs() {
  // Lectura NO destructiva: el buffer circular conserva las ultimas lineas.
  // Opcional ?clear=1 para vaciar.
  server.send(200, "text/plain", getLogText());
  if (server.hasArg("clear"))
    logCount = 0, logHead = 0;
}

void handleSetTime() {
  if (!server.hasArg("epoch")) {
    server.send(400, "text/plain", "No epoch");
    return;
  }
  struct timeval tv;
  tv.tv_sec = server.arg("epoch").toInt();
  tv.tv_usec = 0;
  settimeofday(&tv, NULL);

  time_t now = time(nullptr);
  char buf[LOG_LINE_LEN];
  snprintf(buf, sizeof(buf), "Time Synced: %s", ctime(&now));
  size_t len = strlen(buf);
  if (len && buf[len - 1] == '\n')
    buf[len - 1] = '\0';
  logEvent(buf);
  server.send(200, "text/plain", "Time Set");
}

void handleGetTime() {
  time_t now = time(nullptr);
  struct tm info;
  localtime_r(&now, &info);
  char strBuf[32];
  strftime(strBuf, sizeof(strBuf), "%a %b %e %H:%M:%S %Y", &info);
  char json[80];
  snprintf(json, sizeof(json), "{\"epoch\":%lld,\"str\":\"%s\"}",
           (long long)now, strBuf);
  server.send(200, "application/json", json);
}

void handleSetTimer() {
  if (!server.hasArg("channel") || !server.hasArg("start") ||
      !server.hasArg("end")) {
    server.send(400, "text/plain", "Missing args");
    return;
  }
  int ch = server.arg("channel").toInt();
  if (ch < 1 || ch > NUM_RELAYS) {
    server.send(400, "text/plain", "Bad CH");
    return;
  }

  String startStr = server.arg("start");
  String endStr = server.arg("end");

  Timer t;
  t.startH = startStr.substring(0, 2).toInt();
  t.startM = startStr.substring(3, 5).toInt();
  t.endH = endStr.substring(0, 2).toInt();
  t.endM = endStr.substring(3, 5).toInt();
  // Validacion de horas
  if (t.startH < 0 || t.startH > 23 || t.startM < 0 || t.startM > 59 ||
      t.endH < 0 || t.endH > 23 || t.endM < 0 || t.endM > 59) {
    server.send(400, "text/plain", "Invalid time");
    return;
  }
  t.enabled = server.hasArg("enabled") ? server.arg("enabled") == "1" : true;
  t.isDuration =
      server.hasArg("isDuration") && server.arg("isDuration") == "1";
  long d = server.hasArg("duration") ? server.arg("duration").toInt() : 0;
  t.durationSec = (d > 0 && d <= 86400L * 30) ? (uint32_t)d : 0;

  timers[ch - 1] = t;

  // Persistir
  preferences.begin("timers", false);
  String value = String(t.startH) + "," + String(t.startM) + "," +
                 String(t.endH) + "," + String(t.endM) + "," +
                 String(t.enabled ? 1 : 0) + "," +
                 String(t.isDuration ? 1 : 0) + "," + String(t.durationSec);
  preferences.putString(("t" + String(ch)).c_str(), value);
  preferences.end();

  char msg[LOG_LINE_LEN];
  snprintf(msg, sizeof(msg), "Timer Set %s: %02d:%02d-%02d:%02d%s",
           relayLabels[ch - 1], t.startH, t.startM, t.endH, t.endM,
           t.enabled ? " [ON]" : " [OFF]");
  logEvent(msg);
  server.send(200, "text/plain", "OK");
}

void handleClearTimer() {
  int ch = server.arg("channel").toInt();
  if (ch < 1 || ch > NUM_RELAYS) {
    server.send(400, "text/plain", "Bad CH");
    return;
  }
  int idx = ch - 1;
  timers[idx].enabled = false;
  timers[idx].startH = -1;
  timers[idx].startM = -1;
  timers[idx].endH = -1;
  timers[idx].endM = -1;
  lastOnTimeMs[idx] = 0;

  preferences.begin("timers", false);
  preferences.remove(("t" + String(ch)).c_str());
  preferences.end();

  char msg[LOG_LINE_LEN];
  snprintf(msg, sizeof(msg), "Timer Cleared %s", relayLabels[idx]);
  logEvent(msg);
  server.send(200, "text/plain", "OK");
}

void handleGetTimers() {
  String json = "[";
  for (int i = 0; i < NUM_RELAYS; i++) {
    if (i)
      json += ",";
    json += "{\"enabled\":";
    json += timers[i].enabled ? "true" : "false";
    if (timers[i].startH != -1) {
      char buf[48];
      snprintf(buf, sizeof(buf), ",\"start\":\"%02d:%02d\",\"end\":\"%02d:%02d\"",
               timers[i].startH, timers[i].startM, timers[i].endH,
               timers[i].endM);
      json += buf;
    } else {
      json += ",\"start\":\"\",\"end\":\"\"";
    }
    json += ",\"isDuration\":";
    json += timers[i].isDuration ? "true" : "false";
    json += ",\"duration\":";
    json += String((unsigned long)timers[i].durationSec);
    json += "}";
  }
  json += "]";
  server.send(200, "application/json", json);
}

void handleGetLabels() {
  String json = "[";
  for (int i = 0; i < NUM_RELAYS; i++) {
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
  if (!server.hasArg("channel") || !server.hasArg("label")) {
    server.send(400, "text/plain", "Missing args");
    return;
  }
  int ch = server.arg("channel").toInt();
  if (ch < 1 || ch > NUM_RELAYS) {
    server.send(400, "text/plain", "Invalid channel");
    return;
  }
  String label = server.arg("label");
  if (label.isEmpty()) {
    server.send(400, "text/plain", "Empty label");
    return;
  }
  snprintf(relayLabels[ch - 1], sizeof(relayLabels[ch - 1]), "%s",
           label.c_str());
  preferences.begin("relay-labels", false);
  preferences.putString(("label" + String(ch)).c_str(), label);
  preferences.end();

  char msg[LOG_LINE_LEN];
  snprintf(msg, sizeof(msg), "Label updated: R%d -> %s", ch, label.c_str());
  logEvent(msg);
  server.send(200, "text/plain", "OK");
}

void handleSystemInfo() {
  unsigned long up = millis() / 1000;
  unsigned long days = up / 86400;
  unsigned long hours = (up % 86400) / 3600;
  unsigned long mins = (up % 3600) / 60;
  unsigned long secs = up % 60;

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
  json += ",\"uptimeSeconds\":" + String(up);
  char ub[48];
  snprintf(ub, sizeof(ub), "%lud %luh %lum %lus", days, hours, mins, secs);
  json += ",\"uptimeStr\":\"" + String(ub) + "\"";
  json += ",\"wifiRSSI\":" + String(WiFi.RSSI());
  json += ",\"wifiSSID\":\"" + WiFi.SSID() + "\"";
  json += ",\"ipAddress\":\"" + WiFi.localIP().toString() + "\"";
  json += ",\"macAddress\":\"" + WiFi.macAddress() + "\"";
  json += ",\"apMode\":";
  json += isApMode ? "true" : "false";
  json += "}";
  server.send(200, "application/json", json);
}

// --- OTA ---
void handleDoUpdate() {
  server.sendHeader("Connection", "close");
  if (Update.hasError()) {
    server.send(500, "application/json",
                "{\"success\":false,\"error\":\"Update failed\"}");
  } else {
    server.send(200, "application/json",
                "{\"success\":true,\"message\":\"Rebooting...\"}");
    delay(1000);
    ESP.restart();
  }
}

void handleDoUpdateUpload() {
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    char buf[LOG_LINE_LEN];
    snprintf(buf, sizeof(buf), "OTA Start: %s", upload.filename.c_str());
    logEvent(buf);
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      snprintf(buf, sizeof(buf), "OTA Error: %s", Update.errorString());
      logEvent(buf);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      char buf[LOG_LINE_LEN];
      snprintf(buf, sizeof(buf), "OTA Write Error: %s", Update.errorString());
      logEvent(buf);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    char buf[LOG_LINE_LEN];
    if (Update.end(true)) {
      snprintf(buf, sizeof(buf), "OTA Success: %u bytes",
               (unsigned)upload.totalSize);
      logEvent(buf);
    } else {
      snprintf(buf, sizeof(buf), "OTA Error: %s", Update.errorString());
      logEvent(buf);
    }
  }
}

void handleReboot() {
  if (!checkAuth())
    return;
  server.send(200, "application/json", "{\"success\":true,\"message\":\"Rebooting...\"}");
  delay(1000);
  ESP.restart();
}

// --- Home Assistant API ---
void handleHA() {
  if (server.hasArg("channel") && server.hasArg("state")) {
    int ch = server.arg("channel").toInt();
    String stateStr = server.arg("state");
    stateStr.toUpperCase();
    if (ch >= 1 && ch <= NUM_RELAYS &&
        (stateStr == "ON" || stateStr == "OFF")) {
      setRelay(ch - 1, stateStr == "ON", "HA API");
    } else {
      server.send(400, "application/json", "{\"error\":\"bad channel/state\"}");
      return;
    }
  }
  char json[NUM_RELAYS * 12 + 4];
  size_t pos = 0;
  json[pos++] = '{';
  for (int i = 0; i < NUM_RELAYS; i++) {
    if (i)
      json[pos++] = ',';
    pos += snprintf(json + pos, sizeof(json) - pos, "\"r%d\":\"%s\"", i + 1,
                    relayState[i] ? "ON" : "OFF");
  }
  json[pos++] = '}';
  json[pos] = '\0';
  server.send(200, "application/json", json);
}

// ---------------------------------------------------------------------------
// Timers robustos: disparan aunque se pierda el minuto exacto
// ---------------------------------------------------------------------------

// Minutos absolutos desde medianoche
static int minutesOfDay(const struct tm &t) { return t.tm_hour * 60 + t.tm_min; }

void checkTimers() {
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck < 1000)
    return;
  lastCheck = millis();

  time_t now;
  struct tm info;
  if (time(&now) < 100000)
    return; // Hora sin sincronizar
  localtime_r(&now, &info);
  int nowMin = minutesOfDay(info); // 0..1439

  static int lastCheckedMin = -1;
  if (nowMin == lastCheckedMin)
    return; // Ya evaluado este minuto (los durations van aparte)

  // Rango (exclusivo) de minutos transcurridos desde el ultimo chequeo:
  // si hubo un bloqueo largo o reinicio, igualmente dispara los timers cuyo
  // minuto de inicio cayo dentro de ese intervalo.
  int prevMin = (lastCheckedMin == -1) ? nowMin : lastCheckedMin;
  lastCheckedMin = nowMin;

  // Simplificacion correcta: comprobar cada timer contra el intervalo
  for (int i = 0; i < NUM_RELAYS; i++) {
    if (!timers[i].enabled)
      continue;
    int startTarget = timers[i].startH * 60 + timers[i].startM;

    // START: disparar si el minuto objetivo cae dentro de (prev, now]
    int dStart = nowMin - startTarget;
    if (dStart >= 0 && dStart < (nowMin - prevMin + 1)) {
      if (timers[i].isDuration) {
        if (!relayState[i]) {
          setRelay(i, true, "Timer Duration Start");
          lastOnTimeMs[i] = millis64();
        }
      } else if (!relayState[i]) {
        setRelay(i, true, "Timer Trigger");
      }
    }

    // END (solo modo horario diario)
    if (!timers[i].isDuration) {
      int endTarget = timers[i].endH * 60 + timers[i].endM;
      int dEnd = nowMin - endTarget;
      if (dEnd >= 0 && dEnd < (nowMin - prevMin + 1)) {
        if (relayState[i])
          setRelay(i, false, "Timer Trigger");
      }
    }
  }
}

void checkDurations() {
  for (int i = 0; i < NUM_RELAYS; i++) {
    if (timers[i].enabled && timers[i].isDuration && relayState[i] &&
        lastOnTimeMs[i] > 0) {
      uint64_t elapsed = millis64() - lastOnTimeMs[i];
      if (elapsed >= (uint64_t)timers[i].durationSec * 1000ULL) {
        setRelay(i, false, "Timer Duration End");
        lastOnTimeMs[i] = 0;
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

// ---------------------------------------------------------------------------
// Setup / Loop
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  pinMode(VIS_STATUS_LED, OUTPUT);
  digitalWrite(VIS_STATUS_LED, LOW);

  // Defaults de etiquetas
  for (int i = 0; i < NUM_RELAYS; i++)
    snprintf(relayLabels[i], sizeof(relayLabels[i]), "Relay %d", i + 1);

  // Pines + estado persistido
  preferences.begin("relay-states", true);
  for (int i = 0; i < NUM_RELAYS; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
    relayState[i] = preferences.getInt(("r" + String(i)).c_str(), 0) == 1;
    digitalWrite(RELAY_PINS[i], relayState[i] ? HIGH : LOW);
  }
  preferences.end();

  // Etiquetas persistidas
  preferences.begin("relay-labels", true);
  for (int i = 1; i <= NUM_RELAYS; i++) {
    String saved = preferences.getString(("label" + String(i)).c_str(), "");
    if (!saved.isEmpty())
      snprintf(relayLabels[i - 1], sizeof(relayLabels[i - 1]), "%s",
               saved.c_str());
  }
  preferences.end();

  // Timers persistidos
  preferences.begin("timers", true);
  for (int i = 1; i <= NUM_RELAYS; i++) {
    String v = preferences.getString(("t" + String(i)).c_str(), "");
    if (v.isEmpty())
      continue;
    int idx = i - 1;
    // Formato: startH,startM,endH,endM,enabled,isDuration,durationSec
    int vals[7] = {-1, -1, -1, -1, 1, 0, 0};
    int vi = 0, from = 0;
    while (vi < 7 && from <= (int)v.length()) {
      int comma = v.indexOf(',', from);
      String part = (comma == -1) ? v.substring(from) : v.substring(from, comma);
      if (part.length())
        vals[vi] = part.toInt();
      if (comma == -1)
        break;
      from = comma + 1;
      vi++;
    }
    timers[idx].startH = vals[0];
    timers[idx].startM = vals[1];
    timers[idx].endH = vals[2];
    timers[idx].endM = vals[3];
    timers[idx].enabled = vals[4] == 1;
    timers[idx].isDuration = vals[5] == 1;
    timers[idx].durationSec = (uint32_t)max(vals[6], 0);

    if (timers[idx].startH >= 0 && timers[idx].startH <= 23) {
      char msg[LOG_LINE_LEN];
      snprintf(msg, sizeof(msg),
               "Timer Loaded %s: %02d:%02d-%02d:%02d%s", relayLabels[idx],
               timers[idx].startH, timers[idx].startM, timers[idx].endH,
               timers[idx].endM, timers[idx].enabled ? " [ON]" : " [OFF]");
      logEvent(msg);
    }
  }
  preferences.end();

  // WiFi (modulo dedicado: conexion, NTP, mDNS o fallback AP)
  wifiSetup(preferences, isApMode, DEFAULT_AP_SSID, DEFAULT_AP_PASSWORD);

  // Rutas
  server.on("/", HTTP_GET, handleRoot);
  server.on("/toggle", HTTP_GET, handleToggle);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/logs", HTTP_GET, handleLogs);
  server.on("/set_time", HTTP_GET, handleSetTime);
  server.on("/get_time", HTTP_GET, handleGetTime);
  server.on("/set_timer", HTTP_GET, handleSetTimer);
  server.on("/clear_timer", HTTP_GET, handleClearTimer);
  server.on("/get_timers", HTTP_GET, handleGetTimers);
  server.on("/get_labels", HTTP_GET, handleGetLabels);
  server.on("/set_label", HTTP_GET, handleSetLabel);
  server.on("/api/ha", HTTP_GET, handleHA);
  server.on("/api/ha", HTTP_POST, handleHA);
  // WiFi (con auth)
  server.on("/scan", HTTP_GET, [] {
    if (checkAuth())
      handleScan();
  });
  server.on("/save_wifi", HTTP_POST, [] {
    if (checkAuth())
      handleSaveWiFi();
  });
  server.on("/reset_wifi", HTTP_POST, [] {
    if (checkAuth())
      handleResetWiFi();
  });
  // Sistema y OTA (con auth)
  server.on("/system_info", HTTP_GET, handleSystemInfo);
  server.on("/do_update", HTTP_POST, [] {
    if (!checkAuth())
      return;
    handleDoUpdate();
  }, handleDoUpdateUpload);
  server.on("/reboot", HTTP_POST, handleReboot);

  server.begin();

  char msg[LOG_LINE_LEN];
  snprintf(msg, sizeof(msg), "System Started - v%s%s", APP_VERSION,
           isApMode ? " (AP mode)" : "");
  logEvent(msg);
}

void loop() {
  server.handleClient();
  checkTimers();
  checkDurations();
  updateLed();
  checkWifiReconnect();
}
