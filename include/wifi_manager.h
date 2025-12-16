#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <Preferences.h>

void wifiSetup(Preferences &preferences, bool &isApMode, const char *apSSID, const char *apPassword);
void handleScan();
void handleSaveWiFi();
void handleResetWiFi();

#endif // WIFI_MANAGER_H
