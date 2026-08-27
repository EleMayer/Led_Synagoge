# Notfallplan – LED-Fassadenbeleuchtung

Was im Stör- oder Gefahrenfall zu tun ist. Reihenfolge immer:
**1. Personen schützen → 2. Sachschäden begrenzen → 3. Betrieb wiederherstellen.**

Bei Fragen an die/den **technisch Verantwortliche(n)** wenden (siehe Kontakte, Abschnitt 5).

> **Grundregel:** Im Zweifel zuerst die Anlage **spannungsfrei schalten**
> (Not-Aus, Abschnitt 1) und eine Fachkraft verständigen. Nichts bei Nässe oder
> unter Strom berühren. Arbeiten an der 230-V-Seite nur durch eine
> elektrotechnisch befähigte Person.

---

## 1. Not-Aus – Anlage sofort spannungsfrei schalten

Bei Rauch, Feuer, Wasser, Funken, Geruch oder beschädigten Teilen **zuerst**:

1. Stromversorgung trennen: **Sicherung / Trennschalter aus** bzw. Netzstecker
   des Netzteils ziehen.
2. Bereich absichern, Personen fernhalten.
3. Erst danach nach der Ursache sehen (Abschnitt 2).

Standort der Sicherung/des Trennschalters: ____________________________

---

## 2. Notfälle und Sofortmaßnahmen

| Notfall | Sofortmaßnahme | Danach |
| --- | --- | --- |
| **Rauch, Brandgeruch, Feuer** | Not-Aus, Personen in Sicherheit, **Feuerwehr 122**. Elektrikbrand **nicht mit Wasser** löschen | Verantwortliche(n) informieren, Anlage bis zur Prüfung aus lassen |
| **Wassereintritt / nasse Anlage** | **Nicht berühren**, Not-Aus (Sicherung, nicht das nasse Gerät) | Trocknen lassen, Elektrofachkraft prüfen lassen |
| **Netzteil heiß, Geruch, Funken** | Not-Aus | Netzteil durch Fachkraft tauschen |
| **Kabel/LED-Teile lose oder herabhängend** | Bereich absperren (Stromschlag-/Absturzgefahr), Not-Aus, nicht berühren | Fachkraft reparieren lassen |
| **Nach Sturm/Unwetter** | Vor dem Einschalten auf Nässe und mechanische Schäden prüfen | Erst bei einwandfreiem Zustand wieder einschalten |
| **Dauerhaft grell / blendet, nicht steuerbar** | Modus *Aus* in der App; wenn das nicht geht: Not-Aus | Neustart, danach Abschnitt 3 |
| **Alles dunkel** (keine Gefahr) | – | Sicherung/Netzteil prüfen, Abschnitt 3 |

**Österreichische Notrufe:** Feuerwehr **122** · Rettung **144** · Polizei **133**
· Euro-Notruf **112**

---

## 3. Störungen ohne Gefahr – schnelle Behebung

| Symptom | Wahrscheinliche Ursache | Maßnahme |
| --- | --- | --- |
| Ein ganzes Segment dunkel | Erste LED defekt, Datenleitung, Pegelwandler oder Einspeisung | Datenleitung/Pegelwandler prüfen; erste LED des Segments tauschen |
| Ab einer bestimmten Stelle dunkel | Defekte LED an dieser Stelle (WS2812 sind in Reihe) | Betroffene LED tauschen – ab ihr wird das Signal nicht weitergegeben |
| Alles dunkel | Netzteil, Sicherung oder ESP32 ohne Strom | Netzteil/Sicherung prüfen; Status-LED am ESP32 kontrollieren |
| Logo dunkel, Segmente ok | MOSFET, Logo-LED oder PWM-Leitung (GPIO14) | MOSFET/LED und Gate-Beschaltung prüfen |
| Flackern / Helligkeitssprünge | Spannungseinbruch, Datenleitung, Pegelwandler oder fehlende gemeinsame Masse | Einspeisung/Power-Injection prüfen; **alle Massen verbinden**. Bei gleichzeitiger Erwärmung: Not-Aus |
| Uhrzeit falsch, Automatik verschoben | RTC-Batterie leer und kein NTP | CR2032 tauschen; WLAN/Internet bereitstellen |
| App nicht erreichbar | WLAN, mDNS oder IP | `http://led-fassade.local` bzw. IP versuchen; Setup-AP „Fassade-Setup" nutzen |
| Tags zu dunkel trotz Vollhelligkeit | Strombegrenzung greift (`LED_MAX_MILLIAMPS` zu niedrig fürs Netzteil) | Wert in `config.h` an das reale Netzteil anpassen |

---

## 4. Wieder in Betrieb nehmen (nach Not-Aus)

1. Ursache beseitigt? Anlage trocken und unbeschädigt?
2. Sicherung/Trennschalter wieder ein bzw. Netzteil anstecken.
3. Der Controller startet selbsttätig im **Automatik-Modus**; nach kurzer Zeit
   sollte die Fassade wieder passend leuchten.
4. Funktion in der App prüfen (Modus, Uhrzeit). Bleibt die Störung, Abschnitt 3
   bzw. die/den Verantwortliche(n) kontaktieren.

---

## 5. Kontakte

| Rolle | Name / Stelle | Erreichbar |
| --- | --- | --- |
| Technisch verantwortlich | ____________________ | ____________________ |
| Elektrofachkraft / Firma (230 V) | ____________________ | ____________________ |
| Haustechnik / Notfall vor Ort | ____________________ | ____________________ |

---

## 6. Ereignis dokumentieren

Jeden Notfall hier festhalten.

| Datum/Uhrzeit | Ereignis | Sofortmaßnahme | Ursache / Behebung | Gemeldet an |
| --- | --- | --- | --- | --- |
|  |  |  |  |  |
|  |  |  |  |  |
|  |  |  |  |  |

---

## 7. Ersatzteile (für die Behebung)

Empfohlener Grundvorrat:

- LED-Segment WS2812 (Ersatzstück bzw. einzelne LEDs zum Nachlöten)
- Logo-LED (weiß)
- Feinsicherung passend zum Netzteil
- Knopfzelle CR2032
- MOSFET IRLZ44N
