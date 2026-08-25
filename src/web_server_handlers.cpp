#include "web_server_handlers.h"
#include "index_html.h"
#include <Update.h>
#include <WiFi.h>
#include "wifi_manager.h"

extern const char* APP_VERSION;
extern char logBuffer[2048];
extern bool isApMode;

void registerHandlers(WebServer &server, RelayManager &rm, TimerManager &tm, Preferences &prefs, void (*logCb)(const char*)) {

    server.on("/", [&server]() { server.send(200, "text/html", index_html); });

    server.on("/toggle", [&server, &rm, &prefs, logCb]() {
        if (server.hasArg("channel") && server.hasArg("state")) {
            int ch = server.arg("channel").toInt();
            int st = server.arg("state").toInt();
            rm.setRelay(ch, st == 1, prefs);
            char buf[128];
            snprintf(buf, sizeof(buf), "Manual: %s %s", rm.getLabel(ch-1), st ? "ON" : "OFF");
            logCb(buf);
            server.send(200, "text/plain", "OK");
        } else server.send(400);
    });

    server.on("/status", [&server, &rm]() { server.send(200, "application/json", rm.getStatusJSON()); });

    server.on("/logs", [&server]() { 
        server.send(200, "text/plain", logBuffer); 
        logBuffer[0] = '\0'; 
    });

    server.on("/get_timers", [&server, &tm]() { server.send(200, "application/json", tm.getTimersJSON()); });

    server.on("/set_timer", [&server, &tm, &prefs, logCb]() {
        if (server.hasArg("channel") && server.hasArg("start") && server.hasArg("end")) {
            Timer t;
            int ch = server.arg("channel").toInt();
            String start = server.arg("start");
            String end = server.arg("end");
            t.startH = start.substring(0, 2).toInt();
            t.startM = start.substring(3, 5).toInt();
            t.endH = end.substring(0, 2).toInt();
            t.endM = end.substring(3, 5).toInt();
            t.enabled = server.hasArg("enabled") ? server.arg("enabled") == "1" : true;
            t.isDuration = server.arg("isDuration") == "1";
            t.durationSec = server.arg("duration").toInt();
            
            tm.setTimer(ch, t, prefs);
            server.send(200, "text/plain", "OK");
        } else server.send(400);
    });

    server.on("/clear_timer", [&server, &tm, &prefs]() {
        int ch = server.arg("channel").toInt();
        tm.clearTimer(ch, prefs);
        server.send(200, "text/plain", "OK");
    });

    server.on("/system_info", [&server]() {
        String json = "{";
        json += "\"version\":\"" + String(APP_VERSION) + "\"";
        json += ",\"freeHeap\":" + String(ESP.getFreeHeap());
        json += ",\"uptimeSeconds\":" + String(millis() / 1000);
        json += ",\"ipAddress\":\"" + WiFi.localIP().toString() + "\"";
        json += ",\"apMode\":" + String(isApMode ? "true" : "false");
        json += "}";
        server.send(200, "application/json", json);
    });

    // OTA y Reboot
    server.on("/reboot", HTTP_POST, [&server]() {
        server.send(200, "application/json", "{\"success\":true}");
        delay(1000); ESP.restart();
    });

    // WiFi (Redirigir a los ya existentes en wifi_manager.cpp)
    server.on("/scan", handleScan);
    server.on("/save_wifi", handleSaveWiFi);
    server.on("/reset_wifi", handleResetWiFi);
}