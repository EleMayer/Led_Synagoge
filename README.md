# LED-Fassadenbeleuchtung (Synagoge / Museum Arbeitswelt)

ESP32-Steuerung für eine mehrteilige LED-Fassadenbeleuchtung mit Weiß-LEDs
(WS2812) und dimmbarem Logo (PWM/MOSFET). Bedienung über eine lokale,
installierbare Web-App (PWA) per WLAN – kein Cloud-Dienst.

**Firmware-Version:** 2.4.3 · **Plattform:** ESP32 / Arduino (PlatformIO,
espressif32@7.0.1 → Arduino-Core 2.0.17, alte LEDC-API)

## Funktionen

- Grundmodi: Aus, Statisch, Automatik (tageszeitabhängig, mit RTC + NTP)
- Effekte: Lauflicht, Pulsieren, Atmen, Welle, Sternenfunkeln, Treffpunkt,
  Herzschlag, Wechsellicht
- Stimmungs-Modi: Dauerlicht, Kerzenlicht, Stufenlicht, Dämmerlicht, Feuerschein, Nachtlicht
  (feste Werte, Nachtabschaltung 23:00–06:00)
- Alle Modi außer Automatik sind auf **ruhige Fassadenwirkung** abgestimmt (wenig
  Kontrast, langsame Bewegung, angehobene Grundhelligkeit)

Ausführliche Erklärung aller Modi: siehe [modi.md](modi.md).
- Eigene Szenen (Presets), im NVS gespeichert
- Automatik-Profil (Uhrzeiten **und** Helligkeiten) fest in `config.h`, nur dort
  änderbar; die App zeigt es schreibgeschützt als Phasen-Übersicht
- App-Oberfläche mit **Hell-/Dunkel-Umschalter** und **Deutsch/Englisch**
  (Auswahl wird im Browser gespeichert)
- WLAN-Zugangsdaten fest im Code (`config.h`), Setup-Accesspoint als Fallback für lokalen Zugriff
- Erreichbar über `http://led-fassade.local` (mDNS)
- Firmware-Update über den Browser (OTA)

## Projektstruktur

