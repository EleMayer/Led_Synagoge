#pragma once

#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">

<head>

<meta charset="UTF-8">
<meta name="viewport"
      content="width=device-width, initial-scale=1, viewport-fit=cover">

<title>LED-Fassade</title>

<link rel="manifest" href="/manifest.json">

<meta name="theme-color" content="#12171d">
<meta name="mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style"
      content="black-translucent">
<meta name="apple-mobile-web-app-title" content="Fassade">

<link rel="apple-touch-icon" href="/apple-touch-icon.png">
<link rel="icon" type="image/svg+xml" href="/icon.svg">

<style>

:root{

  --bg:#12171d;
  --card:#1a212a;
  --line:#28323d;
  --line-strong:#374350;
  --fg:#e7ecf2;
  --muted:#8d9aa9;
  --accent:#6f8494;
  --accent-weak:#222c36;
  --accent-ink:#0f151b;
  --fill:#566574;
  --ok:#5a9b74;
  --bad:#b26b73;
  --radius:14px;
  --shadow:0 1px 2px rgba(0,0,0,.25), 0 8px 24px rgba(0,0,0,.22);
}

*{box-sizing:border-box}
html,body{margin:0}
body{
  background:var(--bg); color:var(--fg);
  font:15px/1.5 system-ui,-apple-system,"Segoe UI",Roboto,sans-serif;
  padding-bottom:env(safe-area-inset-bottom);
  -webkit-font-smoothing:antialiased;
}

.top{
  position:sticky; top:0; z-index:5;
  display:flex; justify-content:space-between; align-items:center;
  padding:14px 18px; background:rgba(18,23,29,.9); backdrop-filter:blur(8px);
  border-bottom:1px solid var(--line);
}
.brand{display:flex; align-items:center; gap:10px; font-size:16px; letter-spacing:.01em}
.dot{width:9px;height:9px;border-radius:50%;background:var(--bad);
  box-shadow:0 0 0 4px rgba(178,107,115,.16)}
.dot.on{background:var(--ok);box-shadow:0 0 0 4px rgba(90,155,116,.16)}
.hstat{display:flex;align-items:center;gap:12px}
#clock{font-variant-numeric:tabular-nums;color:var(--muted)}
.badge{background:var(--accent-weak);border:1px solid var(--line);border-radius:999px;
  padding:4px 12px;font-size:12px;font-weight:600;color:var(--fg)}

main{max-width:600px;margin:0 auto;padding:18px;display:grid;gap:16px}

.card{background:var(--card);border:1px solid var(--line);border-radius:var(--radius);
  padding:18px 18px 20px;box-shadow:var(--shadow)}
.card h2{margin:0 0 14px;font-size:12px;color:var(--muted);
  text-transform:uppercase;letter-spacing:.08em;font-weight:700}

.modes{display:grid;grid-template-columns:repeat(2,1fr);gap:10px}
.modes button{
  padding:14px;border:1px solid var(--line-strong);border-radius:11px;
  background:#131a21;color:var(--fg);font-size:15px;cursor:pointer;
  transition:.15s;font-weight:500}
