# ESP32 Relay Board x8 Project

This project is designed for the **ESP32 Relay Board x8** based on the ESP32-WROOM-32E module.

## Hardware Specifications

- **Microcontroller:** ESP32-WROOM-32E
- **Connectivity:** Wi-Fi + Bluetooth
- **Relays:** 8x Optocoupled Relays (10A max)

### Power Supply
- **5V DC:** Via 5V and GND pins.
- **7-30V DC:** Via 7-30V and GND pins.
> **WARNING:** Do NOT connect 24V AC directly. Only DC voltage is supported.

### Pinout Mapping

| Component | Channel | GPIO | Logic |
|-----------|---------|------|-------|
| Relay | 1 | 32 | Active HIGH |
| Relay | 2 | 33 | Active HIGH |
| Relay | 3 | 25 | Active HIGH |
| Relay | 4 | 26 | Active HIGH |
| Relay | 5 | 27 | Active HIGH |
| Relay | 6 | 14 | Active HIGH |
| Relay | 7 | 12 | Active HIGH |
| Relay | 8 | 13 | Active HIGH |
| **Status LED** | - | **23** | - |
| **Boot Button** | - | **0** | Active LOW |

## Programming Instructions

To flash firmware to this board, you need a USB-TTL Adapter (e.g., CP2102, CH340).

**Connections:**
- USB TX -> ESP32 RX
- USB RX -> ESP32 TX
- USB GND -> ESP32 GND
- USB 5V -> ESP32 5V (or power externally)

**Flash Mode:**
1. Connect the USB-TTL adapter.
2. Hold down the **001 (Boot)** button (GPIO 0).
3. Press and release the **EN (Reset)** button.
4. Release the **001 (Boot)** button.
5. Upload the firmware.

## Firmware Features

This project includes a comprehensive firmware with a Web Interface for full control:

- **Web Interface (UI)**: Responsive, mobile-friendly dashboard stored in program memory (PROGMEM).
- **8-Channel Control**:
  - Manual ON/OFF toggle for each relay.
  - Real-time status monitoring with virtual LEDs.
  - Custom Labels: Assign names to each channel (e.g., "Garden Light", "Pump").
- **Programmable Timers**:
  - Independent Daily Timer for each relay (Start time - End time).
  - **Enable/Disable Switch**: Toggle timers on/off without losing configuration.
  - Persistence: Timer settings are saved in NVS (Non-Volatile Storage).
- **WiFi Manager**:
  - Scans for available networks via the web UI.
  - Connects to selected WiFi network (Station Mode).
  - Fallback to Access Point (AP) mode if connection fails (`ESP32-Relay-X8` / `12345678`).
- **System Time**:
  - NTP Client: Automatically syncs time with internet time servers.
  - Manual Sync: Fallback option to set time from browser.
- **Developer Tools**:
  - Web Serial Console: View debug logs directly in the browser (Test-Debug tab).
  - JSON API: Full REST API for integration with other systems (`/status`, `/toggle`, etc.).
