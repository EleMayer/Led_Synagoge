# Inbetriebnahme – erster Hardware-Test

Kurzanleitung für den ersten Anschluss von LEDs (und optional der RTC). Ziel:
gefahrlos prüfen, dass die Segmente und das Logo leuchten und die Modi wirken.

> **Grundregel:** Vor jedem Verkabeln die Anlage **stromlos** machen. Der ESP32
> arbeitet mit 3,3 V, die WS2812 mit 5 V – deshalb Pegelwandler und **eine
> gemeinsame Masse** (siehe unten). Arbeiten an der 230-V-Seite nur durch eine
> Elektrofachkraft.

---

## 1. Für den ersten Test genügt wenig

- ESP32
- ein **kurzes Stück** LED-Streifen (z. B. 8–16 LEDs reichen zum Prüfen)
- USB-Kabel (versorgt ESP + wenige LEDs)
- optional: Pegelwandler 3,3 → 5 V, Vorwiderstand 330–470 Ω
- optional: DS3231-Modul (für die Uhr)

**Wichtig:** Für den ersten Test in `include/config.h` die LED-Zahl klein setzen,
z. B. `NUM_LEDS_LINKS 8`, damit der USB-Strom reicht. Für den echten Aufbau
später wieder **60/60** und ein externes 5-V-Netzteil.

---

## 2. Pinbelegung (aus `config.h`)

| Signal | ESP32-Pin | verbinden mit |
| --- | --- | --- |
| Segment Links – Daten | **GPIO 23** | (Pegelwandler →) **DIN** des linken Streifens |
| Segment Rechts – Daten | **GPIO 13** | (Pegelwandler →) **DIN** des rechten Streifens |
| Logo – PWM | **GPIO 14** | Gate des MOSFET (Logo-LED) |
| RTC SDA | **GPIO 21** | DS3231 SDA |
| RTC SCL | **GPIO 22** | DS3231 SCL |
| GND | **GND** | **gemeinsame Masse** von ESP, LEDs und Netzteil |
| 5V | **5V/VIN** | 5-V-Versorgung |

---

## 3. Reihenfolge beim Anschließen

1. **Strom aus.**
2. **Masse zuerst**: GND von ESP, LED-Streifen und (späterem) Netzteil zusammen.
3. **Datenleitung**: GPIO 23 → (Pegelwandler →) **DIN** des Streifens. Auf die
   **Pfeilrichtung** am Streifen achten (Daten gehen in `DIN`, nicht `DOUT`).
   Ein **Vorwiderstand 330–470 Ω** nah am ersten LED beruhigt das Signal.
4. **5 V** an den Streifen (beim kleinen Test aus dem ESP-5V-Pin, sonst Netzteil).
5. Sichtprüfung, dann **Strom ein**.

---

## 4. Erster Funktionstest

**a) Verkabelung + Farbreihenfolge prüfen (empfohlen):**
In `include/config.h` einmalig `ENABLE_SELFTEST` auf **1** setzen, flashen. Beim
Start läuft dann **Rot → Grün → Blau → Weiß** über die Segmente.
- Alle vier Farben korrekt → Verkabelung **und** `LED_COLOR_ORDER` passen.
- Farben vertauscht (z. B. Grün statt Rot) → `LED_COLOR_ORDER` in `config.h`
  anpassen (`GRB` ↔ `RGB`).
Danach `ENABLE_SELFTEST` wieder auf **0** setzen.

**b) Betrieb prüfen:**
- App öffnen (IP aus dem seriellen Monitor, z. B. `http://192.168.100.97`).
- Modus **Statisch** wählen, Regler hochziehen → Segmente + Logo werden hell/weiß.
- Ein paar **Modi** durchschalten – die Muster ändern sich.

---

## 5. Häufige Fehler und Abhilfe

| Symptom | Ursache | Abhilfe |
| --- | --- | --- |
| Nichts leuchtet | keine gemeinsame Masse, `DIN`/`DOUT` vertauscht, falscher GPIO | Masse verbinden; Datenleitung auf `DIN`; Pin prüfen |
| Nur die erste LED leuchtet | Datenpegel zu niedrig / erste LED defekt | Pegelwandler nutzen; erste LED tauschen |
| Farben falsch (im Selbsttest) | `LED_COLOR_ORDER` passt nicht | in `config.h` auf `RGB`/`GRB` ändern |
| Flackern | Spannungseinbruch, Masse, Pegel | gemeinsame Masse; kürzere Leitung; Pegelwandler |
| ESP startet ständig neu (Brownout) | zu viele LEDs am USB-Strom | LED-Zahl klein halten oder externes 5-V-Netzteil |
| Tags nicht voll hell | `LED_MAX_MILLIAMPS` zu niedrig | in `config.h` an das reale Netzteil erhöhen |

---

## 6. Für den echten Aufbau danach

- `NUM_LEDS_LINKS` / `NUM_LEDS_RECHTS` wieder auf **60**.
- Externes **5-V-Netzteil** (Dimensionierung nach realer LED-Zahl), **gemeinsame
  Masse** mit dem ESP, bei 120 LEDs **beidseitige Einspeisung**.
- `LED_MAX_MILLIAMPS` an das Netzteil anpassen.
- Weiter mit den Prüfungen in [test/betriebstests.md](test/betriebstests.md).