.modes button:hover{border-color:var(--accent);color:#cdd8e2}
.modes button.active{
  background:var(--accent);color:var(--accent-ink);border-color:var(--accent);
  font-weight:600}
.modes button:active{transform:scale(.98)}

.slider{margin:14px 0}
.slider:first-of-type{margin-top:4px}
.slider label{display:flex;justify-content:space-between;color:var(--muted);
  font-size:14px;margin-bottom:8px}
.slider .val{color:var(--fg);font-variant-numeric:tabular-nums;font-weight:600}
input[type=range]{-webkit-appearance:none;appearance:none;width:100%;height:24px;
  background:transparent;cursor:pointer}
input[type=range]::-webkit-slider-runnable-track{height:6px;border-radius:6px;background:#2b3540}
input[type=range]::-moz-range-track{height:6px;border-radius:6px;background:#2b3540}
input[type=range]::-moz-range-progress{height:6px;border-radius:6px;background:var(--fill)}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;appearance:none;
  width:20px;height:20px;border-radius:50%;background:#e7ecf2;border:2px solid var(--accent);
  margin-top:-7px;box-shadow:0 1px 3px rgba(0,0,0,.4)}
input[type=range]::-moz-range-thumb{width:20px;height:20px;border-radius:50%;
  background:#e7ecf2;border:2px solid var(--accent);box-shadow:0 1px 3px rgba(0,0,0,.4)}

button.primary{background:var(--accent);color:var(--accent-ink);border:none;border-radius:10px;
  padding:12px 16px;font-weight:600;cursor:pointer;transition:.15s;width:100%}
button.primary:hover{filter:brightness(1.08)}
label.field{display:block;color:var(--muted);font-size:13px;margin:12px 0 5px}
input[type=text],input[type=password]{width:100%;padding:11px;border-radius:10px;
  border:1px solid var(--line-strong);background:#131a21;color:var(--fg);font-size:15px}
input[type=text]:focus,input[type=password]:focus{outline:none;border-color:var(--accent);
  box-shadow:0 0 0 3px var(--accent-weak)}

input[type=number]{width:100%;padding:10px;border-radius:10px;
  border:1px solid var(--line-strong);background:#131a21;color:var(--fg);
  font-size:15px;text-align:right;font-variant-numeric:tabular-nums}
input[type=number]:focus{outline:none;border-color:var(--accent);
  box-shadow:0 0 0 3px var(--accent-weak)}
.srow{display:grid;grid-template-columns:1fr 88px;gap:12px;align-items:center;
  padding:8px 0;border-bottom:1px solid var(--line)}
.srow:last-of-type{border-bottom:none}
.srow label{color:var(--muted);font-size:14px}
.subhead{color:var(--muted);font-size:12px;text-transform:uppercase;
  letter-spacing:.06em;font-weight:700;margin:16px 0 4px}

.curve{background:#131a21;border:1px solid var(--line-strong);border-radius:10px;
  padding:8px 6px 4px;margin-top:6px}
.curve svg{display:block;width:100%;height:auto}
.curve .grid{stroke:var(--line);stroke-width:1}
.curve .axis{fill:var(--muted);font-size:9px;font-variant-numeric:tabular-nums}
.curve .area{fill:var(--accent-weak)}
.curve .line{fill:none;stroke:var(--fill);stroke-width:2;
  stroke-linejoin:round;stroke-linecap:round}
.curve .now{stroke:var(--accent);stroke-width:1.5;stroke-dasharray:3 3}

.preset{display:flex;gap:8px;margin-top:8px}
.preset .apply{flex:1;padding:12px;border:1px solid var(--line-strong);border-radius:10px;
  background:#131a21;color:var(--fg);cursor:pointer;text-align:left;transition:.15s}
.preset .apply:hover{border-color:var(--accent)}
.preset .delete{width:64px;border:1px solid var(--line-strong);border-radius:10px;
  background:#241a1c;color:#d3a3a8;cursor:pointer}
.preset .delete:hover{border-color:var(--bad)}

.sys{display:grid;gap:0}
.kv{display:flex;justify-content:space-between;padding:9px 0;border-bottom:1px solid var(--line);gap:12px}
.kv:last-child{border-bottom:none}
.kv span{color:var(--muted)}
.kv b{font-weight:600;font-variant-numeric:tabular-nums;text-align:right;word-break:break-all}
.kv b.ok{color:var(--ok)}
.kv b.warn{color:#d0a86a}

hr{border:0;border-top:1px solid var(--line);margin:16px 0}
.hint{color:var(--muted);font-size:13px;margin:12px 0 0}
.hint b{color:var(--fg);font-weight:600}
footer{text-align:center;color:var(--muted);font-size:12px;padding:22px}

</style>

</head>

<body>

<header class="top">
  <div class="brand">
    <span class="dot" id="connDot"></span>
    <strong>LED-Fassade</strong>
  </div>
  <div class="hstat">
    <span id="clock">--:--:--</span>
    <span id="modeBadge" class="badge">&ndash;</span>
  </div>
</header>

<main>

<section class="card" id="installCard" style="display:none">
  <h2>Als App installieren</h2>
  <p class="hint" id="installHint" style="margin-top:0">Diese Seite als App auf
    dem Startbildschirm ablegen &ndash; sie &ouml;ffnet dann randlos wie eine
    gewohnte App.</p>
  <button class="primary" id="installBtn" onclick="installApp()"
          style="margin-top:12px">Installieren</button>
</section>

<section class="card">
  <h2>Modus</h2>
  <div class="modes">
    <button class="mode" data-mode="0" onclick="sendMode(0)">Aus</button>
    <button class="mode" data-mode="1" onclick="sendMode(1)">Statisch</button>
    <button class="mode" data-mode="2" onclick="sendMode(2)">Lauflicht</button>
    <button class="mode" data-mode="3" onclick="sendMode(3)">Automatik</button>
    <button class="mode" data-mode="4" onclick="sendMode(4)">Pulsieren</button>
    <button class="mode" data-mode="5" onclick="sendMode(5)">Atmen</button>
    <button class="mode" data-mode="10" onclick="sendMode(10)">Welle</button>
    <button class="mode" data-mode="6" onclick="sendMode(6)">Dauerlicht</button>
    <button class="mode" data-mode="7" onclick="sendMode(7)">Kerzenlicht</button>
    <button class="mode" data-mode="8" onclick="sendMode(8)">Stufenlicht</button>
    <button class="mode" data-mode="9" onclick="sendMode(9)">Dämmerlicht</button>
    <button class="mode" data-mode="11" onclick="sendMode(11)">Feuerschein</button>
    <button class="mode" data-mode="12" onclick="sendMode(12)">Nachtlicht</button>
  </div>
  <p class="hint" id="modeDesc">&ndash;</p>
</section>

<section class="card">
  <h2>Helligkeit</h2>
  <div class="slider">
    <label>Segment Links <span class="val"><span id="leftValue">80</span>%</span></label>
    <input id="leftSlider" type="range" min="0" max="100" value="80">
  </div>
  <div class="slider">
    <label>Segment Rechts <span class="val"><span id="rightValue">80</span>%</span></label>
    <input id="rightSlider" type="range" min="0" max="100" value="80">
  </div>
  <div class="slider">
    <label>Logo <span class="val"><span id="logoValue">80</span>%</span></label>
    <input id="logoSlider" type="range" min="0" max="100" value="80">
  </div>
  <p class="hint">Diese Regler wirken im Modus <b>Statisch</b> und werden als Szene gespeichert.</p>
</section>

<section class="card">
  <h2>Effekt-Helligkeit</h2>
  <div class="slider">
    <label>Helligkeit <span class="val"><span id="globalValue">80</span>%</span></label>
    <input id="globalSlider" type="range" min="0" max="100" value="80">
  </div>
  <p class="hint">Für Lauflicht, Pulsieren, Atmen und Welle. Die Stimmungs-Modi
    (Dauerlicht, Kerzenlicht, Stufenlicht, Dämmerlicht, Feuerschein, Nachtlicht)
    sind fest hinterlegt.</p>
</section>

<section class="card">
  <h2>Eigene Szenen</h2>
  <p class="hint" style="margin-top:0">Aktuelle Helligkeitseinstellung als Szene speichern.</p>
  <input id="presetName" type="text" maxlength="15" placeholder="Name der Szene" style="margin:12px 0 0">
  <button class="primary" onclick="savePreset()" style="margin-top:12px">Aktuelle Einstellung speichern</button>
  <hr>
  <div id="presetList">
    <p class="hint" style="margin:0">Noch keine Szenen gespeichert.</p>
  </div>
</section>

<section class="card">
  <h2>Zeitprofil (Automatik)</h2>

  <div class="subhead">Uhrzeiten (Stunde)</div>
  <div class="srow">
    <label for="tMorning">Beginn Hochfahren</label>
    <input id="tMorning" type="number" min="0" max="23">
  </div>
  <div class="srow">
    <label for="tDay">Beginn Tag</label>
    <input id="tDay" type="number" min="0" max="23">
  </div>
  <div class="srow">
    <label for="tEvening">Beginn Abend</label>
    <input id="tEvening" type="number" min="0" max="23">
  </div>
  <div class="srow">
    <label for="tNight">Nachtabschaltung</label>
    <input id="tNight" type="number" min="0" max="23">
  </div>

  <div class="subhead">Helligkeiten (%)</div>
  <div class="srow">
    <label for="bMorning">Ziel beim Hochfahren</label>
    <input id="bMorning" type="number" min="0" max="100">
  </div>
  <div class="srow">
    <label for="bDay">Tageshelligkeit</label>
    <input id="bDay" type="number" min="0" max="100">
  </div>
  <div class="srow">
    <label for="bEveStart">Abend Beginn</label>
    <input id="bEveStart" type="number" min="0" max="100">
  </div>
  <div class="srow">
    <label for="bEveEnd">Abend Ende</label>
    <input id="bEveEnd" type="number" min="0" max="100">
  </div>

  <div class="subhead">Tagesverlauf (Vorschau)</div>
  <div class="curve">
    <svg id="curveSvg" viewBox="0 0 320 132" preserveAspectRatio="none"
         role="img" aria-label="Helligkeitsverlauf über 24 Stunden"></svg>
  </div>

  <button class="primary" onclick="saveSchedule()" style="margin-top:14px">Zeitprofil speichern</button>
  <p class="hint" id="autoStatus">Automatikdaten werden geladen &hellip;</p>
</section>

<section class="card">
  <h2>System</h2>
  <div id="systemStatus" class="sys">
    <div class="kv"><span>Status</span><b>Keine Daten</b></div>
  </div>
</section>

</main>

<footer>Museum Arbeitswelt &middot; lokale Steuerung</footer>

<script>

let socket = null;
let reconnectTimer = null;

const MODE_LABEL = {
  0:"Aus",
  1:"Statisch",
  2:"Lauflicht",
  3:"Automatik",
  4:"Pulsieren",
  5:"Atmen",
  6:"Dauerlicht",
  7:"Kerzenlicht",
  8:"Stufenlicht",
  9:"Dämmerlicht",
  10:"Welle",
  11:"Feuerschein",
  12:"Nachtlicht"
};

const MODE_DESC = {
  0:"Alle Bereiche aus. Der Controller bleibt im WLAN erreichbar.",
  1:"Feste Helligkeit je Segment und Logo (per Regler einstellbar).",
  2:"Lauflicht – ein wanderndes Licht mit der Effekt-Helligkeit.",
  3:"Tageszeitabhängige Helligkeit nach fester Kurve, Nachtabschaltung ab 23:00.",
  4:"Pulsieren – zügiges Auf- und Abschwellen mit der Effekt-Helligkeit.",
  5:"Atmen – langsames, ruhiges Auf- und Abschwellen.",
  6:"Dauerlicht – stetiges, ruhig atmendes Licht auf hohem Niveau. Nachtabschaltung ab 23:00 wie alle diese Modi.",
  7:"Kerzenlicht – zwei sanft und unabhängig flackernde Lichter auf den Segmenten.",
  8:"Stufenlicht – die Lichter bauen sich in acht Schritten auf und beginnen dann von vorn.",
  9:"Dämmerlicht – sehr gedämpftes Licht, das über Minuten sanft auf- und abschwillt.",
  10:"Welle – eine Sinuswelle wandert über den Streifen (Effekt-Helligkeit).",
  11:"Feuerschein – eine kräftige, ruhig flackernde Flamme. Nachtabschaltung ab 23:00.",
  12:"Nachtlicht – eine ruhige, stetig brennende Kerze. Nachtabschaltung ab 23:00."
};

function connect()
{
    if(socket &&
       (socket.readyState === WebSocket.OPEN ||
        socket.readyState === WebSocket.CONNECTING))
    {
        return;
    }

    socket = new WebSocket(
        "ws://" + window.location.hostname + "/ws"
    );

    socket.onopen = function()
    {
        setConnected(true);
    };

    socket.onclose = function()
    {
        setConnected(false);
        reconnectTimer = setTimeout(connect, 2000);
    };

    socket.onerror = function()
    {
        setConnected(false);
    };

    socket.onmessage = function(event)
    {
        try
        {
            updateUI(JSON.parse(event.data));
        }
        catch(error)
        {
            console.log(error);
        }
    };
}

function setConnected(ok)
{
    document
        .getElementById("connDot")
        .classList.toggle("on", ok);
}

function send(data)
{
    if(!socket || socket.readyState !== WebSocket.OPEN)
    {
        return;
    }

    socket.send(JSON.stringify(data));
}

function sendMode(mode)
{
    send({ mode: mode });
}

function setBrightness(type, value)
{
    value = parseInt(value);

    document.getElementById(type + "Value").innerText = value;

    const data = {};
    data[type] = value;

    send(data);
}

function updateUI(data)
{
    const buttons = document.querySelectorAll(".mode");
    for(let i = 0; i < buttons.length; i++)
    {
        const button = buttons[i];
        const buttonMode = parseInt(button.dataset.mode);
        if(buttonMode === data.mode)
        {
            button.classList.add("active");
        }
        else
        {
            button.classList.remove("active");
        }
    }

    if(data.mode !== undefined)
    {
        document.getElementById("modeBadge").innerText = MODE_LABEL[data.mode] || "–";
        document.getElementById("modeDesc").innerText  = MODE_DESC[data.mode]  || "";
    }
    document.getElementById("clock").innerText = data.time || "--:--:--";

    updateSlider("left",   data.left);
    updateSlider("right",  data.right);
    updateSlider("logo",   data.logo);
    updateSlider("global", data.global);

    let sys = "";
    sys += kv("Firmware", data.firmware || "-");
    sys += kv("Uhrzeit",  data.time || "--:--:--");
    sys += kv("Datum",    data.date || "--.--.----");

    if(data.rtc)
    {
        sys += kv("RTC", "OK", "ok");
    }
    else
    {
        sys += kv("RTC", "nicht verf&uuml;gbar", "warn");
    }

    if(data.ntp)
    {
        sys += kv("NTP", "synchronisiert", "ok");
    }
    else
    {
        sys += kv("NTP", "nicht synchron.", "warn");
    }

    if(data.wifi)
    {
        sys += kv("WLAN", "verbunden", "ok");
    }
    else if(data.ap)
    {
        sys += kv("WLAN", "Setup-AP", "warn");
    }
    else
    {
        sys += kv("WLAN", "getrennt", "warn");
    }

    sys += kv("IP", data.ip || "-");

    if(data.wifi && data.rssi !== undefined)
    {
        sys += kv("Signal", data.rssi + " dBm");
    }

    document.getElementById("systemStatus").innerHTML = sys;

    if(data.sched)
    {
        fillSchedField("tMorning",  data.sched.tMorning);
        fillSchedField("tDay",      data.sched.tDay);
        fillSchedField("tEvening",  data.sched.tEvening);
        fillSchedField("tNight",    data.sched.tNight);
        fillSchedField("bMorning",  data.sched.bMorning);
        fillSchedField("bDay",      data.sched.bDay);
        fillSchedField("bEveStart", data.sched.bEveStart);
        fillSchedField("bEveEnd",   data.sched.bEveEnd);
    }

    renderScheduleCurve();

    let autoValue = "-";
    if(data.autoBrightness !== undefined)
    {
        autoValue = data.autoBrightness;
    }
    document.getElementById("autoStatus").innerHTML =
        "Aktuelle Automatik-Helligkeit: <b>" + autoValue + " %</b>";

    if(data.presets)
    {
        renderPresets(data.presets);
    }
}

function kv(label, value, cls)
{
    let classAttr = "";
    if(cls)
    {
        classAttr = " class='" + cls + "'";
    }
    return "<div class='kv'><span>" + label + "</span><b" + classAttr + ">" + value + "</b></div>";
}

function updateSlider(type, value)
{
    if(value === undefined)
    {
        return;
    }

    const slider = document.getElementById(type + "Slider");
    const output = document.getElementById(type + "Value");

    if(slider)
    {
        slider.value = value;
    }
    if(output)
    {
        output.innerText = value;
    }
}

function renderPresets(list)
{
    const box = document.getElementById("presetList");

    if(!list || list.length === 0)
    {
        box.innerHTML =
            "<p class='hint' style='margin:0'>Noch keine Szenen gespeichert.</p>";
        return;
    }

    let html = "";

    for(let i = 0; i < list.length; i++)
    {
        const p = list[i];
        html +=
            "<div class='preset'>" +
                "<button class='apply' onclick='applyPreset(" + p.slot + ")'>" +
                    escapeHtml(p.name) + " &middot; " +
                    p.left + " / " + p.right + " / " + p.logo +
                "</button>" +
                "<button class='delete' onclick='deletePreset(" + p.slot + ")'>Entf.</button>" +
            "</div>";
    }

    box.innerHTML = html;
}

function escapeHtml(value)
{
    return String(value)
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;")
        .replace(/'/g, "&#039;");
}

function savePreset()
{
    const field = document.getElementById("presetName");
    const name = field.value.trim();

    if(name.length === 0)
    {
        alert("Bitte einen Namen eingeben.");
        return;
    }

    send({ savePreset: name });
    field.value = "";
}

function fillSchedField(id, value)
{
    if(value === undefined)
    {
        return;
    }
    const field = document.getElementById(id);
    if(field && document.activeElement !== field)
    {
        field.value = value;
    }
}

function readNumber(id, min, max)
{
    let n = parseInt(document.getElementById(id).value);
    if(isNaN(n))
    {
        n = min;
    }
    if(n < min)
    {
        n = min;
    }
    if(n > max)
    {
        n = max;
    }
    return n;
}

function readSchedule()
{
    return {
        tMorning:  readNumber("tMorning",  0, 23),
        tDay:      readNumber("tDay",      0, 23),
        tEvening:  readNumber("tEvening",  0, 23),
        tNight:    readNumber("tNight",    0, 23),
        bMorning:  readNumber("bMorning",  0, 100),
        bDay:      readNumber("bDay",      0, 100),
        bEveStart: readNumber("bEveStart", 0, 100),
        bEveEnd:   readNumber("bEveEnd",   0, 100)
    };
}

function saveSchedule()
{
    const s = readSchedule();

    if(!(s.tMorning < s.tDay && s.tDay < s.tEvening && s.tEvening < s.tNight))
    {
        alert("Die Uhrzeiten müssen aufsteigend sein:\n" +
              "Hochfahren < Tag < Abend < Nachtabschaltung.");
        return;
    }

    send(s);
}

function brightnessAtMinute(m, s)
{
    if(m >= s.tNight * 60 || m < s.tMorning * 60)
    {
        return 0;
    }
    if(m >= s.tMorning * 60 && m < s.tDay * 60)
    {
        let p = (m - s.tMorning * 60) / (s.tDay * 60 - s.tMorning * 60);
        p = Math.max(0, Math.min(1, p));
        return Math.round(p * s.bMorning);
    }
    if(m >= s.tDay * 60 && m < s.tEvening * 60)
    {
        return s.bDay;
    }
    if(m >= s.tEvening * 60 && m < s.tNight * 60)
    {
        let p = (m - s.tEvening * 60) / (s.tNight * 60 - s.tEvening * 60);
        p = Math.max(0, Math.min(1, p));
        return Math.round(s.bEveStart + (s.bEveEnd - s.bEveStart) * p);
    }
    return 0;
}

function renderScheduleCurve()
{
    const svg = document.getElementById("curveSvg");
    if(!svg)
    {
        return;
    }

    const s = readSchedule();

    const L = 22, R = 6, T = 6, B = 18;
    const W = 320, H = 132;
    const pw = W - L - R;
    const ph = H - T - B;

    const xOf = min => L + (min / 1440) * pw;
    const yOf = pct => T + (1 - pct / 100) * ph;

    let pts = [];
    for(let m = 0; m <= 1440; m += 6)
    {
        pts.push([xOf(m), yOf(brightnessAtMinute(Math.min(m, 1439), s))]);
    }

    let line = "M" + pts.map(p => p[0].toFixed(1) + "," + p[1].toFixed(1)).join(" L");
    let area = line +
        " L" + xOf(1440).toFixed(1) + "," + yOf(0).toFixed(1) +
        " L" + xOf(0).toFixed(1)   + "," + yOf(0).toFixed(1) + " Z";

    let svgParts = "";

    [0, 50, 100].forEach(function(pct)
    {
        const y = yOf(pct).toFixed(1);
        svgParts += "<line class='grid' x1='" + L + "' y1='" + y +
                    "' x2='" + (W - R) + "' y2='" + y + "'/>";
        svgParts += "<text class='axis' x='" + (L - 4) + "' y='" +
                    (parseFloat(y) + 3) + "' text-anchor='end'>" + pct + "</text>";
    });

    svgParts += "<path class='area' d='" + area + "'/>";
    svgParts += "<path class='line' d='" + line + "'/>";

    [0, 6, 12, 18, 24].forEach(function(h)
    {
        svgParts += "<text class='axis' x='" + xOf(h * 60).toFixed(1) +
                    "' y='" + (H - 6) + "' text-anchor='middle'>" + h + "</text>";
    });

    const now = new Date();
    const nowX = xOf(now.getHours() * 60 + now.getMinutes()).toFixed(1);
    svgParts += "<line class='now' x1='" + nowX + "' y1='" + T +
                "' x2='" + nowX + "' y2='" + (H - B) + "'/>";

    svg.innerHTML = svgParts;
}

function applyPreset(slot)
{
    send({ applyPreset: slot });
}

function deletePreset(slot)
{
    if(!confirm("Szene wirklich löschen?"))
    {
        return;
    }

    send({ deletePreset: slot });
}

const sliderTypes = ["left", "right", "logo", "global"];
for(let i = 0; i < sliderTypes.length; i++)
{
    const type = sliderTypes[i];
    const slider = document.getElementById(type + "Slider");
    slider.addEventListener("input", function()
    {
        setBrightness(type, this.value);
    });
}

const schedFields = ["tMorning","tDay","tEvening","tNight",
                     "bMorning","bDay","bEveStart","bEveEnd"];
for(let i = 0; i < schedFields.length; i++)
{
    const field = document.getElementById(schedFields[i]);
    if(field)
    {
        field.addEventListener("input", renderScheduleCurve);
    }
}

// --- PWA-Installation ------------------------------------------------------
// Die App laesst sich auf Handy/Tablet/Desktop als eigenstaendige App ablegen.
// Android/Desktop-Browser liefern dafuer das Event "beforeinstallprompt", das
// wir aufheben und hinter dem Installieren-Button ausloesen. iOS/Safari kennt
// das Event nicht und braucht stattdessen einen manuellen Hinweis.

let deferredInstallPrompt = null;

function isStandalone()
{
    return window.matchMedia("(display-mode: standalone)").matches ||
           window.navigator.standalone === true;
}

function isIOS()
{
    return /iphone|ipad|ipod/i.test(navigator.userAgent);
}

function showInstallCard(iosMode)
{
    if(isStandalone())
    {
        return;
    }
    const card = document.getElementById("installCard");
    if(!card)
    {
        return;
    }
    card.style.display = "";

    if(iosMode)
    {
        document.getElementById("installBtn").style.display = "none";
        document.getElementById("installHint").innerHTML =
            "Zum Installieren in Safari auf <b>Teilen</b> tippen und " +
            "<b>&bdquo;Zum Home-Bildschirm&ldquo;</b> w&auml;hlen.";
    }
}

function installApp()
{
    if(!deferredInstallPrompt)
    {
        return;
    }
    deferredInstallPrompt.prompt();
    deferredInstallPrompt.userChoice.then(function()
    {
        deferredInstallPrompt = null;
    });
}

window.addEventListener("beforeinstallprompt", function(event)
{
    event.preventDefault();
    deferredInstallPrompt = event;
    showInstallCard(false);
});

window.addEventListener("appinstalled", function()
{
    deferredInstallPrompt = null;
    const card = document.getElementById("installCard");
    if(card)
    {
        card.style.display = "none";
    }
});

window.addEventListener("load", function()
{
    if(!isStandalone() && isIOS())
    {
        showInstallCard(true);
    }
});

window.addEventListener("load", connect);
window.addEventListener("load", renderScheduleCurve);

</script>

</body>
</html>
)rawliteral";

const char manifest_json[] PROGMEM = R"rawliteral(
{
    "id": "/",
    "name": "LED-Fassade",
    "short_name": "Fassade",
    "description": "Bedienung der LED-Fassadenbeleuchtung",
    "lang": "de",
    "dir": "ltr",
    "categories": ["utilities"],
    "start_url": "/",
    "scope": "/",
    "display": "standalone",
    "display_override": ["standalone", "minimal-ui"],
    "orientation": "portrait",
    "background_color": "#12171d",
    "theme_color": "#12171d",
    "icons": [
        {
            "src": "/icon-192.png",
            "sizes": "192x192",
            "type": "image/png",
            "purpose": "any"
        },
        {
            "src": "/icon-512.png",
            "sizes": "512x512",
            "type": "image/png",
            "purpose": "any"
        },
        {
            "src": "/icon.svg",
            "sizes": "any",
            "type": "image/svg+xml",
            "purpose": "maskable"
        }
    ]
}
)rawliteral";

const char icon_svg[] PROGMEM = R"rawliteral(
<svg xmlns="http://www.w3.org/2000/svg"
     viewBox="0 0 512 512">

<rect width="512" height="512" rx="96" fill="#12171d"/>

<circle cx="256" cy="212" r="118" fill="#6f8494"/>

<rect x="204" y="316" width="104" height="66" rx="18" fill="#6f8494"/>
<rect x="220" y="384" width="72"  height="34" rx="14" fill="#6f8494"/>

<rect x="236" y="150" width="40"  height="120" rx="20" fill="#12171d"/>
<rect x="196" y="192" width="120" height="40"  rx="20" fill="#12171d"/>

</svg>
)rawliteral";
