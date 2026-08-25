# ESP32 Relay X8 Firmware Technical Documentation

## Overview

This firmware is designed to control an 8-channel relay board based on ESP32 (ESP32-WROOM-32E). It provides a full web interface for manual control, real-time monitoring, timer scheduling, and system configuration.

**Current version:** v0.9 (see [CHANGELOG.md](CHANGELOG.md))

## Project Structure

- `src/main.cpp`: Main firmware logic: hardware setup, web server handlers, timers and log system.
- `src/wifi_manager.cpp`: WiFi connection logic (STA/AP fallback), network scan, credential storage, auto-reconnect.
- `include/wifi_manager.h`: Public interface of the WiFi module.
- `include/index_html.h`: Source code for HTML/CSS/JS web interface, stored in flash memory (PROGMEM).
- `include/pinout.h`: Pin definitions for specific hardware.
- `include/credentials.h.example`: Template for WiFi credentials. Copy to `include/credentials.h` (gitignored) before compiling.
- `platformio.ini`: Build environment configuration.
- `post_build.py`: Post-build script — merges bootloader + partitions + app into a single flashable image.

## Key Features

### 1. WiFi Management
- **Access Point Mode (AP)**: If no WiFi config is saved or connection fails.
  - SSID / Pass: defined in `include/credentials.h` (`DEFAULT_AP_SSID` / `DEFAULT_AP_PASSWORD`).
  - IP: `192.168.4.1`
- **Station Mode (STA)**: Connects to a configured WiFi network.
- **Automatic Reconnection** (v0.9): checks link status every 30 s and retries every 15 s if lost.
- **mDNS**: Publishes host as `esp32.local`.

### 2. Relay Control
- Independent control for 8 channels.
- Active-HIGH Logic (High = ON, Low = OFF).
- Persistent state in NVS (Restores state after power loss).

### 3. Timers
- Each relay has an associated programmable daily timer.
- Two modes: **daily window** (Start ON / End OFF) and **duration** (ON at a time, auto-OFF after N seconds).
- **Robust triggering** (v0.9): fires even if the exact minute is missed (long loop blocks, reboots, OTA uploads).
- **64-bit duration clock**: no overflow at 49 days; manual ON starts the countdown if a duration timer is active.
- Persistence: Saved in Non-Volatile Storage (NVS).

### 4. Customization
- Customizable labels for each of the 8 relays.
- Persistence in NVS.

### 5. Time Synchronization
- Automatic NTP Client (`pool.ntp.org`, `time.google.com`).
- Timezone: **Europe/Madrid** with automatic DST via `configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", ...)`.
- Manual sync via browser (as fallback).

### 6. Status Indicators (LED)
- **GPIO 23 (Integrated LED)**:
  - **Fast Blink (100ms)**: Attempting to connect to WiFi / Booting.
  - **Solid ON**: WiFi connection established and system ready.
  - **Slow Blink (1000ms)**: Access Point (AP) mode active due to connection failure.

## Security

Sensitive endpoints require an authentication header:

```
X-Auth-Token: <ADMIN_TOKEN>
```

`ADMIN_TOKEN` is defined in `src/main.cpp`. If left empty (`""`), auth is disabled.

Protected endpoints: `/do_update` (OTA), `/reboot`, `/save_wifi`, `/reset_wifi`, `/scan`.

All inputs are validated: channel range (1-8), relay state (0/1), timer times (HH/MM ranges), SSID length (1-32) and WPA2 password (8-63).

## API Reference (HTTP Endpoints)

The web server listens on port 80. All responses are plain text or JSON.

### Control and Status

| Endpoint | Method | Parameters | Description |
|----------|--------|------------|-------------|
| `/` | GET | - | Main Web Interface |
| `/status` | GET | - | Current relay status. Returns JSON ARRAY `[0,1,0,...]` |
| `/toggle` | GET | `channel` (1-8), `state` (0/1) | Sets relay state. Returns "OK" or 400 on bad input. |
| `/logs` | GET | `clear` (optional, any value) | Last 40 log lines (**non-destructive** unless `clear=1`) |

### Label Management

