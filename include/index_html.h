#ifndef INDEX_HTML_H
#define INDEX_HTML_H

#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Relay Control</title>
  <style>
    :root {
      --bg-color: #121212;
      --card-bg: #1e1e1e;
      --text-color: #e0e0e0;
      --accent-color: #bb86fc;
      --console-bg: #000000;
      --console-text: #00ff00;
      --nav-bg: #2c2c2c;
    }
    
    * { box-sizing: border-box; }
    
    body {
      font-family: 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;
      background-color: var(--bg-color);
      color: var(--text-color);
      display: flex;
      flex-direction: column;
      align-items: center;
      margin: 0;
      padding: 0;
      min-height: 100vh;
    }
    
    h1 { 
      margin: 15px 0; 
      font-weight: 300; 
      letter-spacing: 1px; 
      font-size: clamp(1.5rem, 5vw, 2rem);
      text-align: center;
      padding: 0 10px;
    }

    /* Tabs - Mobile First */
    .tabs { 
      width: 100%; 
      display: flex; 
      background: var(--nav-bg);
      position: sticky;
      top: 0;
      z-index: 100;
      box-shadow: 0 2px 4px rgba(0,0,0,0.3);
    }
    .tab-btn {
      flex: 1; 
      padding: 16px 8px;
      border: none; 
      background: none;
      color: #aaa; 
      font-size: 0.9rem;
      cursor: pointer; 
      transition: all 0.3s;
      border-bottom: 3px solid transparent;
      min-height: 48px;
      font-weight: 500;
    }
    .tab-btn.active { 
      color: var(--accent-color); 
      border-bottom: 3px solid var(--accent-color);
      background: rgba(187, 134, 252, 0.1);
    }
    .tab-btn:hover {
      background: rgba(255,255,255,0.05);
    }
    
    .tab-content { 
      display: none; 
      width: 100%; 
      max-width: 1200px;
      padding: 15px;
    }
    .tab-content.active { 
      display: flex; 
      flex-direction: column; 
      align-items: center; 
    }

    /* Grid - Responsive */
    .grid {
      display: grid;
      grid-template-columns: repeat(auto-fill, minmax(140px, 1fr));
      gap: 12px;
      width: 100%;
      margin-bottom: 20px;
    }
    
    .card {
      background-color: var(--card-bg); 
      border-radius: 12px; 
      padding: 18px;
      display: flex; 
      flex-direction: column; 
      align-items: center;
      box-shadow: 0 4px 6px rgba(0,0,0,0.3);
      transition: transform 0.2s, box-shadow 0.2s;
    }
    
    .card:active {
      transform: scale(0.98);
    }
    
    .label { 
      margin-bottom: 12px; 
      font-size: 1.05em; 
      font-weight: 500;
      text-align: center;
    }
    
    /* Toggle Switch - Larger for mobile */
    .switch {
      position: relative; 
      display: inline-block; 
      width: 64px; 
      height: 38px;
    }
    .switch input { opacity: 0; width: 0; height: 0; }
    .slider {
      position: absolute; 
      cursor: pointer; 
      top: 0; left: 0; right: 0; bottom: 0;
      background-color: #3e3e3e; 
      transition: .3s; 
      border-radius: 38px;
    }
    .slider:before {
      position: absolute; 
      content: ""; 
      height: 30px; 
      width: 30px;
      left: 4px; 
      bottom: 4px; 
      background-color: white; 
      transition: .3s; 
      border-radius: 50%;
    }
    input:checked + .slider { background-color: var(--accent-color); }
    input:checked + .slider:before { transform: translateX(26px); }

    /* Timers - Stack on mobile */
    .timer-row {
      width: 100%; 
      background: var(--card-bg); 
      margin-bottom: 12px;
      padding: 16px; 
      border-radius: 10px; 
      display: flex;
      flex-direction: column;
      gap: 12px;
    }
    .timer-row h3 { 
      margin: 0; 
      font-size: 1.1rem; 
      color: var(--accent-color);
    }
    .timer-controls {
      display: flex;
      flex-wrap: wrap;
      gap: 10px;
      align-items: center;
    }
    .time-input {
      background: #333; 
      border: 1px solid #555; 
      color: white;
      padding: 10px; 
      border-radius: 6px;
      font-size: 1rem;
      min-height: 44px;
      flex: 1;
      min-width: 120px;
    }
    .time-input:disabled {
        background-color: #252525;
        color: #555;
        border-color: #333;
        cursor: not-allowed;
    }
    .btn-save {
      background: var(--accent-color); 
      color: black; 
      border: none;
      padding: 10px 18px; 
      border-radius: 6px; 
      cursor: pointer; 
      font-weight: bold;
      font-size: 1rem;
      min-height: 44px;
      transition: transform 0.2s, opacity 0.2s;
    }
    .btn-save:active {
      transform: scale(0.95);
      opacity: 0.8;
    }

    /* Console */
    #console-container { width: 100%; margin-top: 15px; }
    #console {
      width: 100%; 
      height: 180px;
      background-color: var(--console-bg);
      color: var(--console-text); 
      font-family: 'Courier New', monospace;
      font-size: 0.85rem;
      padding: 12px;
      border-radius: 8px; 
      overflow-y: auto;
      border: 1px solid #333;
      resize: vertical;
    }
    
    /* System Time Card - Responsive */
    .time-card {
      width: 100%;
      background: var(--card-bg);
      border-radius: 12px;
      padding: 16px;
      margin-bottom: 15px;
      display: flex;
      flex-direction: column;
      gap: 12px;
    }
    .time-display {
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 5px;
    }
    .time-controls {
      display: flex;
      gap: 8px;
      flex-wrap: wrap;
    }
    
    /* WiFi Card */
    .wifi-card {
      width: 100%;
      max-width: 500px;
      background: var(--card-bg);
      border-radius: 12px;
      padding: 20px;
    }
    .wifi-card h3 {
      margin-top: 0;
      color: var(--accent-color);
    }
    .wifi-input-group {
      display: flex;
      gap: 8px;
      margin-bottom: 12px;
    }
    #scan-results {
      margin-top: 15px;
      max-height: 300px;
      overflow-y: auto;
    }
    .scan-item {
      padding: 12px;
      border-bottom: 1px solid #333;
      cursor: pointer;
      transition: background 0.2s;
    }
    .scan-item:hover {
      background: rgba(187, 134, 252, 0.1);
    }

    /* Monitor Dashboard */
    .monitor-grid {
      display: grid;
      grid-template-columns: repeat(2, 1fr);
      gap: 12px;
      width: 100%;
      max-width: 900px;
    }
    .status-card {
      background: var(--card-bg);
      border-radius: 12px;
      padding: 20px;
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 15px;
      box-shadow: 0 4px 6px rgba(0,0,0,0.3);
    }
    .status-card h3 {
      margin: 0;
      font-size: 1.2rem;
      color: var(--text-color);
      font-weight: 500;
    }
    .virtual-led {
      width: 80px;
      height: 80px;
      border-radius: 50%;
      background: #2a2a2a;
      box-shadow: inset 0 0 10px rgba(0,0,0,0.5);
      position: relative;
      transition: all 0.3s ease;
    }
    .virtual-led.on {
      background: radial-gradient(circle, #00ff00, #00aa00);
      box-shadow: 0 0 20px #00ff00, 0 0 40px #00ff00, inset 0 0 10px rgba(255,255,255,0.3);
    }
    .virtual-led.off {
      background: radial-gradient(circle, #333, #1a1a1a);
      box-shadow: inset 0 0 10px rgba(0,0,0,0.8);
    }
    .status-label {
      font-size: 1.1rem;
      font-weight: 600;
      text-transform: uppercase;
      letter-spacing: 1px;
    }
    .status-label.on {
      color: #00ff00;
    }
    .status-label.off {
      color: #666;
    }

    /* Media Queries for larger screens */
    @media (min-width: 600px) {
      h1 { margin: 25px 0; }
      .tab-btn { 
        padding: 18px 20px;
        font-size: 1rem;
      }
      .tab-content { padding: 25px; }
      .grid {
        grid-template-columns: repeat(auto-fill, minmax(160px, 1fr));
        gap: 18px;
      }
      .card { padding: 22px; }
      #console { height: 220px; font-size: 0.9rem; }
      .time-card {
        flex-direction: row;
        justify-content: space-between;
        align-items: center;
      }
      .time-display {
        flex-direction: row;
        gap: 15px;
      }
      .monitor-grid {
        grid-template-columns: repeat(auto-fill, minmax(220px, 1fr));
        gap: 20px;
      }
    }

    @media (min-width: 900px) {
      .tab-content { padding: 30px; }
      .grid {
        grid-template-columns: repeat(4, 1fr);
        gap: 20px;
      }
      #console { height: 250px; }
      .monitor-grid {
        grid-template-columns: repeat(4, 1fr);
      }
    }
  </style>
</head>
<body>
  <h1>ESP32 Control <span style="font-size:0.5em; color:#777;">v{{VERSION}}</span></h1>

  <div class="tabs">
    <button class="tab-btn active" onclick="openTab('monitor')">Monitor</button>
    <button class="tab-btn" onclick="openTab('debug')">Test-Debug</button>
    <button class="tab-btn" onclick="openTab('timers')">Temporizadores</button>
    <button class="tab-btn" onclick="openTab('wifi')">WiFi</button>
    <button class="tab-btn" onclick="openTab('labels')">Etiquetas</button>
    <button class="tab-btn" onclick="openTab('system')">Sistema</button>
  </div>

  <!-- Tab 0: Monitor -->
  <div id="monitor" class="tab-content active">
    <div class="monitor-grid" id="monitor-grid"></div>
  </div>

  <!-- Tab 1: Debug -->
  <div id="debug" class="tab-content">
    
    <div class="time-card">
      <div class="time-display">
        <div class="label" style="margin-bottom:0">System Time</div>
        <div id="sys-time" style="color:var(--accent-color); font-size:1.3rem; font-weight:500;">--:--:--</div>
      </div>
      <div class="time-controls">
          <input type="datetime-local" id="manualTime" class="time-input" style="flex:1; min-width:180px;">
          <button class="btn-save" onclick="setManualTime()">Set Time</button>
      </div>
    </div>

    <div class="grid" id="relay-grid"></div>
    
    <div id="console-container">
      <div style="margin-bottom:10px; font-weight:600; font-size:1.05rem;">Monitor Serial Web</div>
      <textarea id="console" readonly></textarea>
    </div>
  </div>

  <!-- Tab 2: Timers -->
  <div id="timers" class="tab-content">
    <div id="timer-list" style="width:100%"></div>
  </div>

  <!-- Tab 3: WiFi -->
  <div id="wifi" class="tab-content">
    <div class="wifi-card">
        <h3>WiFi Configuration</h3>
        <p style="font-size:0.95em; color:#999; margin-bottom:20px;">Connect to your router. Device will restart after saving.</p>
        
        <div class="wifi-input-group">
            <input type="text" id="ssid" placeholder="Network Name (SSID)" class="time-input" style="flex:1;">
            <button class="btn-save" onclick="scanWifi()" style="background:#555; white-space:nowrap;">Scan</button>
        </div>
        
        <input type="password" id="pass" placeholder="Password" class="time-input" style="width:100%; margin-bottom:15px;">
        
        <button class="btn-save" onclick="saveWifi()" style="width:100%; margin-bottom:10px;">Connect & Restart</button>
        <button onclick="resetWifi()" style="background:#b00020; color:white; border:none; padding:12px; border-radius:6px; width:100%; cursor:pointer; font-size:1rem; min-height:44px; font-weight:500;">Reset WiFi Settings</button>

        <div id="scan-results"></div>
    </div>
  </div>

  <!-- Tab 4: Labels -->
  <div id="labels" class="tab-content">
    <div style="width:100%; max-width:600px;">
      <div class="card" style="width:100%; padding:20px; margin-bottom:20px;">
        <h3 style="margin-top:0; color:var(--accent-color);">Etiquetas Personalizadas</h3>
        <p style="font-size:0.95em; color:#999; margin-bottom:20px;">Asigna nombres personalizados a cada relé. Los cambios se guardan automáticamente.</p>
        <div id="label-list" style="width:100%;"></div>
      </div>
    </div>
  </div>

  <!-- Tab 5: System -->
  <div id="system" class="tab-content">
    <div style="width:100%; max-width:700px;">
      <!-- System Info Card -->
      <div class="card" style="width:100%; padding:20px; margin-bottom:20px;">
        <h3 style="margin-top:0; color:var(--accent-color);">📊 Información del Sistema</h3>
        <div id="system-info" style="width:100%;">
          <div style="display:grid; grid-template-columns: repeat(auto-fill, minmax(200px, 1fr)); gap:15px; margin-top:15px;">
            <div class="info-item">
              <span style="color:#999; font-size:0.85rem;">Versión Firmware</span>
              <div id="info-version" style="font-size:1.2rem; color:var(--accent-color); font-weight:600;">--</div>
            </div>
            <div class="info-item">
              <span style="color:#999; font-size:0.85rem;">Modelo Chip</span>
              <div id="info-chip" style="font-size:1rem;">--</div>
            </div>
            <div class="info-item">
              <span style="color:#999; font-size:0.85rem;">CPU Frecuencia</span>
              <div id="info-cpu" style="font-size:1rem;">--</div>
            </div>
            <div class="info-item">
              <span style="color:#999; font-size:0.85rem;">Memoria RAM Libre</span>
              <div id="info-heap" style="font-size:1rem;">--</div>
            </div>
            <div class="info-item">
              <span style="color:#999; font-size:0.85rem;">Flash Total</span>
              <div id="info-flash" style="font-size:1rem;">--</div>
            </div>
            <div class="info-item">
              <span style="color:#999; font-size:0.85rem;">Espacio OTA Libre</span>
              <div id="info-ota-space" style="font-size:1rem;">--</div>
            </div>
            <div class="info-item">
              <span style="color:#999; font-size:0.85rem;">Tiempo Encendido</span>
              <div id="info-uptime" style="font-size:1rem;">--</div>
            </div>
            <div class="info-item">
              <span style="color:#999; font-size:0.85rem;">WiFi RSSI</span>
              <div id="info-rssi" style="font-size:1rem;">--</div>
            </div>
            <div class="info-item">
              <span style="color:#999; font-size:0.85rem;">Dirección IP</span>
              <div id="info-ip" style="font-size:1rem;">--</div>
            </div>
            <div class="info-item">
              <span style="color:#999; font-size:0.85rem;">MAC Address</span>
              <div id="info-mac" style="font-size:1rem;">--</div>
            </div>
          </div>
        </div>
        <button class="btn-save" onclick="loadSystemInfo()" style="margin-top:20px; background:#555;">🔄 Actualizar Info</button>
      </div>

      <!-- OTA Update Card -->
      <div class="card" style="width:100%; padding:20px; margin-bottom:20px;">
        <h3 style="margin-top:0; color:var(--accent-color);">⬆️ Actualización de Firmware (OTA)</h3>
        <p style="font-size:0.95em; color:#999; margin-bottom:20px;">Sube un archivo .bin para actualizar el firmware. El dispositivo se reiniciará automáticamente.</p>
        
        <div style="border:2px dashed #555; border-radius:10px; padding:30px; text-align:center; margin-bottom:15px;" id="drop-zone">
          <div style="font-size:2rem; margin-bottom:10px;">📁</div>
          <p style="margin:0 0 15px 0; color:#999;">Arrastra el archivo .bin aquí o</p>
          <input type="file" id="firmware-file" accept=".bin" style="display:none;" onchange="handleFileSelect(this)">
          <button class="btn-save" onclick="document.getElementById('firmware-file').click()" style="background:#555;">Seleccionar Archivo</button>
        </div>
        
        <div id="file-info" style="display:none; margin-bottom:15px; padding:15px; background:#252525; border-radius:8px;">
          <div style="display:flex; justify-content:space-between; align-items:center;">
            <div>
              <div id="file-name" style="font-weight:600;"></div>
              <div id="file-size" style="color:#999; font-size:0.9rem;"></div>
            </div>
            <button onclick="clearFile()" style="background:none; border:none; color:#f44; cursor:pointer; font-size:1.2rem;">✕</button>
          </div>
        </div>
        
        <div id="progress-container" style="display:none; margin-bottom:15px;">
          <div style="display:flex; justify-content:space-between; margin-bottom:5px;">
            <span>Progreso</span>
            <span id="progress-percent">0%</span>
          </div>
          <div style="background:#333; border-radius:10px; height:20px; overflow:hidden;">
            <div id="progress-bar" style="background:var(--accent-color); height:100%; width:0%; transition:width 0.3s;"></div>
          </div>
          <div id="progress-status" style="margin-top:10px; text-align:center; color:#999;"></div>
        </div>
        
        <button class="btn-save" id="upload-btn" onclick="uploadFirmware()" style="width:100%;" disabled>⬆️ Subir Firmware</button>
      </div>

      <!-- Reboot Card -->
      <div class="card" style="width:100%; padding:20px;">
        <h3 style="margin-top:0; color:var(--accent-color);">🔄 Reiniciar Dispositivo</h3>
        <p style="font-size:0.95em; color:#999; margin-bottom:20px;">Reinicia el ESP32. La configuración se mantiene.</p>
        <button onclick="rebootDevice()" style="background:#b00020; color:white; border:none; padding:12px 24px; border-radius:6px; width:100%; cursor:pointer; font-size:1rem; min-height:44px; font-weight:600;">🔄 Reiniciar Ahora</button>
      </div>
    </div>
  </div>

<script>
  const numRelays = 8;
  const consoleArea = document.getElementById('console');
  let relayLabels = [];
  
  // -- Initialization --
  function init() {
    loadLabels().then(() => {
      createMonitorGrid();
      createCards();
      createTimerRows();
      createLabelInputs();
      
      // Restore last active tab from localStorage
      const savedTab = localStorage.getItem('activeTab');
      if (savedTab) {
        // Remove default active states
        document.querySelectorAll('.tab-content').forEach(el => el.classList.remove('active'));
        document.querySelectorAll('.tab-btn').forEach(el => el.classList.remove('active'));
        
        // Activate saved tab
        const tabContent = document.getElementById(savedTab);
        if (tabContent) {
          tabContent.classList.add('active');
          
          // Activate corresponding button
          const buttons = document.querySelectorAll('.tab-btn');
          buttons.forEach(btn => {
            if (btn.textContent.toLowerCase().includes(savedTab.toLowerCase()) || 
                (savedTab === 'debug' && btn.textContent.includes('Test-Debug')) ||
                (savedTab === 'timers' && btn.textContent.includes('Temporizadores')) ||
                (savedTab === 'labels' && btn.textContent.includes('Etiquetas'))) {
              btn.classList.add('active');
            }
          });
        }
      }
    });
    syncTime();
    loadTimers();
    
    // UI Update Loop
    setInterval(updateStatus, 2000);
    setInterval(fetchLogs, 1000);
    setInterval(updateTimeDisplay, 1000);
  }

  // -- LABELS --
  function loadLabels() {
    return fetch('/get_labels')
      .then(r => r.json())
      .then(data => {
        relayLabels = data;
      })
      .catch(e => {
        // Default labels if fetch fails
        relayLabels = Array.from({length: 8}, (_, i) => `Relay ${i+1}`);
      });
  }

  function createLabelInputs() {
    const list = document.getElementById('label-list');
    list.innerHTML = '';
    for (let i = 1; i <= numRelays; i++) {
      const row = document.createElement('div');
      row.style.marginBottom = '15px';
      row.innerHTML = `
        <label style="display:block; margin-bottom:5px; font-weight:500;">Relé ${i}</label>
        <div style="display:flex; gap:10px;">
          <input type="text" id="labelInput${i}" class="time-input" 
                 style="flex:1;" value="${relayLabels[i-1]}" 
                 placeholder="Nombre del relé ${i}">
          <button class="btn-save" onclick="saveLabel(${i})" style="min-width:100px;">Guardar</button>
        </div>
      `;
      list.appendChild(row);
    }
  }

  function saveLabel(channel) {
    const input = document.getElementById(`labelInput${channel}`);
    const label = input.value.trim() || `Relay ${channel}`;
    
    fetch(`/set_label?channel=${channel}&label=${encodeURIComponent(label)}`)
      .then(r => r.text())
      .then(() => {
        relayLabels[channel - 1] = label;
        // Update all displays
        updateMonitorLabel(channel, label);
        updateDebugLabel(channel, label);
        updateTimerLabel(channel, label);
        logToConsole(`Etiqueta actualizada: ${label}`);
      })
      .catch(e => logToConsole("Error guardando etiqueta"));
  }

  function updateMonitorLabel(channel, label) {
    const card = document.querySelector(`#monitor-grid .status-card:nth-child(${channel}) h3`);
    if (card) card.textContent = label;
  }

  function updateDebugLabel(channel, label) {
    const card = document.querySelector(`#relay-grid .card[data-relay="${channel}"] .label`);
    if (card) card.textContent = label;
  }

  function updateTimerLabel(channel, label) {
    const row = document.querySelector(`#timer-list .timer-row:nth-child(${channel}) h3`);
    if (row) row.textContent = label;
  }

  // -- MONITOR TAB --
  function createMonitorGrid() {
    const grid = document.getElementById('monitor-grid');
    grid.innerHTML = '';
    for (let i = 1; i <= numRelays; i++) {
      const card = document.createElement('div');
      card.className = 'status-card';
      card.innerHTML = `
        <h3>${relayLabels[i-1]}</h3>
        <div class="virtual-led off" id="led${i}"></div>
        <div class="status-label off" id="status${i}">OFF</div>
      `;
      grid.appendChild(card);
    }
  }

  function updateMonitorStatus(data) {
    for (let i = 1; i <= numRelays; i++) {
      const led = document.getElementById(`led${i}`);
      const status = document.getElementById(`status${i}`);
      const isOn = data[i-1] === 1;
      
      if (isOn) {
        led.className = 'virtual-led on';
        status.className = 'status-label on';
        status.textContent = 'ON';
      } else {
        led.className = 'virtual-led off';
        status.className = 'status-label off';
        status.textContent = 'OFF';
      }
    }
  }

  // Time Logic
  function syncTime() {
    fetch('/get_time')
      .then(r => r.json())
      .then(data => {
          if (data.year < 2020) {
               logToConsole("Time invalid. Auto-syncing...");
               forceSync();
          } else {
               logToConsole("Time valid (" + data.str + "). Skipping auto-sync.");
          }
      })
      .catch(e => forceSync()); 
  }

  function forceSync() {
    // Send browser UTC epoch. Valid for both Manual and NTP modes if backend is configured correctly.
    const epoch = Math.floor(Date.now() / 1000);
    fetch(`/set_time?epoch=${epoch}`)
      .then(r => r.text())
      .then(msg => logToConsole("Auto-Sync: " + msg));
  }

  function setManualTime() {
      const val = document.getElementById('manualTime').value;
      if (!val) return alert("Select time first");
      
      const epoch = Math.floor(new Date(val).getTime() / 1000); // Standard JS Epoch
      fetch(`/set_time?epoch=${epoch}`)
        .then(r => r.text())
        .then(msg => logToConsole("Manual Set: " + msg));
  }

  function updateTimeDisplay() {
      fetch('/get_time')
        .then(r => r.json())
        .then(data => {
             document.getElementById('sys-time').innerText = data.str;
        });
  }

  // Load Timers
  function loadTimers() {
      fetch('/get_timers')
        .then(r => r.json())
        .then(data => {
            // data is array of {enabled, start, end, isDuration, duration}
            for(let i=1; i<=numRelays; i++) {
                const t = data[i-1];
                // Set switch state
                const cb = document.getElementById(`timerEnable${i}`);
                if(cb) cb.checked = t.enabled;

                // Set mode
                const dailyRadio = document.querySelector(`input[name="mode${i}"][value="daily"]`);
                const durationRadio = document.querySelector(`input[name="mode${i}"][value="duration"]`);
                if (t.isDuration) {
                  durationRadio.checked = true;
                } else {
                  dailyRadio.checked = true;
                }

                // Set times (backend now returns them even if disabled)
                if(t.start) {
                  document.getElementById(`start${i}`).value = t.start;
                  document.getElementById(`startDur${i}`).value = t.start;
                }
                if(t.end) document.getElementById(`end${i}`).value = t.end;
                
                // Set duration
                const h = Math.floor(t.duration / 3600);
                const m = Math.floor((t.duration % 3600) / 60);
                const s = t.duration % 60;
                document.getElementById(`durH${i}`).value = h;
                document.getElementById(`durM${i}`).value = m;
                document.getElementById(`durS${i}`).value = s;
                
                // Update UI state
                toggleTimerInputs(i);
            }
            logToConsole("Timers Loaded");
        })
        .catch(e => logToConsole("Error loading timers"));
  }

  // TABS
  function openTab(tabName) {
    document.querySelectorAll('.tab-content').forEach(el => el.classList.remove('active'));
    document.querySelectorAll('.tab-btn').forEach(el => el.classList.remove('active'));
    document.getElementById(tabName).classList.add('active');
    
    // Find and activate the corresponding button
    const buttons = document.querySelectorAll('.tab-btn');
    buttons.forEach(btn => {
      if (btn.textContent.toLowerCase().includes(tabName.toLowerCase()) || 
          (tabName === 'debug' && btn.textContent.includes('Test-Debug')) ||
          (tabName === 'timers' && btn.textContent.includes('Temporizadores')) ||
          (tabName === 'labels' && btn.textContent.includes('Etiquetas'))) {
        btn.classList.add('active');
      }
    });
    
    // Save current tab to localStorage
    localStorage.setItem('activeTab', tabName);
  }

  // -- DEBUG TAB --
  function createCards() {
    const grid = document.getElementById('relay-grid');
    grid.innerHTML = '';
    for (let i = 1; i <= numRelays; i++) {
        const card = document.createElement('div');
        card.className = 'card';
        card.setAttribute('data-relay', i);
        card.innerHTML = `
            <div class="label">${relayLabels[i-1]}</div>
            <label class="switch">
                <input type="checkbox" id="relay${i}" onchange="toggleRelay(${i})">
                <span class="slider"></span>
            </label>
        `;
        grid.appendChild(card);
    }
  }

  function toggleRelay(id) {
    const checkbox = document.getElementById(`relay${id}`);
    fetch(`/toggle?channel=${id}&state=${checkbox.checked ? 1 : 0}`)
      .catch(err => {
          checkbox.checked = !checkbox.checked;
          logToConsole("Error: " + err);
      });
  }

  // -- TIMERS TAB --
  function createTimerRows() {
     const list = document.getElementById('timer-list');
     list.innerHTML = '';
     for (let i = 1; i <= numRelays; i++) {
         const row = document.createElement('div');
         row.className = 'timer-row';
         row.innerHTML = `
            <div style="display:flex; justify-content:space-between; align-items:center; width:100%; margin-bottom:10px;">
                <h3 style="margin:0;">${relayLabels[i-1]}</h3>
                <label class="switch" style="transform:scale(0.8);">
                    <input type="checkbox" id="timerEnable${i}" onchange="toggleTimerInputs(${i})">
                    <span class="slider"></span>
                </label>
            </div>
            <div style="margin-bottom:10px;">
              <label><input type="radio" name="mode${i}" value="daily" checked onchange="toggleMode(${i})"> Horario diario</label>
              <label><input type="radio" name="mode${i}" value="duration" onchange="toggleMode(${i})"> Duración</label>
            </div>
            <div class="timer-controls" id="dailyControls${i}">
              <label style="flex:1; min-width:140px;">ON: <input type="time" id="start${i}" class="time-input" style="width:100%;"></label>
              <label style="flex:1; min-width:140px;">OFF: <input type="time" id="end${i}" class="time-input" style="width:100%;"></label>
            </div>
            <div class="timer-controls" id="durationControls${i}" style="display:none; flex-direction: column; gap: 10px;">
              <label style="flex:1; min-width:140px;">Inicio: <input type="time" id="startDur${i}" class="time-input" style="width:100%;"></label>
              <div style="display: flex; gap: 10px; width: 100%;">
                <label style="flex:1; min-width:50px; max-width:60px;">H: <input type="number" id="durH${i}" class="time-input" min="0" max="24" maxlength="2" style="width:100%;"></label>
                <label style="flex:1; min-width:50px; max-width:60px;">M: <input type="number" id="durM${i}" class="time-input" min="0" max="59" maxlength="2" style="width:100%;"></label>
                <label style="flex:1; min-width:50px; max-width:60px;">S: <input type="number" id="durS${i}" class="time-input" min="0" max="59" maxlength="2" style="width:100%;"></label>
              </div>
            </div>
            <button class="btn-save" onclick="saveTimer(${i})" style="width:100%; margin-top:10px;">Guardar</button>
            <div id="timerStatus${i}" style="margin-top:5px; font-size:0.9rem; color:#666;"></div>
         `;
         list.appendChild(row);
     }
  }

  function saveTimer(id) {
      const statusDiv = document.getElementById(`timerStatus${id}`);
      statusDiv.innerHTML = "";
      statusDiv.style.color = "#666";

      const enabled = document.getElementById(`timerEnable${id}`).checked ? 1 : 0;
      const mode = document.querySelector(`input[name="mode${id}"]:checked`).value;
      
      let params = `channel=${id}&enabled=${enabled}`;
      let details = "";
      
      if (mode === 'daily') {
        const start = document.getElementById(`start${id}`).value;
        const end = document.getElementById(`end${id}`).value;
        if(!start || !end) {
          statusDiv.innerHTML = "Error: Define hora inicio y fin";
          statusDiv.style.color = "red";
          return;
        }
        params += `&start=${start}&end=${end}&isDuration=0`;
        details = `Horario diario: ${start} - ${end}`;
      } else {
        const start = document.getElementById(`startDur${id}`).value;
        const h = parseInt(document.getElementById(`durH${id}`).value) || 0;
        const m = parseInt(document.getElementById(`durM${id}`).value) || 0;
        const s = parseInt(document.getElementById(`durS${id}`).value) || 0;
        if (h > 24 || m > 59 || s > 59) {
          statusDiv.innerHTML = "Error: Horas 0-24, Minutos/Segundos 0-59";
          statusDiv.style.color = "red";
          return;
        }
        const duration = h * 3600 + m * 60 + s;
        if(!start || duration <= 0) {
          statusDiv.innerHTML = "Error: Define hora inicio y duración > 0";
          statusDiv.style.color = "red";
          return;
        }
        params += `&start=${start}&end=00:00&isDuration=1&duration=${duration}`;
        details = `Duración: ${start} por ${h}h ${m}m ${s}s`;
      }

      fetch(`/set_timer?${params}`)
        .then(r => r.text())
        .then(msg => {
            statusDiv.innerHTML = `Guardado: ${details}`;
            statusDiv.style.color = "green";
            logToConsole(`Timer ${id} Saved [${enabled ? 'ON' : 'OFF'}]`);
        })
        .catch(e => {
            statusDiv.innerHTML = "Error al guardar";
            statusDiv.style.color = "red";
        });
  }

  function toggleTimerInputs(id) {
    const enabled = document.getElementById(`timerEnable${id}`).checked;
    const radios = document.querySelectorAll(`input[name="mode${id}"]`);
    const dailyControls = document.getElementById(`dailyControls${id}`);
    const durationControls = document.getElementById(`durationControls${id}`);
    
    radios.forEach(r => r.disabled = !enabled);
    if (enabled) {
      toggleMode(id);
    } else {
      dailyControls.style.display = 'none';
      durationControls.style.display = 'none';
    }
  }

  function toggleMode(id) {
    const mode = document.querySelector(`input[name="mode${id}"]:checked`).value;
    const dailyControls = document.getElementById(`dailyControls${id}`);
    const durationControls = document.getElementById(`durationControls${id}`);
    
    if (mode === 'daily') {
      dailyControls.style.display = 'flex';
      durationControls.style.display = 'none';
    } else {
      dailyControls.style.display = 'none';
      durationControls.style.display = 'flex';
    }
  }

  // -- COMMON --
  function updateStatus() {
    fetch('/status').then(r => r.json()).then(data => {
        // Update debug tab toggles
        for (let i = 1; i <= numRelays; i++) {
            const cb = document.getElementById(`relay${i}`);
            if (cb) cb.checked = data[i-1] === 1;
        }
        // Update monitor dashboard
        updateMonitorStatus(data);
    });
  }

  function fetchLogs() {
      fetch('/logs').then(r => r.text()).then(txt => {
          if (txt.length > 0) logToConsole(txt, true);
      });
  }

  function logToConsole(msg, raw = false) {
      if (!raw) {
        const time = new Date().toLocaleTimeString();
        consoleArea.value += `[${time}] ${msg}\n`;
      } else {
        consoleArea.value += msg;
      }
      consoleArea.scrollTop = consoleArea.scrollHeight;
  }

  // -- WIFI --
  function scanWifi() {
      const list = document.getElementById('scan-results');
      list.innerHTML = '<div style="padding:15px; text-align:center; color:#999;">Scanning networks...</div>';
      fetch('/scan')
        .then(r => r.json())
        .then(data => {
            if(data.length === 0) {
                list.innerHTML = '<div style="padding:15px; text-align:center; color:#999;">No networks found</div>';
                return;
            }
            list.innerHTML = '';
            data.forEach(net => {
                const item = document.createElement('div');
                item.className = 'scan-item';
                item.innerHTML = `<b>${net.ssid}</b> <span style="color:#999; float:right;">${net.rssi} dBm</span>`;
                item.onclick = () => {
                    document.getElementById('ssid').value = net.ssid;
                    document.querySelectorAll('.scan-item').forEach(el => el.style.background = '');
                    item.style.background = 'rgba(187, 134, 252, 0.2)';
                };
                list.appendChild(item);
            });
        })
        .catch(e => list.innerHTML = '<div style="padding:15px; text-align:center; color:#f44;">Scan failed</div>');
  }

  function saveWifi() {
      const ssid = document.getElementById('ssid').value;
      const pass = document.getElementById('pass').value;
      if(!ssid) return alert("SSID required");
      
      if(confirm(`Connect to ${ssid} and restart?`)) {
          fetch(`/save_wifi?ssid=${ssid}&pass=${pass}`)
            .then(r => r.text())
            .then(msg => alert(msg))
            .catch(e => alert("Error saving"));
      }
  }

  function resetWifi() {
      if(confirm("Forget WiFi credentials and restart?")) {
          fetch('/reset_wifi')
            .then(r => r.text())
            .then(msg => alert(msg));
      }
  }

  // -- SYSTEM TAB --
  let selectedFile = null;

  function loadSystemInfo() {
    fetch('/system_info')
      .then(r => r.json())
      .then(data => {
        document.getElementById('info-version').textContent = 'v{{VERSION}}';
        document.getElementById('info-chip').textContent = data.chipModel + ' (Rev ' + data.chipRevision + ')';
        document.getElementById('info-cpu').textContent = data.cpuFreqMHz + ' MHz';
        document.getElementById('info-heap').textContent = formatBytes(data.freeHeap) + ' / ' + formatBytes(data.totalHeap);
        document.getElementById('info-flash').textContent = formatBytes(data.flashSize);
        document.getElementById('info-ota-space').textContent = formatBytes(data.freeSketchSpace);
        document.getElementById('info-uptime').textContent = data.uptimeStr;
        document.getElementById('info-rssi').textContent = data.wifiRSSI + ' dBm';
        document.getElementById('info-ip').textContent = data.ipAddress;
        document.getElementById('info-mac').textContent = data.macAddress;
      })
      .catch(e => logToConsole('Error loading system info'));
  }

  function formatBytes(bytes) {
    if (bytes === 0) return '0 B';
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
  }

  function handleFileSelect(input) {
    const file = input.files[0];
    if (file) {
      if (!file.name.endsWith('.bin')) {
        alert('Por favor selecciona un archivo .bin');
        input.value = '';
        return;
      }
      selectedFile = file;
      document.getElementById('file-info').style.display = 'block';
      document.getElementById('file-name').textContent = file.name;
      document.getElementById('file-size').textContent = formatBytes(file.size);
      document.getElementById('upload-btn').disabled = false;
    }
  }

  function clearFile() {
    selectedFile = null;
    document.getElementById('firmware-file').value = '';
    document.getElementById('file-info').style.display = 'none';
    document.getElementById('upload-btn').disabled = true;
    document.getElementById('progress-container').style.display = 'none';
  }

  function uploadFirmware() {
    if (!selectedFile) {
      alert('Selecciona un archivo primero');
      return;
    }

    if (!confirm('¿Estás seguro de actualizar el firmware? El dispositivo se reiniciará.')) {
      return;
    }

    const formData = new FormData();
    formData.append('firmware', selectedFile);

    document.getElementById('upload-btn').disabled = true;
    document.getElementById('progress-container').style.display = 'block';
    document.getElementById('progress-status').textContent = 'Subiendo firmware...';

    const xhr = new XMLHttpRequest();
    
    xhr.upload.addEventListener('progress', function(e) {
      if (e.lengthComputable) {
        const percent = Math.round((e.loaded / e.total) * 100);
        document.getElementById('progress-bar').style.width = percent + '%';
        document.getElementById('progress-percent').textContent = percent + '%';
      }
    });

    xhr.addEventListener('load', function() {
      if (xhr.status === 200) {
        document.getElementById('progress-status').innerHTML = '<span style="color:#4caf50;">✓ Actualización exitosa! Reiniciando...</span>';
        document.getElementById('progress-bar').style.background = '#4caf50';
        logToConsole('OTA Update successful! Rebooting...');
        
        // Wait and reload page
        setTimeout(() => {
          document.getElementById('progress-status').innerHTML = '<span style="color:#999;">Reconectando en 10 segundos...</span>';
          setTimeout(() => {
            window.location.reload();
          }, 10000);
        }, 2000);
      } else {
        document.getElementById('progress-status').innerHTML = '<span style="color:#f44;">✗ Error en la actualización</span>';
        document.getElementById('progress-bar').style.background = '#f44';
        document.getElementById('upload-btn').disabled = false;
        logToConsole('OTA Update failed: ' + xhr.responseText);
      }
    });

    xhr.addEventListener('error', function() {
      document.getElementById('progress-status').innerHTML = '<span style="color:#f44;">✗ Error de conexión</span>';
      document.getElementById('progress-bar').style.background = '#f44';
      document.getElementById('upload-btn').disabled = false;
      logToConsole('OTA Update error: Connection failed');
    });

    xhr.open('POST', '/do_update', true);
    xhr.send(formData);
  }

  function rebootDevice() {
    if (confirm('¿Reiniciar el dispositivo ahora?')) {
      fetch('/reboot', { method: 'POST' })
        .then(r => r.json())
        .then(data => {
          alert('Reiniciando... La página se recargará en 10 segundos.');
          logToConsole('Rebooting device...');
          setTimeout(() => {
            window.location.reload();
          }, 10000);
        })
        .catch(e => alert('Error al reiniciar'));
    }
  }

  // Setup drag and drop for OTA
  function setupDropZone() {
    const dropZone = document.getElementById('drop-zone');
    if (!dropZone) return;

    ['dragenter', 'dragover', 'dragleave', 'drop'].forEach(eventName => {
      dropZone.addEventListener(eventName, preventDefaults, false);
    });

    function preventDefaults(e) {
      e.preventDefault();
      e.stopPropagation();
    }

    ['dragenter', 'dragover'].forEach(eventName => {
      dropZone.addEventListener(eventName, () => {
        dropZone.style.borderColor = 'var(--accent-color)';
        dropZone.style.background = 'rgba(187, 134, 252, 0.1)';
      }, false);
    });

    ['dragleave', 'drop'].forEach(eventName => {
      dropZone.addEventListener(eventName, () => {
        dropZone.style.borderColor = '#555';
        dropZone.style.background = 'transparent';
      }, false);
    });

    dropZone.addEventListener('drop', (e) => {
      const files = e.dataTransfer.files;
      if (files.length > 0) {
        document.getElementById('firmware-file').files = files;
        handleFileSelect(document.getElementById('firmware-file'));
      }
    }, false);
  }

  // Load system info when system tab is opened
  const originalOpenTab = openTab;
  openTab = function(tabName) {
    originalOpenTab(tabName);
    if (tabName === 'system') {
      loadSystemInfo();
      setupDropZone();
    }
  };

  window.onload = init;
</script>
</body>
</html>
)rawliteral";

#endif
