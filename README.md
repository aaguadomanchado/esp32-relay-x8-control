# ESP32 Relay Board x8 Project

![ESP32 Relay Board x8](https://devices.esphome.io/assets/images/image-42531fa600cef0d4854d8601aaf12503.jpg)

**[🛒 Buy on AliExpress](https://www.aliexpress.us/item/3256802045374301.html)**

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
5. Upload the firmware (`firmware/firmware.bin`) using your preferred flashing tool (e.g., esptool.py or Arduino IDE).
6. Upload the web interface (`data` folder) using "Upload Filesystem Image".

## Quick Home Assistant Integration

Add the following to your `configuration.yaml` (adjust the IP and repeat for all 8 channels):

```yaml
switch:
  - platform: rest
    name: "Relay 1"
    resource: "http://<BOARD_IP>/api/ha"
    method: post
    body_on: 'channel=1&state=ON'
    body_off: 'channel=1&state=OFF'
    is_on_template: '{{ value_json.r1 == "ON" }}'
    headers:
      Content-Type: application/x-www-form-urlencoded
```

## Firmware Features

This project includes a comprehensive firmware with a Web Interface for full control:

- **Web Interface (UI)**: Responsive, mobile-friendly dashboard stored in program memory (PROGMEM).
- **8-Channel Control**:
  - Manual ON/OFF toggle for each relay.
  - Real-time status monitoring with virtual LEDs.
  - Custom Labels: Assign names to each channel (e.g., "Garden Light", "Pump").
- **Programmable Timers**:
  - Independent Daily Timer for each relay (Start time - End time).
  - **Duration Timers**: Set relays to turn ON at a specific time and automatically turn OFF after a defined duration (hours, minutes, seconds).
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

## Changelog

### v0.5
- **New Feature**: Advanced Duration Timers - Relays can be set to turn ON at a specific time and automatically turn OFF after a custom duration (up to 24 hours).
- **UI Improvements**: Redesigned timer configuration with radio buttons for mode selection, compact input fields (limited to 2 digits), and inline feedback messages for save actions.
- **Validation**: Input validation for duration fields (hours 0-24, minutes/seconds 0-59) with user-friendly error messages.
- **Persistence**: Duration timer settings are fully saved and restored.

### v0.3
- **New Feature**: Specific HTTP API for Home Assistant (`/api/ha`).
- **Persistence**: Relay states are saved and restored after reboot or power loss.
- **Improved UI**: Timer inputs are visually disabled when the timer is switched off.

### v0.2
- **New Feature**: Added status LED indicators (GPIO 23).
  - **Fast Blink**: Connecting to WiFi.
  - **Solid ON**: Connected to WiFi.
  - **Slow Blink**: AP Mode active.
- **UI Update**: Version number displayed in the header.

### v0.1
- **Initial Release**: Basic functionality, relay control, timers, and Web UI.
 
