#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include <WebServer.h>

void wifiSetup(Preferences &prefs, bool &isApMode, const char *apSSID, const char *apPassword);
void handleScan();
void handleSaveWiFi();
void handleResetWiFi();

// Reconexión periódica en loop(); llamar cada iteración.
void checkWifiReconnect();

#endif // WIFI_MANAGER_H
