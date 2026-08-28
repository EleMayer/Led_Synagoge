#pragma once

// AUTOMATISCH ERZEUGT aus web/ durch tools/build-web.js. Nicht von Hand
// editieren - stattdessen web/index.html, web/style.css, web/manifest.json
// oder web/icon.svg aendern und "node tools/build-web.js" erneut ausfuehren.

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

<meta name="theme-color" content="#0a0b0d">
<meta name="mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style"
      content="black-translucent">
<meta name="apple-mobile-web-app-title" content="Fassade">

<link rel="apple-touch-icon" href="/apple-touch-icon.png">
<link rel="icon" type="image/svg+xml" href="/icon.svg">

<link rel="stylesheet" href="style.css">

</head>

<body>

<header class="top">
  <div class="brand">
    <svg class="brandmark" viewBox="36 142 436 214" aria-hidden="true">
      <polyline points="56,336 128,206 192,336 264,206 328,336 398,206 452,336 452,162"
                fill="none" stroke="currentColor" stroke-width="40"
                stroke-linejoin="miter" stroke-miterlimit="10" stroke-linecap="butt"/>
    </svg>
    <strong>Museum Arbeitswelt Steyr</strong>
  </div>
  <div class="hstat">
    <span id="clock">--:--:--</span>
    <span id="modeBadge" class="badge">&ndash;</span>
    <button id="navBtn" class="hbtn" onclick="toggleView()" title="Vorschau / Preview">Vorschau</button>
    <button id="themeBtn" class="hbtn" onclick="toggleTheme()" title="Hell / Dunkel">&#9788;</button>
    <button id="langBtn" class="hbtn" onclick="toggleLang()" title="Sprache / Language">EN</button>
  </div>
</header>

<main>

<div id="viewControl">

<section class="card" id="installCard" style="display:none">
  <h2 data-i18n="install.title">Als App installieren</h2>
  <p class="hint" id="installHint" style="margin-top:0"></p>
  <button class="primary" id="installBtn" onclick="installApp()"
          style="margin-top:12px" data-i18n="install.button">Installieren</button>
</section>

<section class="card" id="modeCard">
  <h2 data-i18n="modus">Modus</h2>
  <div class="modes">
    <button class="mode" data-mode="0" onclick="sendMode(0)">Aus</button>
    <button class="mode" data-mode="1" onclick="sendMode(1)">Statisch</button>
    <button class="mode" data-mode="2" onclick="sendMode(2)">Lauflicht</button>
    <button class="mode" data-mode="3" onclick="sendMode(3)">Automatik</button>
    <button class="mode" data-mode="4" onclick="sendMode(4)">Pulsieren</button>
    <button class="mode" data-mode="5" onclick="sendMode(5)">Atmen</button>
    <button class="mode" data-mode="10" onclick="sendMode(10)">Welle</button>
    <button class="mode" data-mode="13" onclick="sendMode(13)">Sternenfunkeln</button>
    <button class="mode" data-mode="14" onclick="sendMode(14)">Treffpunkt</button>
    <button class="mode" data-mode="15" onclick="sendMode(15)">Herzschlag</button>
    <button class="mode" data-mode="16" onclick="sendMode(16)">Wechsellicht</button>
    <button class="mode" data-mode="17" onclick="sendMode(17)">Ausstrahlung</button>
    <button class="mode" data-mode="18" onclick="sendMode(18)">Wolkenzug</button>
    <button class="mode" data-mode="19" onclick="sendMode(19)">Leuchtturm</button>
    <button class="mode" data-mode="6" onclick="sendMode(6)">Dauerlicht</button>
    <button class="mode" data-mode="7" onclick="sendMode(7)">Kerzenlicht</button>
    <button class="mode" data-mode="8" onclick="sendMode(8)">Stufenlicht</button>
    <button class="mode" data-mode="9" onclick="sendMode(9)">Dämmerlicht</button>
    <button class="mode" data-mode="11" onclick="sendMode(11)">Feuerschein</button>
    <button class="mode" data-mode="12" onclick="sendMode(12)">Nachtlicht</button>
  </div>
</section>

<section class="card" id="previewCard">
  <h2 data-i18n="preview">Vorschau</h2>
  <canvas id="pvInline" class="pvinline"></canvas>
</section>

<section class="card">
  <h2 data-i18n="brightness">Helligkeit</h2>
  <div class="slider">
    <label><span data-i18n="seg.left">Segment Links</span> <span class="val"><span id="leftValue">90</span>%</span></label>
    <input id="leftSlider" type="range" min="0" max="100" value="90">
  </div>
  <div class="slider">
    <label><span data-i18n="seg.right">Segment Rechts</span> <span class="val"><span id="rightValue">90</span>%</span></label>
    <input id="rightSlider" type="range" min="0" max="100" value="90">
  </div>
  <div class="slider">
    <label><span data-i18n="seg.logo">Logo</span> <span class="val"><span id="logoValue">90</span>%</span></label>
    <input id="logoSlider" type="range" min="0" max="100" value="90">
  </div>
</section>

<section class="card" id="autoCard" style="display:none">
  <h2 data-i18n="auto">Automatik</h2>
  <div id="autoPhases" class="phases"></div>
  <div class="autonow">
    <div class="autonow-top">
      <span data-i18n="autoNow">Aktuelle Helligkeit</span>
      <span class="autonow-val"><span id="autoNowVal">&ndash;</span> %</span>
    </div>
    <div class="meter"><div class="meter-fill" id="autoMeter"></div></div>
  </div>
</section>

<section class="card">
  <h2 data-i18n="system">System</h2>
  <div id="systemStatus" class="sys"></div>
</section>

</div><!-- /viewControl -->

<div id="viewPreview" style="display:none">
  <div class="pvhead">
    <h2 id="pvTitle">Modus-Vorschau</h2>
    <p id="pvIntro" class="pvintro"></p>
  </div>
  <div id="pvSections"></div>
</div>

</main>

<footer>Museum Arbeitswelt &middot; <span data-i18n="footer">lokale Steuerung</span></footer>

<script>

let socket = null;
let reconnectTimer = null;

// Sprache (Deutsch/Englisch) und Design (hell/dunkel).

// Modus-Namen je Sprache.
const MODE_LABEL = {
  de:{0:"Aus",1:"Statisch",2:"Lauflicht",3:"Automatik",4:"Pulsieren",5:"Atmen",
      6:"Dauerlicht",7:"Kerzenlicht",8:"Stufenlicht",9:"Dämmerlicht",10:"Welle",
      11:"Feuerschein",12:"Nachtlicht",13:"Sternenfunkeln",14:"Treffpunkt",
      15:"Herzschlag",16:"Wechsellicht",17:"Ausstrahlung",18:"Wolkenzug",
      19:"Leuchtturm"},
  en:{0:"Off",1:"Static",2:"Running light",3:"Automatic",4:"Pulse",5:"Breathe",
      6:"Steady",7:"Candle",8:"Steps",9:"Dusk",10:"Wave",
      11:"Fire",12:"Night light",13:"Starlight",14:"Meeting",
      15:"Heartbeat",16:"Alternating",17:"Radiate",18:"Clouds",
      19:"Lighthouse"}
};

