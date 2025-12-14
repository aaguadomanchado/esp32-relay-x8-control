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
  <h1>ESP32 Control <span style="font-size:0.5em; color:#777;">v0.3</span></h1>

  <div class="tabs">
    <button class="tab-btn active" onclick="openTab('monitor')">Monitor</button>
    <button class="tab-btn" onclick="openTab('debug')">Test-Debug</button>
    <button class="tab-btn" onclick="openTab('timers')">Temporizadores</button>
    <button class="tab-btn" onclick="openTab('wifi')">WiFi</button>
    <button class="tab-btn" onclick="openTab('labels')">Etiquetas</button>
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
            // data is array of {enabled, start, end}
            for(let i=1; i<=numRelays; i++) {
                const t = data[i-1];
                // Set switch state
                const cb = document.getElementById(`timerEnable${i}`);
                if(cb) cb.checked = t.enabled;

                // Set times (backend now returns them even if disabled)
                if(t.start) document.getElementById(`start${i}`).value = t.start;
                if(t.end) document.getElementById(`end${i}`).value = t.end;
                
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
            <div class="timer-controls">
              <label style="flex:1; min-width:140px;">ON: <input type="time" id="start${i}" class="time-input" style="width:100%;"></label>
              <label style="flex:1; min-width:140px;">OFF: <input type="time" id="end${i}" class="time-input" style="width:100%;"></label>
              <button class="btn-save" onclick="saveTimer(${i})" style="min-width:100px;">Guardar</button>
            </div>
         `;
         list.appendChild(row);
     }
  }

  function saveTimer(id) {
      const start = document.getElementById(`start${id}`).value; // "HH:MM"
      const end = document.getElementById(`end${id}`).value;     // "HH:MM"
      const enabled = document.getElementById(`timerEnable${id}`).checked ? 1 : 0;
      
      if(!start || !end) return alert("Define hora inicio y fin");

      fetch(`/set_timer?channel=${id}&start=${start}&end=${end}&enabled=${enabled}`)
        .then(r => r.text())
        .then(msg => {
            logToConsole(`Timer ${id} Saved [${enabled ? 'ON' : 'OFF'}]`);
        });
  }

  function toggleTimerInputs(id) {
    const enabled = document.getElementById(`timerEnable${id}`).checked;
    const startInput = document.getElementById(`start${id}`);
    const endInput = document.getElementById(`end${id}`);
    
    startInput.disabled = !enabled;
    endInput.disabled = !enabled;
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

  window.onload = init;
</script>
</body>
</html>
)rawliteral";

#endif
