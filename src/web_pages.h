#ifndef WEB_PAGES_H
#define WEB_PAGES_H

#include <Arduino.h>

// CSS-only head section. The body HTML is built dynamically in web_server.cpp.
static const char PAGE_HEAD[] PROGMEM = R"rawraw(<!DOCTYPE html>
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
button,input[type="submit"]{display:block;width:100%;padding:10px;margin-top:12px;background:#00d4ff;color:#1a1a2e;border:none;border-radius:4px;font-weight:bold;cursor:pointer;font-size:0.95em}
button:hover,input[type="submit"]:hover{background:#00b8d9}
.danger{background:#e74c3c;color:#fff}
.danger:hover{background:#c0392b}
.hidden{display:none}
.msg{padding:10px;border-radius:4px;margin-bottom:12px;font-size:0.9em;text-align:center;background:#1e4d2b;color:#4caf50}
</style>
</head>
<body>
<h1>LoRa Gateway</h1>
)rawraw";

static const char PAGE_FOOT[] PROGMEM = R"rawraw(
<script>
function uf(){
  var c=document.getElementById('lora-channel');
  var f=document.getElementById('custom-freq-section');
  var fi=document.getElementById('lora-freq');
  if(c.value==='custom'){f.style.display='block';}
  else{f.style.display='none';fi.value=c.value;}
}
function tf(){
  var m=document.querySelector('input[name="use_mqtt"]:checked').value;
  document.getElementById('mqtt-section').style.display=(m==='1'?'block':'none');
  document.getElementById('http-section').style.display=(m==='0'?'block':'none');
}
</script>
</body>
</html>
)rawraw";

#endif