// Alle uebrigen Texte je Sprache.
const T = {
  de:{
    "install.title":"Als App installieren",
    "install.button":"Installieren",
    "install.hintDefault":"Diese Seite als App auf dem Startbildschirm ablegen – sie öffnet dann randlos wie eine gewohnte App.",
    "install.hintIOS":"Zum Installieren in Safari auf <b>Teilen</b> tippen und <b>„Zum Home-Bildschirm“</b> wählen.",
    "install.hintManual":"Als App ablegen: am Handy im Browser-Menü <b>„Zum Startbildschirm hinzufügen“</b>, am PC das <b>Installieren-Symbol in der Adressleiste</b> nutzen.",
    "modus":"Modus","preview":"Vorschau","brightness":"Helligkeit",
    "auto":"Automatik","autoNow":"Aktuelle Helligkeit","system":"System",
    "footer":"lokale Steuerung",
    "seg.left":"Segment Links","seg.right":"Segment Rechts","seg.logo":"Logo",
    "phase.morning":"Hochfahren","phase.day":"Tag","phase.evening":"Abend",
    "phase.night":"Nacht","phase.off":"aus",
    "sys.firmware":"Firmware","sys.time":"Uhrzeit","sys.date":"Datum","sys.rtc":"RTC",
    "sys.ntp":"NTP","sys.wifi":"WLAN","sys.ip":"IP","sys.signal":"Signal",
    "sys.hours":"Leucht-Stunden","sys.energy":"Energie",
    "val.ok":"OK","val.rtcMissing":"nicht verfügbar","val.ntpSynced":"synchronisiert",
    "val.ntpNot":"nicht synchron.","val.connected":"verbunden","val.setupAp":"Setup-AP",
    "val.disconnected":"getrennt","val.noData":"Keine Daten"
  },
  en:{
    "install.title":"Install as app",
    "install.button":"Install",
    "install.hintDefault":"Add this page to your home screen – it then opens full-screen like a normal app.",
    "install.hintIOS":"To install, tap <b>Share</b> in Safari and choose <b>“Add to Home Screen”</b>.",
    "install.hintManual":"Add as an app: on a phone use <b>“Add to Home screen”</b> in the browser menu, on a PC use the <b>install icon in the address bar</b>.",
    "modus":"Mode","preview":"Preview","brightness":"Brightness",
    "auto":"Automatic","autoNow":"Current brightness","system":"System",
    "footer":"local control",
    "seg.left":"Segment left","seg.right":"Segment right","seg.logo":"Logo",
    "phase.morning":"Ramp-up","phase.day":"Day","phase.evening":"Evening",
    "phase.night":"Night","phase.off":"off",
    "sys.firmware":"Firmware","sys.time":"Time","sys.date":"Date","sys.rtc":"RTC",
    "sys.ntp":"NTP","sys.wifi":"WiFi","sys.ip":"IP","sys.signal":"Signal",
    "sys.hours":"Lit hours","sys.energy":"Energy",
    "val.ok":"OK","val.rtcMissing":"not available","val.ntpSynced":"synced",
    "val.ntpNot":"not synced","val.connected":"connected","val.setupAp":"Setup AP",
    "val.disconnected":"disconnected","val.noData":"No data"
  }
};

let lang = "de";        // aktuelle Sprache
let theme = "dark";     // aktuelles Design
let lastData = null;    // zuletzt empfangener Status (zum Neu-Uebersetzen)
let installMode = "";   // "", "prompt", "ios" oder "manual"

// Uebersetzt einen Textschluessel in die aktuelle Sprache.
function t(key)
{
    if(T[lang] && T[lang][key]) return T[lang][key];
    return key;
}

// Setzt alle statischen Texte, die Modus-Kacheln und die dynamischen Anzeigen
// in der aktuellen Sprache.
function applyLanguage()
{
    document.documentElement.setAttribute("lang", lang);

    let nodes = document.querySelectorAll("[data-i18n]");
    for(let i = 0; i < nodes.length; i++)
    {
        nodes[i].textContent = t(nodes[i].getAttribute("data-i18n"));
    }

    let placeholders = document.querySelectorAll("[data-i18n-ph]");
    for(let i = 0; i < placeholders.length; i++)
    {
        placeholders[i].setAttribute("placeholder", t(placeholders[i].getAttribute("data-i18n-ph")));
    }

    let modeButtons = document.querySelectorAll(".mode");
    for(let i = 0; i < modeButtons.length; i++)
    {
        let m = parseInt(modeButtons[i].dataset.mode);
        modeButtons[i].textContent = MODE_LABEL[lang][m];
    }

    // Knopf zeigt die jeweils andere Sprache an.
    document.getElementById("langBtn").textContent = (lang === "de") ? "EN" : "DE";

    // Zweite Ansicht (Modus-Vorschau) mit umschalten.
    updateNavButton();
    if(pvBuilt) updatePreviewTexts();

    updateInstallHint();
    renderAutoPhases();

    if(lastData)
    {
        updateUI(lastData);
    }
}

// Wechselt zwischen Deutsch und Englisch.
function toggleLang()
{
    lang = (lang === "de") ? "en" : "de";
    try { localStorage.setItem("lang", lang); } catch(e) {}
    applyLanguage();
}

// Setzt das Design (hell/dunkel) und passt Symbol und Statusleistenfarbe an.
function applyTheme()
{
    document.documentElement.setAttribute("data-theme", theme);

    let meta = document.querySelector('meta[name="theme-color"]');
    if(meta)
    {
        meta.setAttribute("content", (theme === "light") ? "#ffffff" : "#0a0b0d");
    }

    // Sonne im dunklen Modus (tippen -> hell), Mond im hellen Modus.
    document.getElementById("themeBtn").innerHTML = (theme === "light") ? "&#9790;" : "&#9728;";
}

// Wechselt zwischen hellem und dunklem Design.
function toggleTheme()
{
    theme = (theme === "dark") ? "light" : "dark";
    try { localStorage.setItem("theme", theme); } catch(e) {}
    applyTheme();
}

