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

<meta name="theme-color" content="#ffffff">
<meta name="mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style"
      content="default">
<meta name="apple-mobile-web-app-title" content="Fassade">

<link rel="apple-touch-icon" href="/apple-touch-icon.png">
<link rel="icon" type="image/svg+xml" href="/icon.svg">

<style>

:root{

  --bg:#0a0b0d;
  --card:#131619;
  --line:#242830;
  --line-strong:#333a44;
  --fg:#e7ecf1;
  --muted:#8b95a1;
  --accent:#2bb6c4;
  --accent-weak:#122a2e;
  --accent-ink:#05171a;
  --fill:#2bb6c4;
  --ok:#4bbf87;
  --warn:#d3a24e;
  --bad:#d76b74;
  --radius:12px;
  --shadow:0 1px 2px rgba(0,0,0,.4);
}

*{box-sizing:border-box}
html,body{margin:0}
body{
  background:var(--bg); color:var(--fg);
  font:15px/1.55 system-ui,-apple-system,"Segoe UI",Roboto,sans-serif;
  padding-bottom:env(safe-area-inset-bottom);
  -webkit-font-smoothing:antialiased;
}

.top{
  position:sticky; top:0; z-index:5;
  display:flex; justify-content:space-between; align-items:center;
  padding:16px 20px; background:rgba(10,11,13,.9); backdrop-filter:blur(8px);
  border-bottom:1px solid var(--line);
}
.brand{display:flex; align-items:center; gap:10px; font-size:16px; font-weight:600; letter-spacing:.01em}
.dot{width:9px;height:9px;border-radius:50%;background:var(--bad);
  box-shadow:0 0 0 3px rgba(192,80,90,.14)}
.dot.on{background:var(--ok);box-shadow:0 0 0 3px rgba(47,138,91,.16)}
.hstat{display:flex;align-items:center;gap:14px}
#clock{font-variant-numeric:tabular-nums;color:var(--muted)}
.badge{background:var(--accent-weak);border:1px solid var(--accent-weak);border-radius:999px;
  padding:4px 12px;font-size:12px;font-weight:600;color:var(--accent)}

main{max-width:600px;margin:0 auto;padding:20px;display:grid;gap:18px}

.card{background:var(--card);border:1px solid var(--line);border-radius:var(--radius);
  padding:20px;box-shadow:var(--shadow)}
.card h2{margin:0 0 16px;font-size:11px;color:var(--muted);
  text-transform:uppercase;letter-spacing:.1em;font-weight:700}

.modes{display:grid;grid-template-columns:repeat(2,1fr);gap:10px}
.modes button{
  padding:14px;border:1px solid var(--line-strong);border-radius:9px;
  background:#171a1f;color:var(--fg);font-size:15px;cursor:pointer;
  transition:.12s;font-weight:500}
.modes button:hover{border-color:var(--accent);color:var(--accent)}
.modes button.active{
  background:var(--accent);color:var(--accent-ink);border-color:var(--accent);
  font-weight:600}
.modes button:active{transform:scale(.99)}

.slider{margin:16px 0}
.slider:first-of-type{margin-top:4px}
.slider label{display:flex;justify-content:space-between;color:var(--muted);
  font-size:14px;margin-bottom:10px}
.slider .val{color:var(--fg);font-variant-numeric:tabular-nums;font-weight:600}
input[type=range]{-webkit-appearance:none;appearance:none;width:100%;height:24px;
  background:transparent;cursor:pointer}
