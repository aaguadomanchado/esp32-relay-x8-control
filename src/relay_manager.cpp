#include "relay_manager.h"

RelayManager relayManager;

RelayManager::RelayManager() {
    for (int i = 0; i < 8; i++) {
        relayState[i] = false;
        relayLabels[i] = ""; // Se asignarán en begin()
    }
}

void RelayManager::begin(Preferences &prefs) {
    // Inicializar Pines
    for (int i = 0; i < NUM_RELAYS; i++) {
        pinMode(RELAY_PINS[i], OUTPUT);
        // Restaurar estado
        int saved = prefs.getInt(("r" + String(i)).c_str(), 0);
        relayState[i] = (saved == 1);
        digitalWrite(RELAY_PINS[i], relayState[i] ? HIGH : LOW);
    }

    // Cargar Etiquetas
    static const char* defaultLabels[8] = {"Relay 1", "Relay 2", "Relay 3", "Relay 4", "Relay 5", "Relay 6", "Relay 7", "Relay 8"};
    for (int i = 1; i <= 8; i++) {
        String saved = prefs.getString(("label" + String(i)).c_str(), "");
        if (saved != "") {
            snprintf(labelStore[i - 1], sizeof(labelStore[i - 1]), "%s", saved.c_str());
            relayLabels[i - 1] = labelStore[i - 1];
        } else {
            relayLabels[i - 1] = defaultLabels[i - 1];
        }
    }
}

void RelayManager::setRelay(int channel, bool state, Preferences &prefs) {
    if (channel < 1 || channel > 8) return;
    int idx = channel - 1;
    relayState[idx] = state;
    digitalWrite(RELAY_PINS[idx], state ? HIGH : LOW);
    
    prefs.begin("relay-states", false);
    prefs.putInt(("r" + String(idx)).c_str(), state ? 1 : 0);
    prefs.end();
}

bool RelayManager::getRelayState(int idx) const {
    if (idx < 0 || idx > 7) return false;
    return relayState[idx];
}

const char* RelayManager::getLabel(int idx) const {
    if (idx < 0 || idx > 7) return "";
    return relayLabels[idx];
}

void RelayManager::setLabel(int channel, const char* label, Preferences &prefs) {
    if (channel < 1 || channel > 8) return;
    int idx = channel - 1;
    snprintf(labelStore[idx], sizeof(labelStore[idx]), "%s", label);
    relayLabels[idx] = labelStore[idx];
    
    prefs.begin("relay-labels", false);
    prefs.putString(("label" + String(channel)).c_str(), label);
    prefs.end();
}

String RelayManager::getStatusJSON() const {
    String json = "[";
    for (int i = 0; i < 8; i++) {
        json += String(relayState[i]);
        if (i < 7) json += ",";
    }
    json += "]";
    return json;
}