// Liest gespeicherte Sprache/Design und wendet beides an.
function initPrefs()
{
    try
    {
        let savedTheme = localStorage.getItem("theme");
        if(savedTheme === "light" || savedTheme === "dark") theme = savedTheme;
        let savedLang = localStorage.getItem("lang");
        if(savedLang === "de" || savedLang === "en") lang = savedLang;
    }
    catch(e) {}

    applyTheme();
    applyLanguage();
}

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
    // Der Verbindungspunkt wurde entfernt; Funktion bleibt als No-Op bestehen.
    void ok;
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
    lastData = data;

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
        document.getElementById("modeBadge").innerText = MODE_LABEL[lang][data.mode] || "–";
        setInlineMode(data.mode);
    }
    document.getElementById("clock").innerText = data.time || "--:--:--";

    // Modus "Aus": deutlich sichtbar machen (Banner + rotes Badge).
    const isOff = (data.mode === 0);
    document.body.classList.toggle("isoff", isOff);
    document.getElementById("modeBadge").classList.toggle("off", isOff);

    updateSlider("left",   data.left);
    updateSlider("right",  data.right);
    updateSlider("logo",   data.logo);

    // Im Automatik-Modus (3) steuert die Uhrzeit die Helligkeit - die Regler
    // sind dann gesperrt. Die Automatik-Uebersicht wird nur in diesem Modus
    // eingeblendet.
    const autoActive = (data.mode === 3);
    setSlidersDisabled(autoActive);
    document.getElementById("autoCard").style.display = autoActive ? "" : "none";

    let sys = "";
    sys += kv(t("sys.firmware"), data.firmware || "-");
    sys += kv(t("sys.date"),     data.date || "--.--.----");

    if(data.rtc)
    {
        sys += kv(t("sys.rtc"), t("val.ok"), "ok");
    }
    else
    {
        sys += kv(t("sys.rtc"), t("val.rtcMissing"), "warn");
    }

    if(data.ntp)
    {
        sys += kv(t("sys.ntp"), t("val.ntpSynced"), "ok");
    }
    else
    {
        sys += kv(t("sys.ntp"), t("val.ntpNot"), "warn");
    }

    if(data.wifi)
    {
        sys += kv(t("sys.wifi"), t("val.connected"), "ok");
    }
    else if(data.ap)
    {
        sys += kv(t("sys.wifi"), t("val.setupAp"), "warn");
    }
    else
    {
        sys += kv(t("sys.wifi"), t("val.disconnected"), "warn");
    }

    sys += kv(t("sys.ip"), data.ip || "-");

    if(data.wifi && data.rssi !== undefined)
    {
        sys += kv(t("sys.signal"), data.rssi + " dBm");
    }

    if(data.onHours !== undefined)
    {
        sys += kv(t("sys.hours"), data.onHours.toFixed(1) + " h");
    }

    if(data.kWh !== undefined)
    {
        sys += kv(t("sys.energy"), "ca. " + data.kWh.toFixed(2) + " kWh");
    }

    document.getElementById("systemStatus").innerHTML = sys;

    if(data.sched)
    {
        // Das komplette Automatik-Profil (Uhrzeiten und Helligkeiten) ist fest
        // im Code (config.h) und wird nur angezeigt, nicht editiert.
        schedule = data.sched;
        renderAutoPhases();
    }

    // Aktuelle Automatik-Helligkeit als Zahl und Balken anzeigen.
    let autoValue = 0;
    if(data.autoBrightness !== undefined)
    {
        autoValue = data.autoBrightness;
    }
    document.getElementById("autoNowVal").innerText = autoValue;
    document.getElementById("autoMeter").style.width = autoValue + "%";
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

