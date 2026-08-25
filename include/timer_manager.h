#ifndef TIMER_MANAGER_H
#define TIMER_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include "relay_manager.h"

struct Timer {
    int startH = -1;
    int startM = -1;
    int endH = -1;
    int endM = -1;
    bool enabled = false;
    bool isDuration = false;
    int durationSec = 0;
};

class TimerManager {
public:
    TimerManager();
    void begin(Preferences &prefs);
    void setTimer(int channel, Timer t, Preferences &prefs);
    void clearTimer(int channel, Preferences &prefs);
    String getTimersJSON() const;
    
    void check(RelayManager &rm, Preferences &prefs, void (*logCb)(const char*));

private:
    Timer timers[8];
    unsigned long lastOnTime[8];
};

extern TimerManager timerManager;

#endif