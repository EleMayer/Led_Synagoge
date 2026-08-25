// ============================================================================
//  Erzeugt eine eigenstaendige Demo-Seite (src/web_page_demo.html) aus
//  src/web_page.h.
//
//  Die Demo laeuft OHNE ESP32 und OHNE Server: ein eingebauter Simulator
//  ersetzt die WebSocket-Verbindung. Einfach src/web_page_demo.html im
//  Browser oeffnen.
//
//  Start:  node tools/build-demo.js   ->  erzeugt src/web_page_demo.html
// ============================================================================
const fs = require('fs');
const path = require('path');

const WEB_PAGE_H = path.join(__dirname, '..', 'src', 'web_page.h');
const OUT = path.join(__dirname, '..', 'src', 'web_page_demo.html');

const src = fs.readFileSync(WEB_PAGE_H, 'utf8');
const m = /const char index_html\[\] PROGMEM = R"rawliteral\(([\s\S]*?)\)rawliteral";/.exec(src);
if (!m) {
    console.error('Fehler: index_html in web_page.h nicht gefunden.');
    process.exit(1);
}
let html = m[1];

// Manifest/Icon verweisen auf den ESP32-Server -> in der lokalen Datei entfernen.
html = html.replace(/<link rel="manifest"[^>]*>/g, '');
html = html.replace(/<link rel="apple-touch-icon"[^>]*>/g, '');
html = html.replace(/<link rel="icon"[^>]*>/g, '');

