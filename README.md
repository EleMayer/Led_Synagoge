# LED-Fassadenbeleuchtung (Synagoge / Museum Arbeitswelt)

ESP32-Steuerung für eine mehrteilige LED-Fassadenbeleuchtung mit Weiß-LEDs
(WS2812) und dimmbarem Logo (PWM/MOSFET). Bedienung über eine lokale,
installierbare Web-App (PWA) per WLAN – kein Cloud-Dienst.

**Firmware-Version:** 2.4.0 · **Plattform:** ESP32 / Arduino (PlatformIO,
espressif32@7.0.1 → Arduino-Core 2.0.17, alte LEDC-API)

## Funktionen

- Grundmodi: Aus, Statisch, Automatik (tageszeitabhängig, mit RTC + NTP)
- Effekte: Lauflicht, Pulsieren, Atmen, Welle
- Stimmungs-Modi: Dauerlicht, Kerzenlicht, Stufenlicht, Dämmerlicht, Feuerschein, Nachtlicht
  (feste Werte, Nachtabschaltung 23:00–06:00)
- Eigene Szenen (Presets), im NVS gespeichert
- Automatik-Zeitprofil einstellbar, mit 24-Stunden-Kurven-Vorschau in der App
- WLAN per App konfigurierbar, Setup-Accesspoint als Fallback
- Erreichbar über `http://led-fassade.local` (mDNS)
- Firmware-Update über den Browser (OTA)

## Projektstruktur

```text
include/config.h    Konfiguration (Pins, LED-Anzahl, Grenzwerte, Modi)
src/main.cpp        Steuerlogik (Modi, Automatik, Netzwerk, OTA)
src/web_page.h      Bedien-App (HTML/CSS/JS, PWA-Manifest, Icon)
tools/mock-server.js  PC-Testserver (simuliert die ESP32-API)
dokumentation.md    Technische Dokumentation
```

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
