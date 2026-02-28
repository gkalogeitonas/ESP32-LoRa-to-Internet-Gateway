#ifndef WEB_PAGES_H
#define WEB_PAGES_H

#include <Arduino.h>

static const char PAGE_HTML[] PROGMEM = R"rawraw(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>LoRa Gateway Config</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:system-ui,-apple-system,sans-serif;background:#1a1a2e;color:#e0e0e0;padding:16px;max-width:600px;margin:0 auto}
h1{text-align:center;color:#00d4ff;margin-bottom:16px;font-size:1.4em}
.card{background:#16213e;border-radius:8px;padding:16px;margin-bottom:12px;border:1px solid #0f3460}
.card h2{color:#00d4ff;margin-bottom:12px;font-size:1.1em;border-bottom:1px solid #0f3460;padding-bottom:8px}
.status-bar{display:flex;justify-content:space-between;flex-wrap:wrap;gap:8px}
.status-item{font-size:0.9em}
.status-item span{color:#00d4ff;font-weight:bold}
label{display:block;margin-top:10px;margin-bottom:4px;font-size:0.9em;color:#a0a0c0}
input[type="text"],input[type="password"],input[type="number"],select{width:100%;padding:8px;background:#0f3460;border:1px solid #1a5276;border-radius:4px;color:#e0e0e0;font-size:0.95em}
input:focus,select:focus{outline:none;border-color:#00d4ff}
.row{display:flex;gap:10px}
.row>*{flex:1}
.radio-group{display:flex;gap:16px;margin:10px 0}
.radio-group label{display:flex;align-items:center;gap:6px;cursor:pointer;margin:0}
input[type="radio"]{accent-color:#00d4ff}
button{display:block;width:100%;padding:10px;margin-top:12px;background:#00d4ff;color:#1a1a2e;border:none;border-radius:4px;font-weight:bold;cursor:pointer;font-size:0.95em}
button:hover{background:#00b8d9}
button.scan-btn{width:auto;display:inline-block;margin-top:0;padding:8px 14px}
button.danger{background:#e74c3c;color:#fff}
button.danger:hover{background:#c0392b}
.hidden{display:none}
#ssid-row{display:flex;gap:8px;align-items:end}
#ssid-row select{flex:1}
.msg{padding:8px;border-radius:4px;margin-top:8px;font-size:0.9em;text-align:center}
.msg.ok{background:#1e4d2b;color:#4caf50}
.msg.err{background:#4d1e1e;color:#e74c3c}
</style>
</head>
<body>
<h1>LoRa Gateway</h1>

<div class="card">
  <div class="status-bar">
    <div class="status-item">WiFi: <span id="st-wifi">---</span></div>
    <div class="status-item">IP: <span id="st-ip">---</span></div>
    <div class="status-item">Batt: <span id="st-batt">---</span></div>
    <div class="status-item">Pkts: <span id="st-pkts">---</span></div>
  </div>
</div>

<div class="card">
  <h2>WiFi Settings</h2>
  <label>SSID</label>
  <div id="ssid-row">
    <select id="wifi-ssid"><option value="">-- Select or type --</option></select>
    <button class="scan-btn" onclick="doScan()">Scan</button>
  </div>
  <label>Or enter manually</label>
  <input type="text" id="wifi-ssid-manual" placeholder="SSID">
  <label>Password</label>
  <input type="password" id="wifi-pass">
  <button onclick="saveWifi()">Save WiFi</button>
  <div id="wifi-msg"></div>
</div>

<div class="card">
  <h2>LoRa Settings</h2>
  <div class="row">
    <div>
      <label>Frequency (Hz)</label>
      <input type="number" id="lora-freq" value="868000000">
    </div>
    <div>
      <label>Spreading Factor</label>
      <select id="lora-sf">
        <option value="7">SF7</option>
        <option value="8">SF8</option>
        <option value="9">SF9</option>
        <option value="10" selected>SF10</option>
        <option value="11">SF11</option>
        <option value="12">SF12</option>
      </select>
    </div>
  </div>
  <div class="row">
    <div>
      <label>Bandwidth</label>
      <select id="lora-bw">
        <option value="125000" selected>125 kHz</option>
        <option value="250000">250 kHz</option>
        <option value="500000">500 kHz</option>
      </select>
    </div>
    <div>
      <label>Coding Rate</label>
      <select id="lora-cr">
        <option value="5" selected>4/5</option>
        <option value="6">4/6</option>
        <option value="7">4/7</option>
        <option value="8">4/8</option>
      </select>
    </div>
  </div>
  <button onclick="saveLora()">Save LoRa</button>
  <div id="lora-msg"></div>
</div>

<div class="card">
  <h2>Data Forwarding</h2>
  <div class="radio-group">
    <label><input type="radio" name="fwd-mode" value="mqtt" checked onchange="toggleFwd()"> MQTT</label>
    <label><input type="radio" name="fwd-mode" value="http" onchange="toggleFwd()"> HTTP</label>
  </div>

  <div id="mqtt-section">
    <div class="row">
      <div>
        <label>Broker</label>
        <input type="text" id="mqtt-broker" placeholder="broker.example.com">
      </div>
      <div>
        <label>Port</label>
        <input type="number" id="mqtt-port" value="1883">
      </div>
    </div>
    <label>Topic</label>
    <input type="text" id="mqtt-topic" value="lora/gateway">
    <div class="row">
      <div>
        <label>Username</label>
        <input type="text" id="mqtt-user">
      </div>
      <div>
        <label>Password</label>
        <input type="password" id="mqtt-pass">
      </div>
    </div>
  </div>

  <div id="http-section" class="hidden">
    <label>Endpoint URL (POST)</label>
    <input type="text" id="http-url" placeholder="https://example.com/api/lora">
  </div>

  <label>Gateway ID</label>
  <input type="text" id="gw-id" value="lora-gw-001">

  <button onclick="saveForward()">Save Forwarding</button>
  <div id="fwd-msg"></div>
</div>

<div class="card">
  <button class="danger" onclick="doRestart()">Restart Device</button>
</div>

<script>
function $(id){return document.getElementById(id)}

function toggleFwd(){
  var m=document.querySelector('input[name="fwd-mode"]:checked').value;
  $('mqtt-section').classList.toggle('hidden',m!=='mqtt');
  $('http-section').classList.toggle('hidden',m!=='http');
}

function showMsg(id,ok,txt){
  var el=$(id);
  el.className='msg '+(ok?'ok':'err');
  el.textContent=txt;
  setTimeout(function(){el.textContent='';el.className='';},4000);
}

function post(url,data,msgId){
  var fd=new URLSearchParams(data);
  fetch(url,{method:'POST',body:fd})
    .then(function(r){return r.json()})
    .then(function(j){showMsg(msgId,j.status==='ok',j.message)})
    .catch(function(e){showMsg(msgId,false,'Error: '+e)});
}

function saveWifi(){
  var ssid=$('wifi-ssid-manual').value||$('wifi-ssid').value;
  post('/save/wifi',{ssid:ssid,pass:$('wifi-pass').value},'wifi-msg');
}

function saveLora(){
  post('/save/lora',{
    frequency:$('lora-freq').value,
    sf:$('lora-sf').value,
    bandwidth:$('lora-bw').value,
    coding_rate:$('lora-cr').value
  },'lora-msg');
}

function saveForward(){
  var m=document.querySelector('input[name="fwd-mode"]:checked').value;
  post('/save/forward',{
    use_mqtt:m==='mqtt'?'1':'0',
    mqtt_broker:$('mqtt-broker').value,
    mqtt_port:$('mqtt-port').value,
    mqtt_topic:$('mqtt-topic').value,
    mqtt_user:$('mqtt-user').value,
    mqtt_pass:$('mqtt-pass').value,
    http_url:$('http-url').value,
    gateway_id:$('gw-id').value
  },'fwd-msg');
}

function doScan(){
  fetch('/scan').then(function(r){return r.json()}).then(function(nets){
    var sel=$('wifi-ssid');
    sel.innerHTML='<option value="">-- Select --</option>';
    nets.forEach(function(n){
      var o=document.createElement('option');
      o.value=n.ssid;
      o.textContent=n.ssid+' ('+n.rssi+' dBm'+(n.enc?' *':'')+')';
      sel.appendChild(o);
    });
  }).catch(function(){});
}

function doRestart(){
  if(confirm('Restart the gateway?')){
    fetch('/restart',{method:'POST'}).then(function(){
      document.body.innerHTML='<h1 style="text-align:center;margin-top:40vh;color:#00d4ff">Restarting...</h1>';
    });
  }
}

function refreshStatus(){
  fetch('/status').then(function(r){return r.json()}).then(function(s){
    $('st-wifi').textContent=s.wifi;
    $('st-ip').textContent=s.ip;
    $('st-batt').textContent=s.battery+'%';
    $('st-pkts').textContent=s.packets;
    // Populate current config values
    if(s.config){
      var c=s.config;
      $('wifi-ssid-manual').placeholder=c.wifi_ssid||'SSID';
      $('lora-freq').value=c.lora_frequency;
      $('lora-sf').value=c.lora_sf;
      $('lora-bw').value=c.lora_bandwidth;
      $('lora-cr').value=c.lora_coding_rate;
      if(c.use_mqtt){
        document.querySelector('input[name="fwd-mode"][value="mqtt"]').checked=true;
      }else{
        document.querySelector('input[name="fwd-mode"][value="http"]').checked=true;
      }
      toggleFwd();
      $('mqtt-broker').value=c.mqtt_broker||'';
      $('mqtt-port').value=c.mqtt_port;
      $('mqtt-topic').value=c.mqtt_topic||'';
      $('mqtt-user').value=c.mqtt_user||'';
      $('http-url').value=c.http_url||'';
      $('gw-id').value=c.gateway_id||'';
    }
  }).catch(function(){});
}

refreshStatus();
setInterval(refreshStatus,10000);
</script>
</body>
</html>
)rawraw";

#endif
