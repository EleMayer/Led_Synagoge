// ============================================================================
//  Mock-Server fuer die LED-Fassade (nur zum Testen am PC, kein Teil der Firmware)
//
//  Er liefert die Bedienoberflaeche direkt aus ../src/web_page.h aus und bildet
//  die komplette ESP32-API nach: WebSocket (/ws) mit denselben JSON-Befehlen wie
//  die Firmware sowie die REST-Endpunkte /api/status und /api/schedule. Dadurch
//  laesst sich die Seite ohne echten ESP32 voll bedienen (Modi, Regler, Szenen,
//  Zeitprofil, WLAN).
//
//  Start:  node tools/mock-server.js     ->  http://localhost:5598
// ============================================================================
const http = require('http');
const fs = require('fs');
const path = require('path');
const crypto = require('crypto');

const PORT = 5598;
const WEB_PAGE_H = path.join(__dirname, '..', 'src', 'web_page.h');

// ---------------------------------------------------------------------------
//  HTML / Manifest / Icon aus web_page.h holen (bleibt so automatisch synchron)
// ---------------------------------------------------------------------------
function loadEmbeddedPages() {
    const src = fs.readFileSync(WEB_PAGE_H, 'utf-8');
    const pages = {};
    const re = /const char (\w+)\[\] PROGMEM = R"rawliteral\(([\s\S]*?)\)rawliteral";/g;
    let m;
    while ((m = re.exec(src)) !== null) {
        pages[m[1]] = m[2];
    }
    // Die Firmware baut die WebSocket-URL aus location.hostname (Port 80 am Geraet).
    // Fuer den Mock auf einem anderen Port brauchen wir location.host (inkl. Port).
    if (pages.index_html) {
        pages.index_html = pages.index_html.replace(/window\.location\.hostname/g,
                                                    'window.location.host');
    }
    return pages;
}
let pages = loadEmbeddedPages();

// ---------------------------------------------------------------------------
//  Zustand (wie in der Firmware)
// ---------------------------------------------------------------------------
const state = {
    mode: 3,                         // 3 = Automatik
    left: 80, right: 80, logo: 80,
    global: 80,
    autoBrightness: 0,
    rtc: true, ntp: true,
    ip: '127.0.0.1', rssi: -55,
    wifi: true, ap: false,
    ssid: 'Museum-Arbeitswelt',
    firmware: '2.3.0-mock',
    sched: { tMorning: 6, tDay: 8, tEvening: 18, tNight: 23,
             bMorning: 90, bDay: 90, bEveStart: 60, bEveEnd: 25 },
    presets: []                      // { slot, name, left, right, logo }
};

let overrideActive = false;
let overrideWindow = -1;

