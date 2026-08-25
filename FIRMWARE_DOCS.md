# ESP32 Relay X8 Firmware Technical Documentation

## Overview

This firmware is designed to control an 8-channel relay board based on ESP32 (ESP32-WROOM-32E). It provides a full web interface for manual control, real-time monitoring, timer scheduling, and system configuration.

## Project Structure

- `src/main.cpp`: Main firmware logic, hardware setup, web server, and task management.
- `include/index_html.h`: Source code for HTML/CSS/JS web interface, stored in flash memory (PROGMEM).
- `include/pinout.h`: Pin definitions for specific hardware.
- `platformio.ini`: Build environment configuration and dependencies.

## Key Features

### 1. WiFi Management
- **Access Point Mode (AP)**: If no WiFi config is saved or connection fails.
  - SSID: `ESP32-Relay-X8`
  - Pass: `12345678`
  - IP: `192.168.4.1`
- **Station Mode (STA)**: Connects to a configured WiFi network.
- **mDNS**: Publishes host as `esp32.local`.

### 2. Relay Control
- Independent control for 8 channels.
- Active-HIGH Logic (High = ON, Low = OFF).
- Persistent state in NVS (Restores state after power loss).

### 3. Timers
- Each relay has an associated programmable daily timer.
- Configuration: Start Time (ON) and End Time (OFF).
- Persistence: Saved in Non-Volatile Storage (NVS).

### 4. Customization
- Customizable labels for each of the 8 relays.
- Persistence in NVS.

### 5. Time Synchronization
- Automatic NTP Client (`pool.ntp.org`).
- Manual sync via browser (as fallback).
- Configured Timezone: UTC+1.

### 6. Status Indicators (LED)
- **GPIO 23 (Integrated LED)**:
  - **Fast Blink (100ms)**: Attempting to connect to WiFi / Booting.
  - **Solid ON**: WiFi connection established and system ready.
  - **Slow Blink (1000ms)**: Access Point (AP) mode active due to connection failure.

## API Reference (HTTP Endpoints)

The web server listens on port 80. All responses are plain text or JSON.

### Control and Status

| Endpoint | Method | Parameters | Description |
|----------|--------|------------|-------------|
| `/` | GET | - | Main Web Interface (Gzipped HTML) |
| `/status` | GET | - | Current relay status. Returns JSON ARRAY `[0,1,0,...]` |
| `/toggle` | GET | `channel` (1-8), `state` (0/1) | Toggles relay state. Returns "OK" or error. |
| `/logs` | GET | - | System logs buffer (last 2000 chars) |

### Label Management

| Endpoint | Method | Parameters | Description |
|----------|--------|------------|-------------|
| `/get_labels` | GET | - | List of names. Returns JSON ARRAY `["Light 1",...]` |
| `/set_label` | GET | `channel` (1-8), `label` (String) | Saves new name. Returns "OK". |

### Home Assistant Integration

| Endpoint | Method | Parameters | Description |
|----------|--------|------------|-------------|
| `/api/ha` | POST/GET | `channel` (1-8), `state` (ON/OFF) | Optimized for `RESTful Switch`. Returns JSON `{"r1":"OFF", "r2":"ON"...}` with all relay states. |

### Timers

| Endpoint | Method | Parameters | Description |
|----------|--------|------------|-------------|
| `/get_timers` | GET | - | JSON Array with configs `[{enabled, start, end},...]` |
| `/set_timer` | GET | `channel`, `start` (HH:MM), `end` (HH:MM), `enabled` (0/1) | Enables/Disables and saves timer. |
| `/clear_timer`| GET | `channel` | Clears timer config and disables. |

### System and Network

| Endpoint | Method | Parameters | Description |
|----------|--------|------------|-------------|
| `/get_time` | GET | - | System time. JSON `{epoch, year, str}` |
| `/set_time` | GET | `epoch` (Unix timestamp) | Sets time manually. |
| `/scan` | GET | - | Scans WiFi networks. Returns JSON Array. |
| `/save_wifi` | GET | `ssid`, `pass` | Saves credentials and restarts. |
| `/reset_wifi` | GET | - | Clears credentials and restarts. |

## Data Persistence (NVS)

Firmware uses the `Preferences` library to save configuration in the ESP32 NVS partition.

| Namespace | Key | Type | Content |
|-------------------|-------|------|-----------|
| `wifi-config` | `ssid` | String | WiFi Network Name |
| `wifi-config` | `pass` | String | WiFi Password |
| `relay-labels` | `label{N}` | String | Custom name for channel N (1-8) |
| `timers` | `t{N}` | String | Timer config: "startH,startM,endH,endM,enabled" |

## Internal Logic

- **Main Loop**:
  - Web client handling (`server.handleClient()`).
  - Timer Check (`checkTimers()`) every second.
- **Log System**: RAM circular buffer (2KB) for web visualization.
- **Web Interface**:
  - Embedded Single Page Application (SPA).
  - Periodic update every 2s (`/status`).
  - Log update every 1s (`/logs`).
  - Responsive Design (Mobile-First).

## Dependencies

- **Arduino Core for ESP32**
- **Libraries**:
  - `WiFi`
  - `WebServer`
  - `ESPmDNS`
  - `Preferences`

## Compilation Flow

1. `index_html.h` contains the minified/optimized frontend.
2. `Preferences` manages non-volatile storage.
3. Order of includes in `main.cpp` is critical (`WiFi.h` before `WebServer.h`).
