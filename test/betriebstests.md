# Betriebs- und Wiederanlauftests

Anleitung zum Prüfen der LED-Fassade im laufenden Betrieb – für den Alltag,
nach einem Stromausfall und bei Störungen. Keine Programmierkenntnisse nötig.
Bei Problemen: Sofortmaßnahmen und Fehlersuche im [notfallplan.md](../notfallplan.md).

---

## Werkzeuge zum Beobachten

- **App** (im WLAN): `http://led-fassade.local` – zeigt Modus, Uhrzeit und in der
  Karte **System** den Status von RTC, NTP und WLAN. Lädt der Name nicht, die
  **IP** aus dem seriellen Monitor nehmen (z. B. `http://192.168.0.42`).
- **Serielle Ausgabe** (nur bei angeschlossenem PC, optional):
  `pio device monitor` mit 115200 Baud. Der Controller meldet dort z. B.
  „System bereit.", „DS3231 gefunden.", „WLAN verbunden. IP: …".

---

## 1. Alltags-Schnellcheck (rund 1 Minute)

| Prüfung | So geht's | Was normal ist |
| --- | --- | --- |
| Beide Segmente leuchten | von außen ansehen | Links und Rechts gleichmäßig, keine dunklen Stellen |
| Logo leuchtet | ansehen | Logo hell und ruhig |
| Automatik plausibel | App öffnen, Modus = Automatik | Helligkeit passt zur Tageszeit (tags hell, abends gedimmt, nach 23:00 aus) |
| Uhrzeit stimmt | App, Karte System | richtige Uhrzeit und Datum |
| App erreichbar | `http://led-fassade.local` öffnen | Seite lädt, „aktuelle Helligkeit" bewegt sich |
| Modus schaltbar | in der App einen Modus wählen, dann wieder Automatik | Fassade reagiert sofort |

---

## 2. Wiederanlauf nach Stromausfall

Nach jedem Stromausfall muss die Fassade **ohne Eingriff vor Ort** korrekt wieder
anlaufen. Sinnvoll zu **drei Tageszeiten** prüfen: tagsüber, abends und nach
23:00 Uhr.

**Ablauf:** Anlage vom Strom trennen (Sicherung aus) → rund 10 Sekunden warten →
wieder einschalten → beobachten.

| Punkt | Was normal ist |
| --- | --- |
| Start | Fassade läuft von selbst wieder an, ohne Eingriff |
| Startmodus | immer **Automatik** – egal welcher Modus vorher aktiv war |
| Uhrzeit | kommt sofort aus der RTC (DS3231), auch ohne WLAN; App zeigt die richtige Zeit |
| Zustand nach Uhrzeit | tagsüber hell, abends gedimmt, **nach 23:00 bleibt es aus** |
| Anlauf | sanftes Hochblenden, kein hartes Aufleuchten, kein Farbtest |
| App | nach kurzer Zeit wieder erreichbar (sobald WLAN verbunden) |

Zur Orientierung, was zu welcher Tageszeit zu sehen sein sollte:

- **tagsüber** → hell (~90 %)
- **abends** → gedimmt (60 → 25 %)
- **nach 23:00** → aus

---

## 3. Gezielte Störungstests

| Test | So auslösen | Was normal ist |
| --- | --- | --- |
| WLAN fällt aus | Access-Point kurz abschalten | Beleuchtung läuft normal weiter; Controller verbindet automatisch neu (≤ 30 s) |
| Kein WLAN beim Start | Gerät ohne erreichbares WLAN starten | Setup-AP **„Fassade-Setup"** erscheint, App darüber erreichbar; Licht läuft nach RTC |
| Uhr ohne Zeitquelle | RTC-Batterie leer **und** kein Internet | sicherer, gedimmter Grundzustand (nicht voll hell, nicht ganz aus); korrigiert sich, sobald NTP da ist |
| Nachtabschaltung | Uhrzeit im Betrieb Richtung 23:00 beobachten | ab 23:00 alles aus, ab 06:00 sanftes Hochfahren |
| Modus „Aus" | in der App Modus **Aus** wählen | Segmente + Logo dunkel; in der App werden Modus-Karte und Aus-Button rot; Gerät bleibt erreichbar |
| Override-Rückkehr | in Automatik einen Regler verstellen, dann Zeitfenster-Wechsel abwarten | nach dem nächsten Übergang (z. B. Tag → Abend) selbsttätig zurück in Automatik |
| OTA nur mit Passwort | Firmware-Upload ohne bzw. mit falschem Passwort | Upload wird abgewiesen (kein Update); nur mit richtigem `OTA_PASSWORD` erfolgreich |

---

Zeiten und Helligkeiten der Automatik stehen in
[include/config.h](../include/config.h).
