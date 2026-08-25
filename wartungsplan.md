# Wartungsplan – LED-Fassadenbeleuchtung

Bei Fragen kann sich jederzeit an Elena Mayer (elena@mayer.or.at) gewendet werden.


## 1. Wartungsintervalle im Überblick

| Intervall | Tätigkeit (Kurzfassung) |
| --- | --- |
| **Monatlich** | Sichtkontrolle aus Distanz: leuchten beide Segmente und das Logo gleichmäßig? Auffällige Ausfälle oder Flackern? |
| **Halbjährlich** | Nahsichtprüfung, Reinigung, Klemmen/Steckverbinder, Gehäuse-Dichtung, Funktionstest aller Modi, Uhrzeit prüfen |
| **Jährlich** | Netzteil und Verkabelung gründlich, Sicherung, Erwärmung, Firmware aktuell |
| **Alle 2–3 Jahre** | RTC-Knopfzelle (CR2032) im DS3231 tauschen |
| **Nach Sturm/Unwetter** | Außerplanmäßige Kontrolle auf Wassereintritt und mechanische Schäden |

---

## 2. Wartungstätigkeiten im Detail

### 2.1 Monatlich – Sichtkontrolle (ohne Werkzeug)
- Beide LED-Segmente (Links, Rechts) leuchten **gleichmäßig**, keine dunklen
  oder verfärbten Stellen.
- Logo-LED leuchtet und lässt sich (Modus *Statisch*) dimmen.
- Kein **Flackern** und keine Helligkeitssprünge.
- Bei Auffälligkeiten Eintrag im Wartungsnachweis (Abschnitt 5) und Prüfung nach
  der Störungstabelle (Abschnitt 4).

### 2.2 Halbjährlich – Nahprüfung und Funktionstest
- **Reinigung:** Abdeckung/Diffusor und Sensorfläche vorsichtig von Staub und
  Schmutz befreien (trockenes/leicht feuchtes Tuch, keine scharfen Reiniger).
- **Klemmen/Steckverbinder:** fest, keine Korrosion, keine losen Litzen.
- **Gehäuse:** Dichtungen intakt, kein Wassereintritt, Verschraubungen fest.
- **Funktionstest:** über die App jeden Modus kurz durchschalten; Automatik
  anwählen und Phasen-/Helligkeitsanzeige prüfen.
- **Uhrzeit:** in der App Datum/Uhrzeit kontrollieren (Sommer-/Winterzeit wird
  bei Internet automatisch per NTP nachgeführt).

### 2.3 Jährlich – Elektrik und Software
- **Netzteil:** Lüftungsschlitze frei, kein übermäßiger Staub, Gehäuse nur
  handwarm. Auf Brummen/Geruch achten.
- **Verkabelung:** Isolierung unbeschädigt, Zugentlastung vorhanden,
  Einspeisung der Segmente fest (bei 120 LEDs beidseitig).
- **Sicherung:** Wert passend zum Netzteil, unbeschädigt.
- **MOSFET/Logo-Stufe:** keine Überhitzung.
- **Firmware:** aktuellen Stand prüfen; bei Bedarf Update (Abschnitt 3).

### 2.4 Alle 2–3 Jahre – RTC-Batterie
- Knopfzelle **CR2032** im DS3231-Modul tauschen. Danach in der App Datum/Uhrzeit
  kontrollieren; bei Internet stellt sich die Zeit über NTP selbst.

---

## 3. Firmware-Update (OTA)

Updates laufen über das lokale Netz und sind **passwortgeschützt**
(`OTA_USER`/`OTA_PASSWORD` aus `config.h`):

```bash
curl --user admin:PASSWORT -F "update=@firmware.bin" http://led-fassade.local/update
```

Nach dem Update startet der Controller neu und läuft automatisch im
Automatik-Modus wieder an. Vorher die neue `firmware.bin` bereithalten und die
Anlage zugänglich halten (Neustart dauert wenige Sekunden).

---

## 4. Störungstabelle (Fehlersuche)

| Symptom | Wahrscheinliche Ursache | Maßnahme |
| --- | --- | --- |
| Ein ganzes Segment dunkel | Erste LED defekt, Datenleitung, Pegelwandler oder Einspeisung | Datenleitung/Pegelwandler prüfen; erste LED des Segments tauschen |
| Ab einer bestimmten Stelle dunkel | Defekte LED an dieser Stelle (WS2812 sind in Reihe) | Betroffene LED tauschen – ab ihr wird das Signal nicht weitergegeben |
| Alles dunkel | Netzteil, Sicherung oder ESP32 ohne Strom | Netzteil/Sicherung prüfen; Status-LED am ESP32 kontrollieren |
| Logo dunkel, Segmente ok | MOSFET, Logo-LED oder PWM-Leitung (GPIO14) | MOSFET/LED und Gate-Beschaltung prüfen |
| Flackern / Helligkeitssprünge | Spannungseinbruch, Datenleitung, Pegelwandler oder fehlende gemeinsame Masse | Einspeisung/Power-Injection prüfen; **alle Massen verbinden** |
| Uhrzeit falsch, Automatik verschoben | RTC-Batterie leer und kein NTP | CR2032 tauschen; WLAN/Internet bereitstellen |
| App nicht erreichbar | WLAN, mDNS oder IP | `http://led-fassade.local` bzw. IP versuchen; Setup-AP „Fassade-Setup" nutzen |
| Tags zu dunkel trotz Vollhelligkeit | Strombegrenzung greift (`LED_MAX_MILLIAMPS` zu niedrig fürs Netzteil) | Wert an das reale Netzteil anpassen (siehe `schaltung.md`) |

---

## 5. Wartungsnachweis

Jede Wartung hier eintragen und abzeichnen.

| Datum | Intervall | Durchgeführte Tätigkeit | Befund / Maßnahme | Prüfer | Unterschrift |
| --- | --- | --- | --- | --- | --- |
|  |  |  |  |  |  |
|  |  |  |  |  |  |
|  |  |  |  |  |  |
|  |  |  |  |  |  |
|  |  |  |  |  |  |

---

## 6. Ersatzteile

Empfohlener Grundvorrat (Details und Richtpreise in [schaltung.md](schaltung.md)):

- LED-Segment WS2812 (Ersatzstück bzw. einzelne LEDs zum Nachlöten)
- Logo-LED (weiß)
- Feinsicherung passend zum Netzteil
- Knopfzelle CR2032
- MOSFET IRLZ44N

Verantwortlich: ____________________   ·   Notfallkontakt: ____________________