function clamp(v, lo, hi) {
    v = parseInt(v);
    if (isNaN(v)) return lo;
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// ---------------------------------------------------------------------------
//  Automatik: aktuelles Zeitfenster und Helligkeit (wie in der Firmware)
// ---------------------------------------------------------------------------
function currentWindow() {
    const now = new Date();
    const m = now.getHours() * 60 + now.getMinutes();
    const s = state.sched;
    if (m >= s.tNight * 60 || m < s.tMorning * 60) return 0;   // Nacht
    if (m >= s.tMorning * 60 && m < s.tDay * 60)   return 1;   // Morgen
    if (m >= s.tDay * 60 && m < s.tEvening * 60)   return 2;   // Tag
    if (m >= s.tEvening * 60 && m < s.tNight * 60) return 3;   // Abend
    return 0;
}

function calcAutomaticBrightness() {
    const now = new Date();
    const m = now.getHours() * 60 + now.getMinutes();
    const s = state.sched;
    if (m >= s.tNight * 60 || m < s.tMorning * 60) return 0;
    if (m >= s.tMorning * 60 && m < s.tDay * 60) {
        let p = (m - s.tMorning * 60) / (s.tDay * 60 - s.tMorning * 60);
        p = Math.max(0, Math.min(1, p));
        return Math.round(p * s.bMorning);
    }
    if (m >= s.tDay * 60 && m < s.tEvening * 60) return s.bDay;
    if (m >= s.tEvening * 60 && m < s.tNight * 60) {
        let p = (m - s.tEvening * 60) / (s.tNight * 60 - s.tEvening * 60);
        p = Math.max(0, Math.min(1, p));
        return Math.round(s.bEveStart + (s.bEveEnd - s.bEveStart) * p);
    }
    return 0;
}

// Manueller Eingriff in der Automatik -> auf Statisch + Override merken.
function enterOverrideIfAutomatic() {
    if (state.mode === 3) {
        state.mode = 1;
        overrideActive = true;
        overrideWindow = currentWindow();
    }
}

// ---------------------------------------------------------------------------
//  Status als JSON (mit aktueller Uhrzeit)
// ---------------------------------------------------------------------------
function two(n) { return String(n).padStart(2, '0'); }
function buildStatus() {
    const d = new Date();
    state.autoBrightness = calcAutomaticBrightness();
    const s = Object.assign({}, state);
    s.time = two(d.getHours()) + ':' + two(d.getMinutes()) + ':' + two(d.getSeconds());
    s.date = two(d.getDate()) + '.' + two(d.getMonth() + 1) + '.' + d.getFullYear();
    return JSON.stringify(s);
}

// ---------------------------------------------------------------------------
//  Eingehende Befehle verarbeiten (dieselben Felder wie die Firmware)
// ---------------------------------------------------------------------------
function handleCommand(msg, socket) {
    let doc;
    try { doc = JSON.parse(msg); } catch { return; }

    // WLAN setzen -> speichern + "Neustart"
    if (typeof doc.wifiSsid === 'string') {
        if (doc.wifiSsid.length > 0) {
            state.ssid = doc.wifiSsid;
            send(socket, '{"reboot":true}');
        }
        return;
    }

    if (typeof doc.mode === 'number' && doc.mode >= 0 && doc.mode <= 9) {
        state.mode = doc.mode;
        if (state.mode === 3) {
            overrideActive = false;
        } else {
            overrideActive = true;
            overrideWindow = currentWindow();
        }
    }

    if (doc.left  !== undefined) { state.left  = clamp(doc.left, 0, 100);  enterOverrideIfAutomatic(); }
    if (doc.right !== undefined) { state.right = clamp(doc.right, 0, 100); enterOverrideIfAutomatic(); }
    if (doc.logo  !== undefined) { state.logo  = clamp(doc.logo, 0, 100);  enterOverrideIfAutomatic(); }
    if (doc.global !== undefined) state.global = clamp(doc.global, 0, 100);

    // Automatik-Zeitprofil (Zeiten 0..23, Helligkeiten 0..100)
    if (doc.tMorning  !== undefined) state.sched.tMorning  = clamp(doc.tMorning, 0, 23);
    if (doc.tDay      !== undefined) state.sched.tDay      = clamp(doc.tDay, 0, 23);
    if (doc.tEvening  !== undefined) state.sched.tEvening  = clamp(doc.tEvening, 0, 23);
    if (doc.tNight    !== undefined) state.sched.tNight    = clamp(doc.tNight, 0, 23);
    if (doc.bMorning  !== undefined) state.sched.bMorning  = clamp(doc.bMorning, 0, 100);
    if (doc.bDay      !== undefined) state.sched.bDay      = clamp(doc.bDay, 0, 100);
    if (doc.bEveStart !== undefined) state.sched.bEveStart = clamp(doc.bEveStart, 0, 100);
    if (doc.bEveEnd   !== undefined) state.sched.bEveEnd   = clamp(doc.bEveEnd, 0, 100);

    // Szene speichern
    if (typeof doc.savePreset === 'string' && doc.savePreset.length > 0) {
        let p = state.presets.find(x => x.name === doc.savePreset);
        if (!p && state.presets.length < 6) {
            p = { slot: state.presets.length, name: doc.savePreset };
            state.presets.push(p);
        }
        if (p) { p.left = state.left; p.right = state.right; p.logo = state.logo; }
    }

    // Szene anwenden
    if (typeof doc.applyPreset === 'number') {
        const p = state.presets.find(x => x.slot === doc.applyPreset);
        if (p) {
            state.left = p.left; state.right = p.right; state.logo = p.logo;
            state.mode = 1;
            overrideActive = true;
            overrideWindow = currentWindow();
        }
    }

    // Szene loeschen
    if (typeof doc.deletePreset === 'number') {
        state.presets = state.presets.filter(x => x.slot !== doc.deletePreset);
        state.presets.forEach((x, i) => x.slot = i);   // Slots neu durchnummerieren
    }

    broadcast();
}

// ---------------------------------------------------------------------------
//  Periodischer Status + selbsttaetige Rueckkehr in die Automatik
// ---------------------------------------------------------------------------
setInterval(() => {
    if (overrideActive && state.mode !== 3) {
        const w = currentWindow();
        if (w >= 0 && overrideWindow >= 0 && w !== overrideWindow) {
            state.mode = 3;
            overrideActive = false;
        }
    }
    broadcast();
}, 1000);

// ---------------------------------------------------------------------------
//  HTTP-Server (Seite + REST)
// ---------------------------------------------------------------------------
const server = http.createServer((req, res) => {
    const u = new URL(req.url, 'http://x');

    if (u.pathname === '/api/status') {
        res.setHeader('Content-Type', 'application/json');
        return res.end(buildStatus());
    }
    if (u.pathname === '/api/schedule') {
        res.setHeader('Content-Type', 'application/json');
        const s = state.sched;
        return res.end(JSON.stringify(Object.assign({}, s, { autoBrightness: state.autoBrightness })));
    }
    if (u.pathname === '/' || u.pathname === '/index.html') {
        res.setHeader('Content-Type', 'text/html');
        return res.end(pages.index_html || '<h1>web_page.h nicht gefunden</h1>');
    }
    if (u.pathname === '/manifest.json') {
        res.setHeader('Content-Type', 'application/manifest+json');
        return res.end(pages.manifest_json || '{}');
    }
    if (u.pathname === '/icon.svg') {
        res.setHeader('Content-Type', 'image/svg+xml');
        return res.end(pages.icon_svg || '');
    }
    res.statusCode = 404;
    res.end('404');
});

// ---------------------------------------------------------------------------
//  Minimaler WebSocket (Senden + Empfangen)
// ---------------------------------------------------------------------------
const clients = new Set();

function broadcast() {
    const msg = frame(buildStatus());
    for (const s of clients) {
        try { s.write(msg); } catch { /* Client weg */ }
    }
}

function send(socket, str) {
    try { socket.write(frame(str)); } catch { /* Client weg */ }
}

server.on('upgrade', (req, socket) => {
    const key = req.headers['sec-websocket-key'];
    const accept = crypto.createHash('sha1')
        .update(key + '258EAFA5-E914-47DA-95CA-C5AB0DC85B11')
        .digest('base64');
    socket.write('HTTP/1.1 101 Switching Protocols\r\n' +
                 'Upgrade: websocket\r\nConnection: Upgrade\r\n' +
                 'Sec-WebSocket-Accept: ' + accept + '\r\n\r\n');

    clients.add(socket);
    send(socket, buildStatus());               // Startstatus wie die Firmware

    let buffer = Buffer.alloc(0);
    socket.on('data', (chunk) => {
        buffer = Buffer.concat([buffer, chunk]);
        buffer = parseFrames(buffer, socket);
    });
    socket.on('close', () => clients.delete(socket));
    socket.on('error', () => clients.delete(socket));
});

// Ausgehenden Text-Frame bauen (Server maskiert nicht).
function frame(str) {
    const b = Buffer.from(str);
    const len = b.length;
    let h;
    if (len < 126) {
        h = Buffer.from([0x81, len]);
    } else {
        h = Buffer.alloc(4);
        h[0] = 0x81; h[1] = 126;
        h.writeUInt16BE(len, 2);
    }
    return Buffer.concat([h, b]);
}

// Eingehende (maskierte) Frames vom Client zerlegen. Gibt den Rest zurueck.
function parseFrames(buf, socket) {
    while (buf.length >= 2) {
        const opcode = buf[0] & 0x0f;
        const masked = (buf[1] & 0x80) !== 0;
        let len = buf[1] & 0x7f;
        let offset = 2;

        if (len === 126) {
            if (buf.length < 4) break;
            len = buf.readUInt16BE(2);
            offset = 4;
        } else if (len === 127) {
            // So grosse Frames schickt die App nicht - verwerfen.
            return Buffer.alloc(0);
        }

        const need = offset + (masked ? 4 : 0) + len;
        if (buf.length < need) break;              // Frame noch nicht vollstaendig

        let payload;
        if (masked) {
            const mask = buf.slice(offset, offset + 4);
            payload = Buffer.alloc(len);
            for (let i = 0; i < len; i++) {
                payload[i] = buf[offset + 4 + i] ^ mask[i % 4];
            }
        } else {
            payload = buf.slice(offset, offset + len);
        }
        buf = buf.slice(need);

        if (opcode === 0x8) {                      // Close
            try { socket.end(); } catch {}
            clients.delete(socket);
            return Buffer.alloc(0);
        } else if (opcode === 0x9) {               // Ping -> Pong
            socket.write(Buffer.concat([Buffer.from([0x8a, payload.length]), payload]));
        } else if (opcode === 0x1) {               // Text
            handleCommand(payload.toString('utf-8'), socket);
        }
    }
    return buf;
}

server.listen(PORT, () => {
    console.log('LED-Fassade Mock laeuft auf http://localhost:' + PORT);
    console.log('(liefert die Seite aus src/web_page.h und simuliert die ESP32-API)');
});