// Sperrt oder entsperrt alle Helligkeits-Regler (Segmente und Effekt).
function setSlidersDisabled(disabled)
{
    const ids = ["leftSlider", "rightSlider", "logoSlider"];
    for(let i = 0; i < ids.length; i++)
    {
        const slider = document.getElementById(ids[i]);
        if(!slider)
        {
            continue;
        }
        slider.disabled = disabled;
        slider.parentNode.classList.toggle("off", disabled);
    }
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
        [t("phase.morning"), s.tMorning, s.tDay,     "&rarr; " + s.bMorning + " %"],
        [t("phase.day"),     s.tDay,     s.tEvening, s.bDay + " %"],
        [t("phase.evening"), s.tEvening, s.tNight,   s.bEveStart + " &rarr; " + s.bEveEnd + " %"],
        [t("phase.night"),   s.tNight,   s.tMorning, t("phase.off")]
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

const sliderTypes = ["left", "right", "logo"];
for(let i = 0; i < sliderTypes.length; i++)
{
    const type = sliderTypes[i];
    const slider = document.getElementById(type + "Slider");
    slider.addEventListener("input", function()
    {
        setBrightness(type, this.value);
    });
}

// PWA-Installation.
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

// Setzt den Hinweistext der Installieren-Karte (je nach Sprache und Geraet).
function updateInstallHint()
{
    const hint = document.getElementById("installHint");
    if(!hint)
    {
        return;
    }
    if(installMode === "ios")         hint.innerHTML = t("install.hintIOS");
    else if(installMode === "manual") hint.innerHTML = t("install.hintManual");
    else                              hint.innerHTML = t("install.hintDefault");
}

// Zeigt die Installieren-Karte im passenden Modus:
//   "prompt" = Browser bietet echten Installieren-Knopf (Chrome/Edge/Android)
//   "ios"    = Safari, manueller Weg ueber "Teilen"
//   "manual" = alle anderen Browser, manueller Weg ueber das Browser-Menue
function showInstallCard(mode)
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
    installMode = mode;

    // Der Knopf funktioniert nur mit echtem Prompt, sonst nur die Anleitung.
    document.getElementById("installBtn").style.display = (mode === "prompt") ? "" : "none";
    updateInstallHint();
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
    showInstallCard("prompt");
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

// Falls der Browser keinen "beforeinstallprompt" liefert (Safari, Firefox oder
// Zugriff ueber einfaches HTTP), zeigen wir die Karte nach kurzer Wartezeit
// trotzdem - mit der passenden manuellen Anleitung. So erscheint sie ueberall.
window.addEventListener("load", function()
{
    setTimeout(function()
    {
        if(deferredInstallPrompt || isStandalone())
        {
            return;
        }
        showInstallCard(isIOS() ? "ios" : "manual");
    }, 1500);
});

window.addEventListener("load", initPrefs);
window.addEventListener("load", connect);

// ===========================================================================
//  Zweite Ansicht: Modus-Vorschau
//
//  Ueber den Navbar-Knopf erreichbar. Zeigt je Modus eine kleine animierte
//  "LED-Leiste" (raeumlich) plus eine Wellenform-Spur (Gesamthelligkeit ueber
//  die Zeit) - so bekommt jeder Modus einen eindeutigen Fingerabdruck. Reine
//  Anzeige, schaltet keine Modi. Die Navigation laeuft ueber den URL-Hash
//  (#vorschau), damit der Zurueck-Knopf des Browsers funktioniert.
// ===========================================================================

// Texte der zweiten Ansicht je Sprache.
const PV_TXT = {
  de:{ nav:"Vorschau", back:"‹ Steuerung",
       title:"Modus-Vorschau",
       intro:"Alle 20 Modi mit animierter Vorschau. Das Licht ist ausschließlich weiß – die Vorschau zeigt die Bewegung, nicht eine Farbe.",
       grund:"Grundmodi", effekt:"Effekte", stimmung:"Stimmungs-Modi",
       grundNote:"Der normale Betrieb.",
       effektNote:"Animierte Modi. Keine Nachtabschaltung.",
       stimmungNote:"Feste Lichtstimmungen. Abschaltung 23:00–06:00.",
       catGrund:"Grundmodus", catEffekt:"Effekt", catStimmung:"Stimmung" },
  en:{ nav:"Preview", back:"‹ Control",
       title:"Mode preview",
       intro:"All 20 modes with an animated preview. The light is white only – the preview shows the motion, not a colour.",
       grund:"Basic modes", effekt:"Effects", stimmung:"Mood modes",
       grundNote:"Normal operation.",
       effektNote:"Animated modes. No night switch-off.",
       stimmungNote:"Fixed light moods. Off between 23:00–06:00.",
       catGrund:"Basic", catEffekt:"Effect", catStimmung:"Mood" }
};

// Kurzbeschreibung je Modus (Nummer -> Text) je Sprache.
const PV_DESC = {
  de:{ 0:"Alle Bereiche aus.", 1:"Konstantes Weiß je Bereich.", 3:"Tageszeitabhängige Helligkeit.",
       2:"Ein Lichtschweif gleitet über die Segmente.", 4:"Ruhiges gemeinsames Auf- und Abschwellen.",
       5:"Sehr langsames Ein- und Ausatmen.", 10:"Eine Sinuswelle wandert über den Streifen.",
       13:"Grundglanz mit verglimmenden Funken.", 14:"Zwei Lichter treffen sich in der Mitte.",
       15:"Ruhiger Doppelschlag der Fassade.", 16:"Links und Rechts gegenläufig.",
       17:"Welle vom Zentrum nach außen.", 18:"Organische Helligkeit wie ziehende Wolken.",
       19:"Ein Lichtband wandert über die Linie.", 6:"Gleichmäßig, langsam atmend (hoch).",
       7:"Zwei langsam schimmernde Lichter.", 8:"Aufbau in acht Schritten.",
       9:"Sanfter, niedriger Abendglanz.", 11:"Warmes, turbulentes Lodern.", 12:"Ruhiger, niedriger Grundglanz." },
  en:{ 0:"All areas off.", 1:"Constant white per area.", 3:"Time-of-day brightness.",
       2:"A light tail glides across the segments.", 4:"Calm shared swelling up and down.",
       5:"Very slow breathing in and out.", 10:"A sine wave travels across the strip.",
       13:"Faint glow with fading sparkles.", 14:"Two lights meet in the middle.",
       15:"Calm double-beat of the facade.", 16:"Left and right in opposition.",
       17:"A wave from the centre outward.", 18:"Organic brightness like drifting clouds.",
       19:"A light band sweeps across the line.", 6:"Even, slowly breathing (high).",
       7:"Two slowly shimmering lights.", 8:"Builds up in eight steps.",
       9:"Soft, low evening glow.", 11:"Warm, turbulent flickering.", 12:"Calm, low base glow." }
};

// Reihenfolge und Gruppierung wie in modi.md.
const PV_MODES = [
  {mode:0,cat:"grund"},  {mode:1,cat:"grund"},  {mode:3,cat:"grund"},
  {mode:2,cat:"effekt"}, {mode:4,cat:"effekt"}, {mode:5,cat:"effekt"},  {mode:10,cat:"effekt"},
  {mode:13,cat:"effekt"},{mode:14,cat:"effekt"},{mode:15,cat:"effekt"}, {mode:16,cat:"effekt"},
  {mode:17,cat:"effekt"},{mode:18,cat:"effekt"},{mode:19,cat:"effekt"},
  {mode:6,cat:"stimmung"},{mode:7,cat:"stimmung"},{mode:8,cat:"stimmung"},
  {mode:9,cat:"stimmung"},{mode:11,cat:"stimmung"},{mode:12,cat:"stimmung"}
];

const PV_CELLS = 24;     // LED-Punkte je Vorschau-Leiste
const PV_HIST  = 100;    // Laenge der Wellenform-Spur (Samples ueber die Zeit)
let pvItems = [];        // je Karte: { mode, cat, canvas, ctx, spark[], hist[], nameEl, catEl, descEl }
let pvBuilt = false;
let pvVisible = false;
let pvLast = 0;
let pvResizeAt = 0;

// Baut die drei Abschnitte (Grundmodi, Effekte, Stimmungs-Modi) mit Karten.
function buildPreview()
{
    let host = document.getElementById("pvSections");

    let groups = [
        { cat:"grund",    title:"grund",    note:"grundNote" },
        { cat:"effekt",   title:"effekt",   note:"effektNote" },
        { cat:"stimmung", title:"stimmung", note:"stimmungNote" }
    ];

    for(let g = 0; g < groups.length; g++)
    {
        let sec = document.createElement("div");
        sec.className = "pvsec";

        let h3 = document.createElement("h3");
        h3.dataset.pv = groups[g].title;
        sec.appendChild(h3);

        let note = document.createElement("p");
        note.className = "pvnote";
        note.dataset.pv = groups[g].note;
        sec.appendChild(note);

        let grid = document.createElement("div");
        grid.className = "pvgrid";

        for(let k = 0; k < PV_MODES.length; k++)
        {
            if(PV_MODES[k].cat !== groups[g].cat) continue;
            grid.appendChild(buildPvCard(PV_MODES[k]));
        }

        sec.appendChild(grid);
        host.appendChild(sec);
    }

    updatePreviewTexts();
}

// Baut eine einzelne Vorschau-Karte (Leiste + Nummer, Name, Art, Text).
function buildPvCard(m)
{
    let card = document.createElement("div");
    card.className = "pvcard";

    let canvas = document.createElement("canvas");
    canvas.className = "pv";
    card.appendChild(canvas);

    let row = document.createElement("div");
    row.className = "pvrow";

    let num = document.createElement("span");
    num.className = "pvnum";
    num.textContent = m.mode;

    let name = document.createElement("span");
    name.className = "pvname";

    let cat = document.createElement("span");
    cat.className = "pvcat";

    row.appendChild(num);
    row.appendChild(name);
    row.appendChild(cat);
    card.appendChild(row);

    let desc = document.createElement("p");
    desc.className = "pvdesc";
    card.appendChild(desc);

    let spark = [];
    for(let x = 0; x < PV_CELLS; x++) spark.push(0);
    let hist = [];
    for(let x = 0; x < PV_HIST; x++) hist.push(0);

    pvItems.push({
        mode:m.mode, cat:m.cat, canvas:canvas, ctx:canvas.getContext("2d"),
        spark:spark, hist:hist, nameEl:name, catEl:cat, descEl:desc
    });

    return card;
}

// Setzt Titel/Notizen/Namen/Beschreibungen der Vorschau in der aktuellen Sprache.
function updatePreviewTexts()
{
    let x = PV_TXT[lang];

    let title = document.getElementById("pvTitle");
    let intro = document.getElementById("pvIntro");
    if(title) title.textContent = x.title;
    if(intro) intro.textContent = x.intro;

    let heads = document.querySelectorAll("[data-pv]");
    for(let i = 0; i < heads.length; i++)
    {
        heads[i].textContent = x[heads[i].dataset.pv];
    }

    let catName = { grund:x.catGrund, effekt:x.catEffekt, stimmung:x.catStimmung };
    for(let i = 0; i < pvItems.length; i++)
    {
        let it = pvItems[i];
        it.nameEl.textContent = MODE_LABEL[lang][it.mode];
        it.catEl.textContent = catName[it.cat];
        it.descEl.textContent = PV_DESC[lang][it.mode];
    }

    updateNavButton();
}

// Passt die Canvas-Aufloesung an die reale Anzeigegroesse an (scharfe Punkte).
function resizePreviews()
{
    let dpr = window.devicePixelRatio || 1;
    for(let i = 0; i < pvItems.length; i++)
    {
        let c = pvItems[i].canvas;
        let w = Math.round((c.clientWidth || 240) * dpr);
        let h = Math.round((c.clientHeight || 46) * dpr);
        if(w > 0 && (c.width !== w || c.height !== h))
        {
            c.width = w;
            c.height = h;
        }
    }
}

// Weiche Schwingung 0..1 mit der Periode periodMs.
function pvOsc(t, periodMs)
{
    return 0.5 - 0.5 * Math.cos(2 * Math.PI * ((t % periodMs) / periodMs));
}

// Weiches Rausch-Signal 0..1 (fuer Kerze/Feuer/Wolken - ohne echtes Perlin).
function pvNoise(x, t)
{
    let a = Math.sin(x * 1.7 + t * 0.0021);
    let b = Math.sin(x * 0.9 - t * 0.0013 + 2.0);
    return 0.5 + 0.5 * a * b;
}

// Dreiecks-"Schlag" 0..peak fuer den Herzschlag (0 ausserhalb der Breite).
function pvBump(t, center, width, peak)
{
    let d = Math.abs(t - center);
    if(d >= width) return 0;
    return peak * (1 - d / width);
}

// Berechnet die Helligkeit (0..1) je LED-Punkt fuer einen Modus zum Zeitpunkt t.
function pvValues(p, t)
{
    let N = PV_CELLS;
    let half = N / 2;
    let v = [];
    let i;
    for(i = 0; i < N; i++) v.push(0);

    switch(p.mode)
    {
        case 0:   // Aus
            break;

        case 1:   // Statisch - konstant hell, keine Bewegung
            for(i = 0; i < N; i++) v[i] = 0.90;
            break;

        case 3:   // Automatik - ruhiger, mittlerer Dauerwert
            for(i = 0; i < N; i++) v[i] = 0.60 + 0.06 * pvOsc(t, 14000);
            break;

        case 4:   // Pulsieren - deutlicher, zuegiger Puls der ganzen Flaeche
        {
            let lv = 0.35 + 0.65 * pvOsc(t, 1900);
            for(i = 0; i < N; i++) v[i] = lv;
            break;
        }

        case 5:   // Atmen - sehr langsames, weites Ein-/Ausatmen
        {
            let lv = 0.22 + 0.73 * pvOsc(t, 7000);
            for(i = 0; i < N; i++) v[i] = lv;
            break;
        }

        case 6:   // Dauerlicht - hoch, mit sichtbarem ruhigem Atmen
        {
            let lv = 0.70 + 0.20 * pvOsc(t, 8000);
            for(i = 0; i < N; i++) v[i] = lv;
            break;
        }

        case 9:   // Daemmerlicht - niedriger Abendglanz, langsam schwellend
        {
            let lv = 0.16 + 0.28 * pvOsc(t, 6000);
            for(i = 0; i < N; i++) v[i] = lv;
            break;
        }

        case 2:   // Lauflicht - heller Kopf mit ausklingendem Schweif
        {
            let pos = (t / 70) % N;
            for(i = 0; i < N; i++)
            {
                let d = (pos - i + N) % N;
                v[i] = Math.max(0.03, 1 - d * 0.22);
            }
            break;
        }

        case 14:  // Treffpunkt - zwei Koepfe laufen zur Mitte
        {
            let pos = (t / 85) % (half + 1);
            let lp = pos;
            let rp = (N - 1) - pos;
            for(i = 0; i < N; i++)
            {
                let d = Math.min(Math.abs(i - lp), Math.abs(i - rp));
                v[i] = Math.max(0.03, 1 - d * 0.30);
            }
            break;
        }

        case 10:  // Welle - mehrere raeumliche Wellen wandern
            for(i = 0; i < N; i++)
                v[i] = 0.12 + 0.88 * (0.5 + 0.5 * Math.sin(i * 0.85 - t / 170));
            break;

        case 17:  // Ausstrahlung - symmetrische Welle aus der Mitte nach aussen
        {
            let c = (N - 1) / 2;
            for(i = 0; i < N; i++)
            {
                let d = Math.abs(i - c);
                v[i] = 0.12 + 0.88 * (0.5 + 0.5 * Math.sin(d * 0.75 - t / 190));
            }
            break;
        }

        case 16:  // Wechsellicht - Links/Rechts stark gegenlaeufig
        {
            let a = 0.12 + 0.88 * pvOsc(t, 5000);
            let b = 0.12 + 0.88 * (1 - pvOsc(t, 5000));
            for(i = 0; i < N; i++) v[i] = (i < half) ? a : b;
            break;
        }

        case 7:   // Kerzenlicht - zwei Flammen (Haelften) schimmern als Bloecke
        {
            let flameL = 0.52 + 0.30 * pvNoise(3,  t * 0.9);
            let flameR = 0.52 + 0.30 * pvNoise(60, t * 0.9);
            for(i = 0; i < N; i++)
            {
                let base = (i < half) ? flameL : flameR;
                v[i] = base + 0.06 * pvNoise(i * 2.0, t * 1.6);
            }
            break;
        }

        case 11:  // Feuerschein - turbulentes Lodern mit einzelnen hellen Zungen
            for(i = 0; i < N; i++)
            {
                let turb  = pvNoise(i * 1.9 + 5, t * 2.6) * pvNoise(i * 0.8 + 12, t * 1.5);
                let flare = Math.pow(pvNoise(i * 3.1 + 20, t * 3.0), 3);
                v[i] = 0.40 + 0.45 * turb + 0.35 * flare;
            }
            break;

        case 12:  // Nachtlicht - gleichmaessiger, sehr niedriger Grundglanz
        {
            let lv = 0.10 + 0.06 * pvOsc(t, 11000);
            for(i = 0; i < N; i++) v[i] = lv;
            break;
        }

        case 18:  // Wolkenzug - grosse, weiche Helligkeitsinseln ziehen langsam
            for(i = 0; i < N; i++)
            {
                let n = 0.5 + 0.5 * Math.sin(i * 0.28 - t * 0.0009) * Math.sin(i * 0.10 + t * 0.0004 + 1.3);
                v[i] = 0.18 + 0.82 * n;
            }
            break;

        case 8:   // Stufenlicht - Aufbau in acht Schritten von aussen nach innen
        {
            let cycle = 7000;
            let ph = (t % cycle) / cycle;
            let steps = Math.min(8, Math.floor(ph * 9));
            let lit = Math.round(steps / 8 * half);
            for(i = 0; i < N; i++)
            {
                let fromOuter = (i < half) ? i : (N - 1 - i);
                v[i] = (fromOuter < lit) ? 0.92 : 0.06;
            }
            break;
        }

        case 15:  // Herzschlag - kraeftiger Doppelschlag auf niedrigem Grund
        {
            let tt = t % 2000;
            let b1 = pvBump(tt, 110, 150, 1.0);
            let b2 = pvBump(tt, 380, 190, 0.8);
            let lv = Math.max(0.14, Math.max(b1, b2));
            for(i = 0; i < N; i++) v[i] = lv;
            break;
        }

        case 19:  // Leuchtturm - ein helles Band wandert ueber niedrigem Grund
        {
            let period = 3800;
            let pos = ((t % period) / period) * N;
            let width = 3.2;
            let floor = 0.14;
            for(i = 0; i < N; i++)
            {
                let d = Math.abs(i - pos);
                let lv = floor;
                if(d < width) lv = floor + (1 - floor) * (width - d) / width;
                v[i] = lv;
            }
            break;
        }

        case 13:  // Sternenfunkeln - dunkler Grund mit hellen, verglimmenden Funken
        {
            for(i = 0; i < N; i++)
            {
                p.spark[i] *= 0.86;
                if(p.spark[i] < 0.10) p.spark[i] = 0.10;
                v[i] = p.spark[i];
            }
            if(Math.random() < 0.22) p.spark[Math.floor(Math.random() * N)] = 1.0;
            break;
        }

        default:
            for(i = 0; i < N; i++) v[i] = 0.6;
            break;
    }

    return v;
}

// Zeichnet oben die raeumliche LED-Leiste, unten die Wellenform-Spur.
function pvDraw(p, vals)
{
    let ctx = p.ctx;
    let w = p.canvas.width;
    let h = p.canvas.height;
    let N = vals.length;

    ctx.clearRect(0, 0, w, h);

    let stripH   = Math.round(h * 0.55);
    let traceTop = Math.round(h * 0.64);
    let traceH   = h - traceTop - Math.round(h * 0.06);
    let baseY    = traceTop + traceH;

    // LED-Leiste (raeumlich)
    let gap = Math.max(1, w * 0.006);
    let cw = (w - (N - 1) * gap) / N;
    let pad = Math.round(stripH * 0.12);

    for(let i = 0; i < N; i++)
    {
        let val = vals[i];
        if(val < 0) val = 0;
        if(val > 1) val = 1;

        let x = i * (cw + gap);

        ctx.fillStyle = "rgba(255,245,230,0.05)";
        ctx.fillRect(x, pad, cw, stripH - 2 * pad);

        if(val > 0.02)
        {
            ctx.fillStyle = "rgba(255,247,235," + (0.12 + 0.88 * val) + ")";
            ctx.fillRect(x, pad, cw, stripH - 2 * pad);
        }
    }

    // Wellenform-Spur (Gesamthelligkeit ueber die Zeit)
    let hist = p.hist;
    let L = hist.length;

    ctx.strokeStyle = "rgba(255,255,255,0.10)";
    ctx.lineWidth = Math.max(1, h * 0.018);
    ctx.beginPath();
    ctx.moveTo(0, baseY);
    ctx.lineTo(w, baseY);
    ctx.stroke();

    ctx.strokeStyle = "rgba(120,220,232,0.95)";
    ctx.lineWidth = Math.max(1.5, h * 0.05);
    ctx.lineJoin = "round";
    ctx.lineCap = "round";
    ctx.beginPath();
    for(let i = 0; i < L; i++)
    {
        let x = (L > 1) ? (i / (L - 1)) * w : 0;
        let val = hist[i];
        if(val < 0) val = 0;
        if(val > 1) val = 1;
        let y = baseY - val * traceH;
        if(i === 0) ctx.moveTo(x, y);
        else ctx.lineTo(x, y);
    }
    ctx.stroke();
}

// Inline-Vorschau des AKTUELLEN Modus (Karte "Vorschau" in der Steuerung):
// zeigt Segment Links | Logo | Segment Rechts als Live-Streifen plus die
// Wellenform-Spur - so ist auch dort jeder Modus eindeutig erkennbar.
let inlinePv = null;

// Legt die Inline-Vorschau auf dem Canvas #pvInline an.
function buildInlinePreview()
{
    let canvas = document.getElementById("pvInline");
    if(!canvas) return;

    let spark = [];
    for(let k = 0; k < PV_CELLS; k++) spark.push(0);
    let hist = [];
    for(let k = 0; k < PV_HIST; k++) hist.push(0);

    inlinePv = { mode:3, canvas:canvas, ctx:canvas.getContext("2d"), spark:spark, hist:hist };
    resizeInline();
}

// Liest die aktuellen Regler-Werte (0..1) fuer die Inline-Vorschau. So geht die
// Lichtstaerke im Vorschau-Streifen mit, wenn man die Regler bewegt.
function inlineScales()
{
    function val(id)
    {
        let el = document.getElementById(id);
        let n = el ? parseInt(el.value) : 100;
        if(isNaN(n)) n = 100;
        if(n < 0) n = 0;
        if(n > 100) n = 100;
        return n / 100;
    }
    return { left: val("leftSlider"), right: val("rightSlider"), logo: val("logoSlider") };
}

// Setzt den Modus der Inline-Vorschau (bei Wechsel Spur/Funken zuruecksetzen).
function setInlineMode(m)
{
    if(!inlinePv || m === undefined || inlinePv.mode === m) return;
    inlinePv.mode = m;
    for(let i = 0; i < inlinePv.spark.length; i++) inlinePv.spark[i] = 0;
    inlinePv.hist = [];
    for(let i = 0; i < PV_HIST; i++) inlinePv.hist.push(0);
}

// Canvas-Aufloesung der Inline-Vorschau an die Anzeigegroesse anpassen.
function resizeInline()
{
    if(!inlinePv) return;
    let dpr = window.devicePixelRatio || 1;
    let c = inlinePv.canvas;
    let w = Math.round((c.clientWidth || 300) * dpr);
    let h = Math.round((c.clientHeight || 64) * dpr);
    if(w > 0 && (c.width !== w || c.height !== h))
    {
        c.width = w;
        c.height = h;
    }
}

// Zeichnet die Inline-Vorschau: zwei Segmente, Logo-Punkt in der Mitte, Spur.
// vals sind bereits mit den Reglern skaliert; logoVal ist die Logo-Helligkeit.
function pvDrawInline(p, vals, logoVal)
{
    let ctx = p.ctx;
    let w = p.canvas.width;
    let h = p.canvas.height;
    let N = vals.length;
    let half = Math.floor(N / 2);

    ctx.clearRect(0, 0, w, h);

    let stripH   = Math.round(h * 0.52);
    let traceTop = Math.round(h * 0.62);
    let traceH   = h - traceTop - Math.round(h * 0.05);
    let baseY    = traceTop + traceH;

    // Platz fuer den Logo-Punkt in der Mitte.
    let dotR   = Math.round(stripH * 0.40);
    let dotCx  = w / 2;
    let dotCy  = Math.round(stripH / 2);
    let dotGap = dotR * 2 + Math.round(w * 0.03);
    let segW   = (w - dotGap) / 2;

    let pad = Math.round(stripH * 0.16);
    let gap = Math.max(1, segW * 0.012);
    let cw  = (segW - (half - 1) * gap) / half;

    function cell(x, val)
    {
        if(val < 0) val = 0;
        if(val > 1) val = 1;
        ctx.fillStyle = "rgba(255,245,230,0.05)";
        ctx.fillRect(x, pad, cw, stripH - 2 * pad);
        if(val > 0.02)
        {
            ctx.fillStyle = "rgba(255,247,235," + (0.12 + 0.88 * val) + ")";
            ctx.fillRect(x, pad, cw, stripH - 2 * pad);
        }
    }

    // linkes Segment (Zellen 0..half-1)
    for(let i = 0; i < half; i++) cell(i * (cw + gap), vals[i]);
    // rechtes Segment (Zellen half..N-1)
    let rx = segW + dotGap;
    for(let i = 0; i < half; i++) cell(rx + i * (cw + gap), vals[half + i]);

    // Logo-Punkt = mit dem Logo-Regler skalierte Helligkeit.
    let lv = logoVal;
    if(lv < 0) lv = 0;
    if(lv > 1) lv = 1;

    ctx.beginPath();
    ctx.arc(dotCx, dotCy, dotR, 0, 2 * Math.PI);
    ctx.fillStyle = "rgba(255,245,230,0.06)";
    ctx.fill();
    if(lv > 0.02)
    {
        ctx.beginPath();
        ctx.arc(dotCx, dotCy, dotR, 0, 2 * Math.PI);
        ctx.fillStyle = "rgba(255,247,235," + (0.12 + 0.88 * lv) + ")";
        ctx.fill();
    }

    // Wellenform-Spur (Gesamthelligkeit ueber die Zeit)
    let hist = p.hist;
    let L = hist.length;

    ctx.strokeStyle = "rgba(255,255,255,0.10)";
    ctx.lineWidth = Math.max(1, h * 0.014);
    ctx.beginPath();
    ctx.moveTo(0, baseY);
    ctx.lineTo(w, baseY);
    ctx.stroke();

    ctx.strokeStyle = "rgba(120,220,232,0.95)";
    ctx.lineWidth = Math.max(1.5, h * 0.04);
    ctx.lineJoin = "round";
    ctx.lineCap = "round";
    ctx.beginPath();
    for(let i = 0; i < L; i++)
    {
        let x = (L > 1) ? (i / (L - 1)) * w : 0;
        let val = hist[i];
        if(val < 0) val = 0;
        if(val > 1) val = 1;
        let y = baseY - val * traceH;
        if(i === 0) ctx.moveTo(x, y);
        else ctx.lineTo(x, y);
    }
    ctx.stroke();
}

// Eine Animationsschleife fuer alle Vorschauen (~30 fps, pausiert im Hintergrund):
// auf der zweiten Seite die 20 Kacheln, sonst die Inline-Vorschau der Steuerung.
function pvFrame(now)
{
    requestAnimationFrame(pvFrame);

    if(document.hidden) return;
    if(now - pvLast < 33) return;
    pvLast = now;

    if(now - pvResizeAt > 500)
    {
        pvResizeAt = now;
        if(pvVisible) resizePreviews();
        else resizeInline();
    }

    if(pvVisible)
    {
        // Zweite Seite: alle 20 Kacheln.
        for(let i = 0; i < pvItems.length; i++)
        {
            let p = pvItems[i];
            let vals = pvValues(p, now);

            let sum = 0;
            for(let k = 0; k < vals.length; k++) sum += vals[k];
            p.hist.push(sum / vals.length);
            if(p.hist.length > PV_HIST) p.hist.shift();

            pvDraw(p, vals);
        }
    }
    else if(inlinePv)
    {
        // Steuerung: Inline-Vorschau des aktuellen Modus, mit den Reglern skaliert.
        let p = inlinePv;
        let vals = pvValues(p, now);
        let sc = inlineScales();
        let half = Math.floor(vals.length / 2);

        // Muster je Segment mit dem passenden Regler (Links/Rechts) skalieren.
        let patternSum = 0;
        for(let k = 0; k < vals.length; k++)
        {
            patternSum += vals[k];
            vals[k] = vals[k] * (k < half ? sc.left : sc.right);
        }

        // Logo folgt dem Logo-Regler (Grundhelligkeit = Muster-Durchschnitt).
        let logoVal = (patternSum / vals.length) * sc.logo;

        // Spur = tatsaechliche (skalierte) Gesamthelligkeit der Segmente.
        let sum = 0;
        for(let k = 0; k < vals.length; k++) sum += vals[k];
        p.hist.push(sum / vals.length);
        if(p.hist.length > PV_HIST) p.hist.shift();

        pvDrawInline(p, vals, logoVal);
    }
}

// Aktualisiert die Beschriftung des Navbar-Knopfes (zeigt das jeweilige Ziel).
function updateNavButton()
{
    let btn = document.getElementById("navBtn");
    if(!btn) return;
    let x = PV_TXT[lang];
    btn.textContent = (location.hash === "#vorschau") ? x.back : x.nav;
}

// Zeigt die gewuenschte Ansicht ("control" oder "preview").
function showView(name)
{
    let ctrl = document.getElementById("viewControl");
    let prev = document.getElementById("viewPreview");

    if(name === "preview")
    {
        if(!pvBuilt)
        {
            buildPreview();
            pvBuilt = true;
        }
        ctrl.style.display = "none";
        prev.style.display = "";
        pvVisible = true;
        resizePreviews();
        try { window.scrollTo(0, 0); } catch(e) {}
    }
    else
    {
        prev.style.display = "none";
        ctrl.style.display = "";
        pvVisible = false;
    }

    updateNavButton();
}

// Liest den URL-Hash und schaltet die passende Ansicht.
function router()
{
    showView((location.hash === "#vorschau") ? "preview" : "control");
}

// Navbar-Knopf: wechselt die Ansicht ueber den Hash (Browser-Zurueck moeglich).
function toggleView()
{
    if(location.hash === "#vorschau") location.hash = "";
    else location.hash = "vorschau";
}

window.addEventListener("hashchange", router);
window.addEventListener("resize", function(){ if(pvVisible) resizePreviews(); else resizeInline(); });
window.addEventListener("load", function()
{
    buildInlinePreview();
    router();
    requestAnimationFrame(pvFrame);
});

</script>

</body>
</html>
)rawliteral";

