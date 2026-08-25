#ifndef PINOUT_H
#define PINOUT_H

#include <Arduino.h>

// Relays
#define VIS_RELAY_1 32
#define VIS_RELAY_2 33
#define VIS_RELAY_3 25
#define VIS_RELAY_4 26
#define VIS_RELAY_5 27
#define VIS_RELAY_6 14
#define VIS_RELAY_7 12
#define VIS_RELAY_8 13

// Indicators
#define VIS_STATUS_LED 23

// Buttons
#define VIS_BUTTON_BOOT 0

// Array for easier iteration
const uint8_t RELAY_PINS[] = {VIS_RELAY_1, VIS_RELAY_2, VIS_RELAY_3,
                              VIS_RELAY_4, VIS_RELAY_5, VIS_RELAY_6,
                              VIS_RELAY_7, VIS_RELAY_8};

const int NUM_RELAYS = sizeof(RELAY_PINS) / sizeof(RELAY_PINS[0]);

#endif // PINOUT_H