input[type=range]::-webkit-slider-runnable-track{height:5px;border-radius:5px;background:#2a2f37}
input[type=range]::-moz-range-track{height:5px;border-radius:5px;background:#2a2f37}
input[type=range]::-moz-range-progress{height:5px;border-radius:5px;background:var(--fill)}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;appearance:none;
  width:20px;height:20px;border-radius:50%;background:#e7ecf1;border:2px solid var(--accent);
  margin-top:-8px;box-shadow:0 1px 2px rgba(0,0,0,.5)}
input[type=range]::-moz-range-thumb{width:20px;height:20px;border-radius:50%;
  background:#e7ecf1;border:2px solid var(--accent);box-shadow:0 1px 2px rgba(0,0,0,.5)}

button.primary{background:var(--accent);color:var(--accent-ink);border:none;border-radius:9px;
  padding:12px 16px;font-weight:600;cursor:pointer;transition:.12s;width:100%}
button.primary:hover{filter:brightness(1.05)}
input[type=text]{width:100%;padding:11px 12px;border-radius:9px;
  border:1px solid var(--line-strong);background:#fff;color:var(--fg);font-size:15px}
input[type=text]:focus{outline:none;border-color:var(--accent);
  box-shadow:0 0 0 3px var(--accent-weak)}

.curve{background:#fafbfc;border:1px solid var(--line);border-radius:9px;
  padding:8px 6px 4px;margin-top:4px}
.curve svg{display:block;width:100%;height:auto}
.curve .grid{stroke:var(--line);stroke-width:1}
.curve .axis{fill:var(--muted);font-size:9px;font-variant-numeric:tabular-nums}
.curve .area{fill:var(--accent-weak)}
.curve .line{fill:none;stroke:var(--fill);stroke-width:2;
  stroke-linejoin:round;stroke-linecap:round}
.curve .now{stroke:var(--accent);stroke-width:1.5;stroke-dasharray:3 3}

.phases{margin-top:16px}
.phase{display:grid;grid-template-columns:1fr auto auto;gap:14px;align-items:baseline;
  padding:11px 0;border-bottom:1px solid var(--line)}
.phase:last-child{border-bottom:none}
.phase .ph-name{color:var(--fg)}
.phase .ph-time{color:var(--muted);font-variant-numeric:tabular-nums;font-size:13px}
.phase .ph-val{color:var(--fg);font-weight:600;font-variant-numeric:tabular-nums;
  text-align:right;min-width:84px}

.preset{display:flex;gap:8px;margin-top:10px}
.preset .apply{flex:1;padding:12px;border:1px solid var(--line-strong);border-radius:9px;
  background:#fff;color:var(--fg);cursor:pointer;text-align:left;transition:.12s}
.preset .apply:hover{border-color:var(--accent)}
.preset .delete{width:64px;border:1px solid var(--line-strong);border-radius:9px;
  background:#fff;color:var(--bad);cursor:pointer;transition:.12s}
.preset .delete:hover{border-color:var(--bad);background:#fdf3f4}

.sys{display:grid;gap:0}
.kv{display:flex;justify-content:space-between;padding:10px 0;border-bottom:1px solid var(--line);gap:12px}
.kv:last-child{border-bottom:none}
.kv span{color:var(--muted)}
.kv b{font-weight:600;font-variant-numeric:tabular-nums;text-align:right;word-break:break-all}
.kv b.ok{color:var(--ok)}
.kv b.warn{color:var(--warn)}

hr{border:0;border-top:1px solid var(--line);margin:18px 0}
.hint{color:var(--muted);font-size:13px;margin:12px 0 0}
.hint b{color:var(--fg);font-weight:600}
footer{text-align:center;color:var(--muted);font-size:12px;padding:24px}

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
</section>

<section class="card">
  <h2>Effekt-Helligkeit</h2>
  <div class="slider">
    <label>Helligkeit <span class="val"><span id="globalValue">80</span>%</span></label>
    <input id="globalSlider" type="range" min="0" max="100" value="80">
  </div>
</section>

<section class="card">
  <h2>Eigene Szenen</h2>
  <input id="presetName" type="text" maxlength="15" placeholder="Name der Szene">
  <button class="primary" onclick="savePreset()" style="margin-top:12px">Aktuelle Einstellung speichern</button>
  <hr>
  <div id="presetList">
    <p class="hint" style="margin:0">Noch keine Szenen gespeichert.</p>
  </div>
</section>

<section class="card">
  <h2>Automatik</h2>

  <div class="curve">
    <svg id="curveSvg" viewBox="0 0 320 132" preserveAspectRatio="none"
         role="img" aria-label="Helligkeitsverlauf über 24 Stunden"></svg>
  </div>

  <div id="autoPhases" class="phases"></div>

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
        // Das komplette Automatik-Profil (Uhrzeiten und Helligkeiten) ist fest
        // im Code (config.h) und wird nur angezeigt, nicht editiert.
        schedule = data.sched;
        renderAutoPhases();
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

// Komplettes Automatik-Profil (Uhrzeiten + Helligkeiten). Steht fest in
// config.h und kommt nur ueber den Status - in der App reine Anzeige.
let schedule = {
    tMorning: 6, tDay: 8, tEvening: 18, tNight: 23,
    bMorning: 90, bDay: 90, bEveStart: 60, bEveEnd: 25
};

function two(n)
{
    return String(n).padStart(2, "0");
}

// Baut die schreibgeschuetzte Phasen-Uebersicht: Name, Zeitfenster, Helligkeit.
function renderAutoPhases()
{
    const s = schedule;
    const rows = [
        ["Hochfahren", s.tMorning, s.tDay,     "&rarr; " + s.bMorning + " %"],
        ["Tag",        s.tDay,     s.tEvening, s.bDay + " %"],
        ["Abend",      s.tEvening, s.tNight,   s.bEveStart + " &rarr; " + s.bEveEnd + " %"],
        ["Nacht",      s.tNight,   s.tMorning, "aus"]
    ];

    let html = "";
    for(let i = 0; i < rows.length; i++)
    {
        const r = rows[i];
        html +=
            "<div class='phase'>" +
                "<span class='ph-name'>" + r[0] + "</span>" +
                "<span class='ph-time'>" + two(r[1]) + ":00–" + two(r[2]) + ":00</span>" +
                "<span class='ph-val'>" + r[3] + "</span>" +
            "</div>";
    }

    document.getElementById("autoPhases").innerHTML = html;
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

    const s = schedule;

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
window.addEventListener("load", renderAutoPhases);

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
    "background_color": "#eef2f5",
    "theme_color": "#ffffff",
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