// Simulator: ersetzt window.WebSocket, damit die Seite ohne ESP32 laeuft.
// Er haelt einen einfachen Zustand und schickt ihn - wie der echte Controller -
// als JSON zurueck. Bewusst schlicht und gut lesbar geschrieben.
const simulator = `
<script>
(function()
{
    let state = {
        mode: 3,
        left: 80, right: 80, logo: 80, global: 80,
        autoBrightness: 0,
        rtc: true, ntp: true,
        ip: 'Demo', rssi: -55, wifi: true, ap: false,
        ssid: 'Demo (ohne ESP32)', firmware: 'Demo',
        sched: { tMorning: 6, tDay: 8, tEvening: 18, tNight: 23,
                 bMorning: 90, bDay: 90, bEveStart: 60, bEveEnd: 25 }
    };

    let overrideActive = false;
    let overrideWindow = -1;

    // Begrenzt einen Wert auf den Bereich lo..hi.
    function clamp(value, lo, hi)
    {
        value = parseInt(value);
        if(isNaN(value)) return lo;
        if(value < lo) return lo;
        if(value > hi) return hi;
        return value;
    }

    // Minuten seit Mitternacht (aktuelle Uhrzeit).
    function nowMinutes()
    {
        let d = new Date();
        return d.getHours() * 60 + d.getMinutes();
    }

    // Aktuelles Zeitfenster: 0 = Nacht, 1 = Morgen, 2 = Tag, 3 = Abend.
    function currentWindow()
    {
        let t = nowMinutes();
        let s = state.sched;
        if(t >= s.tNight * 60 || t < s.tMorning * 60) return 0;
        if(t < s.tDay * 60) return 1;
        if(t < s.tEvening * 60) return 2;
        return 3;
    }

    // Automatik-Helligkeit fuer die aktuelle Uhrzeit.
    function autoBrightness()
    {
        let t = nowMinutes();
        let s = state.sched;

        if(t >= s.tNight * 60 || t < s.tMorning * 60) return 0;

        if(t < s.tDay * 60)
        {
            let p = (t - s.tMorning * 60) / (s.tDay * 60 - s.tMorning * 60);
            return Math.round(clamp01(p) * s.bMorning);
        }

        if(t < s.tEvening * 60) return s.bDay;

        let q = (t - s.tEvening * 60) / (s.tNight * 60 - s.tEvening * 60);
        return Math.round(s.bEveStart + (s.bEveEnd - s.bEveStart) * clamp01(q));
    }

    // Begrenzt einen Wert auf 0..1.
    function clamp01(p)
    {
        if(p < 0) return 0;
        if(p > 1) return 1;
        return p;
    }

    // Zweistellige Zahl (z. B. 7 -> "07").
    function two(n)
    {
        return String(n).padStart(2, '0');
    }

    // Baut die Statusmeldung als JSON (wie der echte Controller).
    function buildStatus()
    {
        let d = new Date();
        state.autoBrightness = autoBrightness();
        state.time = two(d.getHours()) + ':' + two(d.getMinutes()) + ':' + two(d.getSeconds());
        state.date = two(d.getDate()) + '.' + two(d.getMonth() + 1) + '.' + d.getFullYear();
        return JSON.stringify(state);
    }

    // Ein manueller Eingriff waehrend der Automatik schaltet auf Statisch.
    function enterOverride()
    {
        if(state.mode === 3)
        {
            state.mode = 1;
            overrideActive = true;
            overrideWindow = currentWindow();
        }
    }

    // Verarbeitet einen Befehl der App (wie onWebSocketEvent in der Firmware).
    function handle(msg)
    {
        let doc;
        try { doc = JSON.parse(msg); } catch(e) { return; }

        if(typeof doc.mode === 'number' && doc.mode >= 0 && doc.mode <= 16)
        {
            state.mode = doc.mode;
            if(doc.mode === 3)
            {
                overrideActive = false;
            }
            else
            {
                overrideActive = true;
                overrideWindow = currentWindow();
            }
        }

        if(doc.left !== undefined)  { state.left  = clamp(doc.left, 0, 100);  enterOverride(); }
        if(doc.right !== undefined) { state.right = clamp(doc.right, 0, 100); enterOverride(); }
        if(doc.logo !== undefined)  { state.logo  = clamp(doc.logo, 0, 100);  enterOverride(); }
        // Effekt-Helligkeit (global) ist fest im Code (config.h) - nicht annehmen.

        // Automatik-Zeitprofil ist fest im Code (config.h) - wird nicht angenommen.
    }

    // Ersatz fuer WebSocket: verbindet sofort und schickt jede Sekunde den Status.
    class FakeWebSocket
    {
        constructor()
        {
            let self = this;
            this.readyState = 0;
            this.timer = null;

            setTimeout(function()
            {
                self.readyState = 1;
                if(self.onopen) self.onopen();
                if(self.onmessage) self.onmessage({ data: buildStatus() });
            }, 50);

            this.timer = setInterval(function()
            {
                if(self.readyState !== 1 || !self.onmessage) return;

                // Automatik-Rueckkehr beim naechsten Zeitfensterwechsel.
                if(overrideActive && state.mode !== 3)
                {
                    let w = currentWindow();
                    if(w >= 0 && overrideWindow >= 0 && w !== overrideWindow)
                    {
                        state.mode = 3;
                        overrideActive = false;
                    }
                }
                self.onmessage({ data: buildStatus() });
            }, 1000);
        }

        send(msg)
        {
            handle(msg);
            if(this.onmessage) this.onmessage({ data: buildStatus() });
        }

        close()
        {
            this.readyState = 3;
            clearInterval(this.timer);
        }
    }

    FakeWebSocket.CONNECTING = 0;
    FakeWebSocket.OPEN = 1;
    FakeWebSocket.CLOSING = 2;
    FakeWebSocket.CLOSED = 3;

    window.WebSocket = FakeWebSocket;
})();
</script>
`;

html = html.replace('<body>', '<body>\n' + simulator);

// Generierte Datei kennzeichnen: web_page_demo.html ist die zweite Variante der App und
// wird IMMER aus src/web_page.h erzeugt. Nicht von Hand editieren, sonst laeuft
// sie aus dem Takt mit der echten Firmware (so entstand der urspruengliche
// Fehler). Nach Aenderungen an web_page.h einfach dieses Skript erneut starten.
const banner = '<!-- AUTOMATISCH ERZEUGT aus src/web_page.h durch tools/build-demo.js.'
             + ' Nicht von Hand editieren - Aenderungen in web_page.h vornehmen und'
             + ' "node tools/build-demo.js" erneut ausfuehren. -->\n';
html = banner + html.replace(/^\s+/, '');

fs.writeFileSync(OUT, html);
console.log('web_page_demo.html erzeugt:', OUT);
console.log('Einfach im Browser oeffnen - laeuft ohne ESP32 und ohne Server.');
