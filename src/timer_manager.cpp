#include "timer_manager.h"
#include <time.h>

TimerManager timerManager;

TimerManager::TimerManager() {
    for (int i = 0; i < 8; i++) lastOnTime[i] = 0;
}

void TimerManager::begin(Preferences &prefs) {
    for (int i = 1; i <= 8; i++) {
        String value = prefs.getString(("t" + String(i)).c_str(), "");
        if (value != "") {
            int idx = i - 1;
            int c1 = value.indexOf(',');
            int c2 = value.indexOf(',', c1 + 1);
            int c3 = value.indexOf(',', c2 + 1);
            int c4 = value.indexOf(',', c3 + 1);
            int c5 = value.indexOf(',', c4 + 1);
            
            timers[idx].startH = value.substring(0, c1).toInt();
            timers[idx].startM = value.substring(c1 + 1, c2).toInt();
            timers[idx].endH = value.substring(c2 + 1, c3).toInt();
            timers[idx].endM = value.substring(c3 + 1, c4).toInt();
            timers[idx].enabled = value.substring(c4 + 1, c5).toInt() == 1;
            
            int c6 = value.indexOf(',', c5 + 1);
            if (c5 > 0) {
                timers[idx].isDuration = value.substring(c5 + 1, c6).toInt() == 1;
                timers[idx].durationSec = value.substring(c6 + 1).toInt();
            }
        }
    }
}

void TimerManager::setTimer(int channel, Timer t, Preferences &prefs) {
    if (channel < 1 || channel > 8) return;
    timers[channel - 1] = t;
    
    String value = String(t.startH) + "," + String(t.startM) + "," + 
                   String(t.endH) + "," + String(t.endM) + "," + 
                   String(t.enabled ? 1 : 0) + "," + 
                   String(t.isDuration ? 1 : 0) + "," + String(t.durationSec);
    
    prefs.begin("timers", false);
    prefs.putString(("t" + String(channel)).c_str(), value);
    prefs.end();
}

void TimerManager::clearTimer(int channel, Preferences &prefs) {
    if (channel < 1 || channel > 8) return;
    int idx = channel - 1;
    timers[idx].enabled = false;
    timers[idx].startH = -1;
    
    prefs.begin("timers", false);
    prefs.remove(("t" + String(channel)).c_str());
    prefs.end();
}

String TimerManager::getTimersJSON() const {
    String json = "[";
    for (int i = 0; i < 8; i++) {
        json += "{";
        json += "\"enabled\":" + String(timers[i].enabled ? "true" : "false");
        if (timers[i].startH != -1) {
            char buf[32];
            sprintf(buf, ",\"start\":\"%02d:%02d\",\"end\":\"%02d:%02d\"", timers[i].startH, timers[i].startM, timers[i].endH, timers[i].endM);
            json += buf;
        } else {
            json += ",\"start\":\"\",\"end\":\"\"";
        }
        json += ",\"isDuration\":" + String(timers[i].isDuration ? "true" : "false");
        json += ",\"duration\":" + String(timers[i].durationSec);
        json += "}";
        if (i < 7) json += ",";
    }
    json += "]";
    return json;
}

void TimerManager::check(RelayManager &rm, Preferences &prefs, void (*logCb)(const char*)) {
    time_t now;
    struct tm info;
    if (time(&now) < 100000) return;
    localtime_r(&now, &info);

    static int lastMin = -1;
    if (info.tm_min == lastMin) {
        // Aún así chequeamos duraciones cada segundo
        for (int i = 0; i < 8; i++) {
            if (timers[i].enabled && timers[i].isDuration && rm.getRelayState(i) && lastOnTime[i] > 0) {
                if (millis() - lastOnTime[i] >= (unsigned long)timers[i].durationSec * 1000) {
                    rm.setRelay(i + 1, false, prefs);
                    lastOnTime[i] = 0;
                    char buf[64];
                    snprintf(buf, sizeof(buf), "Timer Duration End: %s OFF", rm.getLabel(i));
                    logCb(buf);
                }
            }
        }
        return;
    }
    lastMin = info.tm_min;

    for (int i = 0; i < 8; i++) {
        if (!timers[i].enabled) continue;

        if (info.tm_hour == timers[i].startH && info.tm_min == timers[i].startM) {
            if (!rm.getRelayState(i)) {
                rm.setRelay(i + 1, true, prefs);
                if (timers[i].isDuration) lastOnTime[i] = millis();
                char buf[128];
                snprintf(buf, sizeof(buf), "Timer Trigger: %s ON", rm.getLabel(i));
                logCb(buf);
            }
        }

        if (!timers[i].isDuration && info.tm_hour == timers[i].endH && info.tm_min == timers[i].endM) {
            if (rm.getRelayState(i)) {
                rm.setRelay(i + 1, false, prefs);
                char buf[128];
                snprintf(buf, sizeof(buf), "Timer Trigger: %s OFF", rm.getLabel(i));
                logCb(buf);
            }
        }
    }
}