| Endpoint | Method | Parameters | Description |
|----------|--------|------------|-------------|
| `/get_labels` | GET | - | List of names. Returns JSON ARRAY `["Light 1",...]` |
| `/set_label` | GET | `channel` (1-8), `label` (String, non-empty) | Saves new name. Returns "OK". |

### Home Assistant Integration

| Endpoint | Method | Parameters | Description |
|----------|--------|------------|-------------|
| `/api/ha` | POST/GET | `channel` (1-8), `state` (ON/OFF) | Optimized for `RESTful Switch`. Returns JSON `{"r1":"OFF", "r2":"ON"...}` with all relay states. Invalid channel/state → HTTP 400. |

### Timers

| Endpoint | Method | Parameters | Description |
|----------|--------|------------|-------------|
| `/get_timers` | GET | - | JSON Array with configs `[{enabled, start, end, isDuration, duration},...]` |
| `/set_timer` | GET | `channel`, `start` (HH:MM), `end` (HH:MM), `enabled` (0/1), `isDuration` (0/1), `duration` (seconds) | Enables/Disables and saves timer. Times validated. |
| `/clear_timer`| GET | `channel` | Clears timer config and disables. |

### System and Network

| Endpoint | Method | Auth | Parameters | Description |
|----------|--------|------|------------|-------------|
| `/get_time` | GET | - | - | System time. JSON `{epoch, str}` |
| `/set_time` | GET | - | `epoch` (Unix timestamp) | Sets time manually. |
| `/system_info` | GET | - | - | Heap, uptime, chip info, WiFi RSSI/IP/MAC, version. |
| `/scan` | GET | ✔ | - | Scans WiFi networks. Returns bounded JSON Array. |
| `/save_wifi` | POST | ✔ | `ssid` (1-32), `pass` (8-63) | Saves credentials and restarts. |
| `/reset_wifi` | POST | ✔ | - | Clears credentials and restarts. |
| `/do_update` | POST | ✔ | firmware binary (multipart) | OTA firmware update. |
| `/reboot` | POST | ✔ | - | Restarts the device. |

## Data Persistence (NVS)

Firmware uses the `Preferences` library to save configuration in the ESP32 NVS partition.

| Namespace | Key | Type | Content |
|-------------------|-------|------|-----------|
| `wifi-config` | `ssid` | String | WiFi Network Name |
| `wifi-config` | `pass` | String | WiFi Password |
| `relay-states` | `r{N}` | Int | Last state of channel N (0/1, 0-indexed) |
| `relay-labels` | `label{N}` | String | Custom name for channel N (1-8) |
| `timers` | `t{N}` | String | Timer config: `"startH,startM,endH,endM,enabled,isDuration,durationSec"` |

## Internal Logic

- **Main Loop**:
  - Web client handling (`server.handleClient()`).
  - Timer check (`checkTimers()`) every second — interval-based, survives missed minutes.
  - Duration countdown check (`checkDurations()`).
  - LED update.
  - WiFi reconnect check (`checkWifiReconnect()`).
- **Log System**: RAM circular buffer of 40 lines × 96 chars. Reading `/logs` does NOT consume it (use `/logs?clear=1` to reset).
- **Web Interface**:
  - Embedded Single Page Application (SPA).
  - Periodic update every 2s (`/status`).
  - Log update every 1s (`/logs`).
  - Responsive Design (Mobile-First).

## Dependencies

- **Arduino Core for ESP32** (espressif32 platform via PlatformIO)
- **Libraries**:
  - `WiFi`
  - `WebServer`
  - `ESPmDNS`
  - `Preferences`
  - `Update` (OTA)

## Building & Flashing

```bash
# Setup (first time)
python3 -m venv .venv && .venv/bin/pip install platformio pyserial

# Build (produces .pio/build/esp32dev/firmware_full_*.bin)
.venv/bin/pio run

# Flash over serial
.venv/bin/pio run -t upload

# Serial monitor
.venv/bin/pio device monitor
```

Before building, create your local credentials:

```bash
cp include/credentials.h.example include/credentials.h
$EDITOR include/credentials.h   # never committed (gitignored)
```
