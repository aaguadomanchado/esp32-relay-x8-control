# Changelog - ESP32 Relay X8 Control

## [0.10] - 2026-08-25
### Added
- **API REST unificada**:
  - `GET /api/relays` — estado completo (id, estado, label, timer activo) en una sola llamada.
  - `PUT /api/relays/{n}` — control por JSON: `{"state": true}` o `{"state": false}`.
- **Estadísticas de uso por canal**: contador de conmutaciones y horas acumuladas de encendido, visibles en `GET /api/stats`. Persistidas en NVS con escritura diferida (cada 60 s solo si hay cambios ≥60 s nuevos, para minimizar desgaste de flash).
- **Modo interlock configurable**: grupos de canales mutuamente excluyentes vía `INTERLOCK_GROUPS` en `main.cpp` (ej. bombas o resistencias que no deben estar simultáneamente ON). Al encender un canal se apagan automáticamente los demás de su grupo.
- **CI con GitHub Actions**: workflow `.github/workflows/build.yml` que compila el firmware en cada push/PR y publica el binario como artefacto.

## [0.9] - 2026-08-25
### Security
- **Autenticación por token** (`X-Auth-Token`) en endpoints sensibles: OTA (`/do_update`), `/reboot`, `/save_wifi`, `/reset_wifi` y `/scan`.
- **Credenciales fuera del repo**: `credentials.h` eliminado del control de versiones (gitignored). Se incluye plantilla `credentials.h.example`.
- **Validación de entradas en todos los endpoints**: canal 1-8, estado 0/1, horas válidas de timers, SSID 1-32 chars y contraseña WPA2 8-63 chars.
- **Fix overflow en `/scan`**: buffer acotado con escape JSON correcto de SSIDs (antes podía escribir fuera de un buffer fijo de 4 KB).

### Added
- **Reconexión automática de WiFi**: chequeo cada 30 s + reintento, el dispositivo se recupera solo si el router cae.
- **Logs circulares no destructivos**: buffer circular de 40 líneas que se conserva al consultar `/logs` (opcional `?clear=1` para vaciar).
- **Timers robustos**: disparan aunque se pierda el minuto exacto (bloqueos del loop, reinicios, subidas OTA).
- **Duraciones sin overflow**: reloj interno de 64 bits (`millis64()`), sin límite práctico de duración.
- **Encendido manual compatible con timers de duración**: encender un relé manualmente arranca la cuenta atrás si tiene timer de duración activo.
- **Zona horaria Europe/Madrid**: NTP con CET/CEST automático vía `configTzTime` (sustituye al offset UTC+1 fijo).

### Changed
- **Código muerto eliminado**: borrados `relay_manager.*`, `timer_manager.*`, `web_server_handlers.*` (no se usaban; `main.cpp` era la implementación real).
- **Duplicación WiFi eliminada**: `main.cpp` ahora usa `wifi_manager.cpp` para conexión, NTP, mDNS y fallback AP.
- **JSON construido con buffers fijos** (`snprintf`) en handlers de estado/timers/HA: menos fragmentación de heap en ejecución 24/7.

### Fixed
- Validación de `state` en `/toggle` (antes aceptaba cualquier entero).
- `/api/ha` ahora responde 400 ante canal/estado inválidos (antes los ignoraba silenciosamente).

## [0.8] - 2026-06-06
### Changed
- Credenciales AP/STA externalizadas a `credentials.h`.
- Versión mostrada dinámicamente en la web (reemplazo de `{{VERSION}}`).
- Binarios con timestamp en el nombre (`firmware_full_YYYYMMDD_HHMMSS.bin`).
- Imagen completa post-build (bootloader + particiones + app) para flasheo desde offset 0x0.

### Security note (retroactivo)
La v0.8 publicó credenciales WiFi reales en `include/credentials.h`. El historial fue purgado y el repo recreado limpio en agosto de 2026. Si clonaste antes, cambia tu contraseña WiFi.

## [0.7] - 2024-05-22
### Added
- Preparación para la nueva versión de desarrollo.
- Estructura base para el seguimiento de cambios (Changelog).

### Changed
- Actualizada versión de la aplicación de 0.6 a 0.7 en `src/main.cpp`.