```text
include/config.h    Konfiguration (Pins, LED-Anzahl, Grenzwerte, Modi)
include/icons.h       PNG-Home-Screen-Icons als Byte-Arrays (generiert)
src/main.cpp        Steuerlogik (Modi, Automatik, Netzwerk, OTA)
src/web_page.h        Bedien-App (HTML/CSS/JS, PWA-Manifest, Icon) - Quelle der Wahrheit
src/web_page_demo.html  zweite Variante der App: laeuft ohne ESP32/Server (generiert)
tools/mock-server.js  PC-Testserver (simuliert die ESP32-API)
tools/build-demo.js   erzeugt src/web_page_demo.html aus src/web_page.h
tools/build-icons.js  erzeugt include/icons.h aus tools/icons/*.png
tools/icons/          Home-Screen-Icons (icon-180/192/512.png)
modi.md               Erklärung aller Betriebsmodi
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

## WLAN einstellen

Die WLAN-Zugangsdaten stehen **fest im Code** in [`include/config.h`](include/config.h)
und werden nicht über die App geändert (rein lokale Bedienung, feste
Zugangsdaten – Pflichtenheft Kap. 2/9):

```c
#define WIFI_SSID     "Museum-Arbeitswelt"   // WLAN-Name
#define WIFI_PASS     "willkommen"           // WLAN-Passwort
#define SETUP_AP_SSID "Fassade-Setup"        // Notfall-Accesspoint (nur Zugriff)
#define SETUP_AP_PASS "fassade2026"
```

SSID/Passwort dort ändern und die Firmware neu aufspielen:

```bash
pio run -t upload
```

Findet der Controller das WLAN nicht, öffnet er selbst den Accesspoint
`Fassade-Setup`, über den die Oberfläche lokal erreichbar bleibt. Dieser AP
dient nur dem Zugriff – ein Eingeben neuer Zugangsdaten (Setup-Portal mit
Speicherung im NVS) ist bewusst nicht umgesetzt.

## Anschlüsse (Pinbelegung)

Definiert in `include/config.h`. Der ESP32 gibt nur kleine Steuersignale nach
außen; die LED-Hochstromversorgung wird separat direkt am Streifen eingespeist.

| Signal | GPIO | Beschreibung |
|---|---|---|
| Segment Links (Daten) | 23 | Datenleitung zum linken Streifen (über Pegelwandler 3,3 V → 5 V) |
| Segment Rechts (Daten) | 13 | Datenleitung zum rechten Streifen (über Pegelwandler) |
| Logo (PWM) | 14 | dimmbares Logo über externen MOSFET-Treiber |
| RTC SDA | 21 | I²C-Datenleitung zum DS3231 |
| RTC SCL | 22 | I²C-Taktleitung zum DS3231 |

Streifen: je 60 LEDs, Typ WS2812 (GRB). Die endgültige LED-Wahl (laut
Pflichtenheft WS2811, 12 V) sowie Netzteil und Strombegrenzung
(`LED_MAX_MILLIAMPS`) sind vor dem Einsatz an die reale Hardware anzupassen.

## Verhalten bei Stromausfall (Failsafe)

Das System läuft nach einem Stromausfall ohne Eingriff vor Ort wieder an:

1. Der Controller startet **immer im Automatik-Modus**, unabhängig vom zuletzt
   aktiven Modus. Ein nächtlicher Ausfall führt so nicht zu ungewolltem
   Hellschalten.
2. Die Uhrzeit kommt sofort aus dem batteriegepufferten RTC-Modul (DS3231) –
   ohne WLAN/Internet. Der passende Helligkeitszustand wird direkt berechnet.
3. Ist keine gültige Zeit verfügbar (leere RTC-Batterie, noch kein NTP), fährt
   die Automatik in einen sicheren, gedimmten Grundzustand
   (`SAFE_DEFAULT_BRIGHTNESS`) und korrigiert sich, sobald NTP synchronisiert.
4. Bei Internetverbindung wird die RTC periodisch per NTP nachgeführt
   (Sommer-/Winterzeit berücksichtigt).
5. Die Benutzerkonfiguration (manuelle Helligkeiten, Szenen) liegt im NVS und
   übersteht den Stromausfall. Das Automatik-Profil (Uhrzeiten und
   Helligkeiten) steht dagegen fest in `config.h`.

## Bedienung (Kurzüberblick)

- **Modus** wählen, **Helligkeit** je Bereich (Links, Rechts, Logo) im Modus
  *Statisch* regeln und eigene **Szenen** speichern. Das **Automatik-Profil**
  (Uhrzeiten und Helligkeiten) ist fest in `config.h` hinterlegt und in der App
  nur als schreibgeschützte Übersicht sichtbar.
- Ein manueller Eingriff übersteuert die Automatik. Das System kehrt beim
  nächsten Zeitfenster-Übergang (z. B. Tag → Abend) selbsttätig in die Automatik
  zurück.

## Installation als App (PWA)

Die Bedienoberfläche ist eine PWA und lässt sich auf Handy/Tablet als
eigenständige App auf dem Startbildschirm ablegen (öffnet dann randlos, ohne
Browserleiste):

- **Android/Desktop (Chrome):** In der App erscheint oben die Karte *„Als App
  installieren"* mit einem Button; alternativ das Browser-Menü → *Installieren*.
- **iOS (Safari):** *Teilen* → *Zum Home-Bildschirm*.

Das App-Icon wird als PNG ausgeliefert (`/apple-touch-icon.png` für iOS,
`/icon-192.png` und `/icon-512.png` für Android). Diese PNGs liegen in
`tools/icons/` und werden per `node tools/build-icons.js` in `include/icons.h`
eingebettet, das die Firmware ausliefert. Ein echter Offline-Cache (Service
Worker) ist nicht möglich, da Browser diesen nur über HTTPS/localhost erlauben,
der ESP32 aber über HTTP im WLAN ausliefert — Installieren und App-Start
funktionieren davon unabhängig.

## Design & Codestil

- **Oberfläche:** klinisch-reduziertes Design ohne erklärende Zusatztexte –
  bewusst schlicht und übersichtlich. Über den Kopfzeilen-Knopf zwischen
  **dunkel** (Standard, schwarz) und **hell** umschaltbar; ein zweiter Knopf
  schaltet die Sprache zwischen **Deutsch und Englisch**. Beide Einstellungen
  werden im Browser gespeichert.
- **Codestil:** bewusst einfach und anfängerfreundlich gehalten – benannte
  Funktionen statt anonymer Funktionen (Lambdas), klare `for`-Schleifen statt
  Array-Kniffe (`map`/`forEach`), kurze Funktionen und durchgehende Kommentare.

## Bauen & Flashen

```bash
pio run                 # kompilieren
pio run -t upload       # auf den ESP32 flashen (Board per USB angeschlossen)
pio device monitor      # serielle Ausgabe (115200 Baud)
```

Der Build ist geprüft und läuft fehlerfrei durch. Aktuelle Größe (esp32dev):
**Flash ≈ 74,5 %** (inkl. eingebetteter App und PNG-Icons), **RAM ≈ 15 %**.

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