const char style_css[] PROGMEM = R"rawliteral(

:root{

  /* Dunkles (schwarzes) Design - Standard */
  --bg:#0a0b0d;
  --offbg:#4a0f16;   /* Modus-Karte im Zustand "Aus" (deutlich rot) */
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
  --btn-bg:#171a1f;
  --input-bg:#0e1013;
  --track:#2a2f37;
  --thumb:#e7ecf1;
  --top-bg:rgba(10,11,13,.9);
  --radius:12px;
  --shadow:0 1px 2px rgba(0,0,0,.4);
}

:root[data-theme="light"]{

  /* Helles (klinisches) Design */
  --bg:#eef2f5;
  --offbg:#f6d9dc;   /* Modus-Karte im Zustand "Aus" */
  --card:#ffffff;
  --line:#e6ebf0;
  --line-strong:#d3dbe2;
  --fg:#1b2733;
  --muted:#68737f;
  --accent:#0e8a94;
  --accent-weak:#e4f3f4;
  --accent-ink:#ffffff;
  --fill:#0e8a94;
  --ok:#2f8a5b;
  --warn:#b07d1e;
  --bad:#c0505a;
  --btn-bg:#ffffff;
  --input-bg:#ffffff;
  --track:#e1e7ed;
  --thumb:#ffffff;
  --top-bg:rgba(255,255,255,.92);
  --shadow:0 1px 2px rgba(20,40,60,.05);
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
  flex-wrap:wrap; gap:8px 12px;
  padding:16px 20px; background:var(--top-bg); backdrop-filter:blur(8px);
  border-bottom:1px solid var(--line);
}
.brand{display:flex; align-items:center; gap:9px; font-size:15px; font-weight:600; letter-spacing:.01em}
.brandmark{width:30px; height:15px; flex:none; display:block}
.hstat{display:flex;align-items:center;gap:10px}
#clock{font-variant-numeric:tabular-nums;color:var(--muted)}
.badge{background:var(--accent-weak);border:1px solid var(--accent-weak);border-radius:999px;
  padding:4px 12px;font-size:12px;font-weight:600;color:var(--accent)}
