#ifndef RELAY_MANAGER_H
#define RELAY_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include "pinout.h"

class RelayManager {
public:
    RelayManager();
    void begin(Preferences &prefs);
    
    void setRelay(int channel, bool state, Preferences &prefs);
    bool getRelayState(int idx) const;
    
    const char* getLabel(int idx) const;
    void setLabel(int channel, const char* label, Preferences &prefs);
    
    String getStatusJSON() const;

private:
    bool relayState[8];
    const char* relayLabels[8];
    char labelStore[8][32];
};

extern RelayManager relayManager;

#endif