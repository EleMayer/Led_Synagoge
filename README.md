# LED-Fassadenbeleuchtung (Synagoge / Museum Arbeitswelt)

ESP32-Steuerung für eine mehrteilige LED-Fassadenbeleuchtung mit Weiß-LEDs
(WS2812) und dimmbarem Logo (PWM/MOSFET). Bedienung über eine lokale,
installierbare Web-App (PWA) per WLAN – kein Cloud-Dienst.

**Firmware-Version:** 2.4.1 · **Plattform:** ESP32 / Arduino (PlatformIO,
espressif32@7.0.1 → Arduino-Core 2.0.17, alte LEDC-API)

## Funktionen

- Grundmodi: Aus, Statisch, Automatik (tageszeitabhängig, mit RTC + NTP)
- Effekte: Lauflicht, Pulsieren, Atmen, Welle
- Stimmungs-Modi: Dauerlicht, Kerzenlicht, Stufenlicht, Dämmerlicht, Feuerschein, Nachtlicht
  (feste Werte, Nachtabschaltung 23:00–06:00)
- Eigene Szenen (Presets), im NVS gespeichert
- Automatik-Zeitprofil einstellbar, mit 24-Stunden-Kurven-Vorschau in der App
- WLAN-Zugangsdaten fest im Code (`config.h`), Setup-Accesspoint als Fallback für lokalen Zugriff
- Erreichbar über `http://led-fassade.local` (mDNS)
- Firmware-Update über den Browser (OTA)

## Projektstruktur

```text
include/config.h    Konfiguration (Pins, LED-Anzahl, Grenzwerte, Modi)
src/main.cpp        Steuerlogik (Modi, Automatik, Netzwerk, OTA)
src/web_page.h        Bedien-App (HTML/CSS/JS, PWA-Manifest, Icon) - Quelle der Wahrheit
src/web_page_demo.html  zweite Variante der App: laeuft ohne ESP32/Server (generiert)
tools/mock-server.js  PC-Testserver (simuliert die ESP32-API)
tools/build-demo.js   erzeugt src/web_page_demo.html aus src/web_page.h
dokumentation.md      Technische Dokumentation
```

## Zweite Variante: eigenstaendige Demo (ohne ESP32)

`src/web_page_demo.html` ist dieselbe Bedien-App wie auf dem ESP32, aber mit
einem eingebauten Simulator statt WebSocket-Verbindung – zum Vorfuehren und
Testen der Oberflaeche im Browser, ganz ohne Hardware oder Server. Die Datei
wird **automatisch** aus `src/web_page.h` erzeugt und nicht von Hand bearbeitet:

```bash
node tools/build-demo.js       # erzeugt/aktualisiert src/web_page_demo.html
```

Nach jeder Aenderung an `src/web_page.h` das Skript erneut ausfuehren, damit die
Demo mit der echten App synchron bleibt.

## Bauen & Flashen

```bash
pio run                 # kompilieren
pio run -t upload       # auf den ESP32 flashen
pio device monitor      # serielle Ausgabe (115200 Baud)
```

## Bedienoberfläche am PC testen (ohne ESP32)

Der Mock-Server liefert die Seite aus `src/web_page.h` aus und bildet die
komplette API (WebSocket + REST) nach:

```bash
node tools/mock-server.js      # → http://localhost:5598
```

## Wichtiger Hinweis

Der Code nutzt die **alte LEDC-PWM-API** (`ledcSetup`/`ledcAttachPin`) und
setzt daher **Arduino-ESP32-Core 2.x** voraus. Die Versionen in
`platformio.ini` sind bewusst gepinnt – nicht ungeprüft auf Core 3.x heben.