.hbtn{background:transparent;border:1px solid var(--line-strong);border-radius:8px;
  color:var(--muted);font-size:12px;font-weight:600;padding:5px 9px;cursor:pointer;
  min-width:34px;transition:.12s}
.hbtn:hover{border-color:var(--accent);color:var(--accent)}
.badge.off{background:transparent;border-color:var(--bad);color:var(--bad)}


main{max-width:600px;margin:0 auto;padding:16px;display:grid;gap:12px}

.card{background:var(--card);border:1px solid var(--line);border-radius:var(--radius);
  padding:15px 16px;box-shadow:var(--shadow);transition:background .3s ease,border-color .3s ease}
.card h2{margin:0 0 11px;font-size:11px;color:var(--muted);
  text-transform:uppercase;letter-spacing:.1em;font-weight:700}

/* Modus "Aus": die Modus-Karte und der aktive "Aus"-Button werden rot. */
body.isoff #modeCard{background:var(--offbg);border-color:var(--bad)}
body.isoff #modeCard h2{color:var(--bad)}
body.isoff .modes button.active{background:var(--bad);border-color:var(--bad);color:#fff}

.modes{display:grid;grid-template-columns:repeat(2,1fr);gap:8px}
.modes button{
  padding:11px;border:1px solid var(--line-strong);border-radius:9px;
  background:var(--btn-bg);color:var(--fg);font-size:15px;cursor:pointer;
  transition:.12s;font-weight:500}
.modes button:hover{border-color:var(--accent);color:var(--accent)}
.modes button.active{
  background:var(--accent);color:var(--accent-ink);border-color:var(--accent);
  font-weight:600}
.modes button:active{transform:scale(.99)}

.slider{margin:12px 0}
.slider:first-of-type{margin-top:4px}
.slider label{display:flex;justify-content:space-between;color:var(--muted);
  font-size:14px;margin-bottom:8px}
.slider .val{color:var(--fg);font-variant-numeric:tabular-nums;font-weight:600}
input[type=range]{-webkit-appearance:none;appearance:none;width:100%;height:24px;
  background:transparent;cursor:pointer}
input[type=range]::-webkit-slider-runnable-track{height:5px;border-radius:5px;background:var(--track)}
input[type=range]::-moz-range-track{height:5px;border-radius:5px;background:var(--track)}
input[type=range]::-moz-range-progress{height:5px;border-radius:5px;background:var(--fill)}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;appearance:none;
  width:20px;height:20px;border-radius:50%;background:var(--thumb);border:2px solid var(--accent);
  margin-top:-8px;box-shadow:0 1px 2px rgba(0,0,0,.3)}
