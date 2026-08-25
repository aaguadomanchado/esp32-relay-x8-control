# ESP32 Relay Board x8 Project
http://www.alejandroaguado.es/wp-content/uploads/2026/06/Screenshot-2026-06-06-at-02-03-49-ESP8266-_ESP32-WIFI-modulo-de-rele-de-8-canales-esp-12f-_ESP32-WROOM-placa-de-desarrollo-fuente-de-alimentacion-5V_7-28V-AliExpress-502.png

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
5. Upload the firmware (`firmware_full_YYYYMMDD_HHMMSS.bin`) using your preferred flashing tool (e.g., esptool.py or Arduino IDE).
   > **Note**: The firmware filename now includes the compilation timestamp for better version control (e.g., `firmware_full_20260105_033339.bin`).
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
  - NTP Client: Automatically syncs time with internet time servers (Europe/Madrid timezone, DST auto).
  - Manual Sync: Fallback option to set time from browser.
- **Reliability** (v0.9):
  - Automatic WiFi reconnection every 30s if the router drops.
  - Robust timers that still fire if the exact minute is missed.
  - Circular non-destructive log buffer (40 lines) served via `/logs`.
- **Developer Tools**:
  - Web Serial Console: View debug logs directly in the browser (Test-Debug tab).
  - JSON API: Full REST API for integration with other systems (`/status`, `/toggle`, `/api/ha`, etc.).

## Security (v0.9)

- Sensitive endpoints (`/do_update` OTA, `/reboot`, `/save_wifi`, `/reset_wifi`, `/scan`) require an `X-Auth-Token` header. Set your token in `ADMIN_TOKEN` (`src/main.cpp`) before compiling.
- WiFi credentials are **not** stored in the repository anymore. Copy `include/credentials.h.example` to `include/credentials.h` and fill in your values before compiling — it is gitignored.
- All HTTP inputs are validated (channel range, state, timer times, SSID/password lengths).

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for the full version history.

### v0.9 - 2026-08-25
- **Security**: token auth on sensitive endpoints, credentials out of the repo, full input validation, fixed buffer overflow in `/scan`.
- **Reliability**: automatic WiFi reconnect, robust timers, non-destructive circular logs, 64-bit duration clock.
- **Code health**: removed dead modules (`relay_manager`, `timer_manager`, `web_server_handlers`), de-duplicated WiFi logic into `wifi_manager.cpp`, NTP timezone Europe/Madrid.

### v0.8
- **Externalized Credentials**: WiFi AP and STA default credentials moved to `credentials.h` for easy pre-compilation editing.
- **Dynamic Version Display**: Firmware version now dynamically injected into HTML from code variable, ensuring consistency.
- **Timestamped Firmware Builds**: Firmware binaries now include compilation date and time in filename (e.g., `firmware_full_20260105_033339.bin`) for improved version tracking.
- **Full Firmware Image**: Post-build script generates complete ESP32 image including bootloader and partitions, preventing boot loops after full chip erase.

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
 
