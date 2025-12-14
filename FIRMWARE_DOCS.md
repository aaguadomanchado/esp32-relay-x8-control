# Documentación Técnica del Firmware ESP32 Relay X8

## Visión General

Este firmware está diseñado para controlar una placa de 8 relés basada en ESP32 (ESP32-WROOM-32E). Proporciona una interfaz web completa para el control manual, monitorización en tiempo real, programación de temporizadores y configuración del sistema.

## Estructura del Proyecto

- `src/main.cpp`: Lógica principal del firmware, configuración de hardware, servidor web y gestión de tareas.
- `include/index_html.h`: Contiene el código fuente HTML/CSS/JS de la interfaz web, almacenado en memoria flash (PROGMEM).
- `include/pinout.h`: Definiciones de pines para el hardware específico.
- `platformio.ini`: Configuración del entorno de compilación y dependencias.

## Características Principales

### 1. Gestión WiFi
- **Modo Punto de Acceso (AP)**: Si no hay configuración WiFi guardada o falla la conexión.
  - SSID: `ESP32-Relay-X8`
  - Pass: `12345678`
  - IP: `192.168.4.1`
- **Modo Estación (STA)**: Se conecta a una red WiFi configurada.
- **mDNS**: Publica el host como `esp32.local`.

### 2. Control de Relés
- Control independiente para 8 canales.
- Lógica Active-HIGH (High = ON, Low = OFF).
- Estado persistente en RAM (se reinicia al apagar, por seguridad).

### 3. Temporizadores
- Cada relé tiene un temporizador diario programable asociado.
- Configuración: Hora de inicio (ON) y Hora de fin (OFF).
- Persistencia: Se guardan en memoria no volátil (NVS).

### 4. Personalización
- Etiquetas personalizables para cada uno de los 8 relés.
- Persistencia en NVS.

- **Sincronización de Hora**:
- Cliente NTP automático (`pool.ntp.org`).
- Sincronización manual desde el navegador (como fallback).
- Zona horaria configurada: UTC+1.

### 6. Indicadores de Estado (LED)
- **GPIO 23 (LED Integrado)**:
  - **Parpadeo Rápido (100ms)**: Intentando conectar a WiFi / Iniciando.
  - **Fijo (ON)**: Conexión WiFi establecida y sistema listo.
  - **Parpadeo Lento (1000ms)**: Modo Punto de Acceso (AP) activo por fallo de conexión.

## API Reference (Endpoints HTTP)

El servidor web escucha en el puerto 80. Todas las respuestas son en texto plano o JSON.

### Control y Estado

| Endpoint | Método | Parámetros | Descripción |
|----------|--------|------------|-------------|
| `/` | GET | - | Interfaz Web Principal (Gzipped HTML) |
| `/status` | GET | - | Estado actual de relés. Retorna JSON ARRAY `[0,1,0,...]` |
| `/toggle` | GET | `channel` (1-8), `state` (0/1) | Cambia estado de relé. Retorna "OK" o error. |
| `/logs` | GET | - | Buffer de logs del sistema (últimos 2000 caracteres) |

### Gestión de Etiquetas

| Endpoint | Método | Parámetros | Descripción |
|----------|--------|------------|-------------|
| `/get_labels` | GET | - | Lista de nombres. Retorna JSON ARRAY `["Luz 1",...]` |
| `/set_label` | GET | `channel` (1-8), `label` (String) | Guarda nuevo nombre. Retorna "OK". |

### Integración Home Assistant

| Endpoint | Método | Parámetros | Descripción |
|----------|--------|------------|-------------|
| `/api/ha` | POST/GET | `channel` (1-8), `state` (ON/OFF) | Optimizado para `RESTful Switch`. Retorna JSON `{"r1":"OFF", "r2":"ON"...}` con el estado de todos los relés. |

### Temporizadores

| Endpoint | Método | Parámetros | Descripción |
|----------|--------|------------|-------------|
| `/get_timers` | GET | - | JSON Array con configs `[{enabled, start, end},...]` |
| `/set_timer` | GET | `channel`, `start` (HH:MM), `end` (HH:MM), `enabled` (0/1) | Activa/Desactiva y guarda temporizador. |
| `/clear_timer`| GET | `channel` | Borra configuración de temporizador y lo desactiva. |

### Sistema y Red

| Endpoint | Método | Parámetros | Descripción |
|----------|--------|------------|-------------|
| `/get_time` | GET | - | Hora sistema. JSON `{epoch, year, str}` |
| `/set_time` | GET | `epoch` (Unix timestamp) | Establece hora manualmente. |
| `/scan` | GET | - | Escanea redes WiFi. Retorna JSON Array. |
| `/save_wifi` | GET | `ssid`, `pass` | Guarda credenciales y reinicia. |
| `/reset_wifi` | GET | - | Borra credenciales y reinicia. |

## Persistencia de Datos (NVS)

El firmware utiliza la librería `Preferences` para guardar configuración en la partición NVS del ESP32.

| Espacio de Nombres | Clave | Tipo | Contenido |
|-------------------|-------|------|-----------|
| `wifi-config` | `ssid` | String | Nombre de red WiFi |
| `wifi-config` | `pass` | String | Contraseña WiFi |
| `relay-labels` | `label{N}` | String | Nombre personalizado para canal N (1-8) |
| `timers` | `t{N}` | String | Configuración temporizador: "startH,startM,endH,endM,enabled" |

## Lógica Interna

- **Loop Principal**:
  - Manejo de clientes web (`server.handleClient()`).
  - Verificación de temporizadores (`checkTimers()`) cada segundo.
- **Sistema de Logs**: Buffer circular en memoria RAM de 2KB para visualización web.
- **Web Interface**:
  - Single Page Application (SPA) embebida.
  - Actualización periódica cada 2s (`/status`).
  - Actualización de logs cada 1s (`/logs`).
  - Diseño Responsive (Mobile-First).

## Dependencias

- **Arduino Core for ESP32**
- **Libraries**:
  - `WiFi`
  - `WebServer`
  - `ESPmDNS`
  - `Preferences`

## Flujo de Compilación

1. `index_html.h` contiene el frontend minificado/optimizado.
2. `Preferences` gestiona el almacenamiento no volátil.
3. El orden de includes en `main.cpp` es crítico (`WiFi.h` antes de `WebServer.h`).