input[type=range]::-moz-range-thumb{width:20px;height:20px;border-radius:50%;
  background:var(--thumb);border:2px solid var(--accent);box-shadow:0 1px 2px rgba(0,0,0,.3)}
input[type=range]:disabled{cursor:not-allowed}
.slider.off{opacity:.4}

button.primary{background:var(--accent);color:var(--accent-ink);border:none;border-radius:9px;
  padding:12px 16px;font-weight:600;cursor:pointer;transition:.12s;width:100%}
button.primary:hover{filter:brightness(1.05)}
input[type=text]{width:100%;padding:11px 12px;border-radius:9px;
  border:1px solid var(--line-strong);background:var(--input-bg);color:var(--fg);font-size:15px}
input[type=text]:focus{outline:none;border-color:var(--accent);
  box-shadow:0 0 0 3px var(--accent-weak)}

.phases{margin-top:2px}
.phase{display:grid;grid-template-columns:1fr auto auto;gap:14px;align-items:baseline;
  padding:9px 0;border-bottom:1px solid var(--line)}
.phase:last-child{border-bottom:none}
.phase .ph-name{color:var(--fg)}
.phase .ph-time{color:var(--muted);font-variant-numeric:tabular-nums;font-size:13px}
.phase .ph-val{color:var(--fg);font-weight:600;font-variant-numeric:tabular-nums;
  text-align:right;min-width:84px}

.autonow{margin-top:16px}
.autonow-top{display:flex;justify-content:space-between;align-items:baseline;
  color:var(--muted);font-size:14px;margin-bottom:8px}
.autonow-val{color:var(--fg);font-weight:700;font-size:16px;font-variant-numeric:tabular-nums}
.meter{height:8px;border-radius:6px;background:var(--track);overflow:hidden}
.meter-fill{height:100%;width:0;background:var(--fill);border-radius:6px;transition:width .4s ease}

.sys{display:grid;gap:0}
.kv{display:flex;justify-content:space-between;padding:8px 0;border-bottom:1px solid var(--line);gap:12px}
.kv:last-child{border-bottom:none}
.kv span{color:var(--muted)}
.kv b{font-weight:600;font-variant-numeric:tabular-nums;text-align:right;word-break:break-all}
.kv b.ok{color:var(--ok)}
.kv b.warn{color:var(--warn)}

.hint{color:var(--muted);font-size:13px;margin:12px 0 0}
.hint b{color:var(--fg);font-weight:600}
footer{text-align:center;color:var(--muted);font-size:12px;padding:24px}

/* ===== Zweite Ansicht: Modus-Vorschau (ueber den Navbar-Knopf) ===== */
#navBtn{color:var(--accent);border-color:var(--accent)}
.pvhead{padding:2px 2px 0}
.pvhead h2{margin:0;font-size:16px;font-weight:650}
.pvintro{color:var(--muted);font-size:13px;margin:8px 0 0;line-height:1.5}
.pvsec{margin-top:18px}
.pvsec > h3{font-size:11px;color:var(--muted);text-transform:uppercase;letter-spacing:.12em;
  font-weight:700;margin:0 2px 4px;border-bottom:1px solid var(--line);padding-bottom:7px}
.pvnote{color:var(--muted);font-size:12px;margin:0 2px 10px}
.pvgrid{display:grid;grid-template-columns:repeat(auto-fill,minmax(240px,1fr));gap:10px}
.pvcard{background:var(--card);border:1px solid var(--line);border-radius:11px;
  padding:11px 12px 12px;box-shadow:var(--shadow)}
.pvcard .pv{display:block;width:100%;height:46px;border-radius:7px;
  background:#0c0e11;border:1px solid #05070a}
.pvrow{display:flex;align-items:center;gap:8px;margin-top:9px}
.pvnum{font-variant-numeric:tabular-nums;color:var(--muted);font-size:12px;font-weight:600;min-width:18px}
.pvname{font-weight:600;font-size:14.5px;flex:1}
.pvcat{font-size:10px;font-weight:700;letter-spacing:.03em;padding:3px 8px;border-radius:999px;
  background:var(--accent-weak);color:var(--accent)}
.pvdesc{color:var(--muted);font-size:12.5px;margin:7px 1px 0;line-height:1.45}

/* Inline-Vorschau (aktueller Modus) in der Steuerung: Segmente + Logo + Spur */
.pvinline{display:block;width:100%;height:64px;border-radius:8px;
  background:#0c0e11;border:1px solid #05070a;margin-top:2px}
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
    "background_color": "#0a0b0d",
    "theme_color": "#0a0b0d",
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

<!-- Museum Arbeitswelt - Bildmarke (Zickzack mit hohem Balken rechts) -->
<rect width="512" height="512" rx="96" fill="#ffffff"/>

<polyline points="56,336 128,206 192,336 264,206 328,336 398,206 452,336 452,162"
          fill="none" stroke="#111111" stroke-width="40"
          stroke-linejoin="miter" stroke-miterlimit="10" stroke-linecap="butt"/>

</svg>
)rawliteral";
