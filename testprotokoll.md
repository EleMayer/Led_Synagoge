# Testprotokoll – LED-Fassadenbeleuchtung

Nachweis der Anforderungen aus dem Pflichtenheft (MAW – LED-Fassadenbeleuchtung)
gegen die Firmware **Version 2.4.3**.

Das Protokoll trennt bewusst zwei Prüfarten:

- **SW** – am Schreibtisch prüfbar über den mitgelieferten Simulator/Mock
  (`tools/mock-server.js` + `src/web_page_demo.html`). Diese Fälle sind bereits
  bestätigt.
- **HW** – nur am aufgebauten Gerät (ESP32 + LEDs + RTC + Netzteil) sinnvoll
  prüfbar. Diese Zeilen sind für die **Abnahme am Gerät** vorbereitet und dort
  abzuzeichnen (Ergebnis eintragen, Datum/Prüfer ergänzen).

Legende Ergebnis: **✔ bestanden** · **[ ] offen** (noch auszuführen) · **✘ nicht in Ordnung**

---

## 1. Prüfumgebung

| Punkt | Angabe |
| --- | --- |
| Firmware-Version | 2.4.3 |
| Plattform | ESP32 (esp32dev), Arduino-Core 2.x, `espressif32@7.0.1` |
| Build | `pio run` – Erfolg (Flash ~75,6 %, RAM ~15,0 %) |
| SW-Prüfstand | `node tools/mock-server.js` → `http://localhost:5598` |
| HW-Prüfstand | ESP32, 2× LED-Segment (je 60), Logo-LED (PWM/MOSFET), DS3231, 5-V-Netzteil |
| Datum / Prüfer | ____________ / ____________ |

---

## 2. Funktionale Anforderungen (Pflichtenheft Kap. 6)

| Nr. | Anforderung | Durchführung | Erwartetes Ergebnis | Art | Ergebnis |
| --- | --- | --- | --- | --- | --- |
| F1 | Links, Rechts, Logo getrennt/gemeinsam steuerbar | Regler Links/Rechts/Logo einzeln verstellen; danach alle gleich | Jeder Bereich reagiert einzeln; gemeinsame Werte möglich | SW | ✔ |
| F2 | Mehrere Modi per App wählbar | Jeden der 17 Modi antippen | Badge + Ausgabe wechseln je Modus | SW | ✔ |
| F3 | Automatik tageszeitabhängig, Nachtabschaltung ab 23:00 | Systemuhr über Tagesverlauf prüfen (Mock zeigt Live-Wert) | Morgenrampe → Tag → Abendrampe → Nacht = 0 | SW | ✔ |
| F3-HW | Nachtabschaltung real | Uhrzeit am Gerät auf 23:05 stellen | Segmente + Logo dunkel | HW | [ ] |
| F4 | Helligkeit je Bereich manuell | `left/right/logo` per App senden | Wert wird übernommen und angezeigt | SW | ✔ |
| F5 | Konfiguration persistent | Werte ändern, Gerät neu starten | Manuelle Helligkeiten wieder da (NVS) | HW | [ ] |
| F6 | Definierter Zustand nach Stromausfall | Stromlos schalten, wieder einschalten | Startet in Automatik, Helligkeit nach Uhrzeit | HW | [ ] |
| F7 | RTC-Zeitbasis + NTP-Abgleich | Ohne WLAN Zeit prüfen; mit WLAN NTP abwarten | RTC läuft autark; NTP korrigiert inkl. Sommerzeit | HW | [ ] |
| F8 | OTA-Update über lokales Netz | `curl --user admin:… -F "update=@firmware.bin" http://led-fassade.local/update` | Update ok, Neustart | HW | [ ] |
| F8-Sec | OTA ohne/falsches Passwort abgewiesen | Upload ohne bzw. mit falschem Passwort | HTTP 401, keine Firmware geschrieben | HW | [ ] |
| F9 | Bedienung nur lokal, kein Fernzugriff | Netzwerkverkehr sichten; nach Cloud-Verbindungen suchen | Keine ausgehende Verbindung ins Internet außer NTP | SW/HW | ✔ (SW) |

---

## 3. Abnahmekriterien (Pflichtenheft Kap. 12)

| Nr. | Kriterium | Durchführung | Erwartetes Ergebnis | Art | Ergebnis |
| --- | --- | --- | --- | --- | --- |
| A1 | Bereiche einzeln + gemeinsam | siehe F1 | wie F1 | SW | ✔ |
| A2 | Automatik-Kurve korrekt, Abschaltung 23:00 | Tageskurve im Mock über 24 h nachfahren | Werte folgen der Kurve, 23–06 Uhr = 0 | SW | ✔ |
| A3 | Manuelle Änderung < 500 ms | Regler bewegen, Reaktion beobachten | Sichtbare Reaktion praktisch sofort | SW | ✔ |
| A3-HW | Reaktionszeit am Gerät | Regler bewegen, LED-Reaktion messen | < 500 ms (NFR Kap. 5) | HW | [ ] |
| A4 | Zustand nach Stromausfall nach Uhrzeit | siehe F6 | wie F6 | HW | [ ] |
| A5 | Konfiguration bleibt erhalten | siehe F5 | wie F5 | HW | [ ] |
| A6 | Helligkeits-/Strombegrenzung greift | Vollweiß in Statisch 100 %; Strom messen | Strom bleibt unter `LED_MAX_MILLIAMPS`; Software-Deckel aktiv | HW | [ ] |
| A7 | Bedienung ausschließlich lokal | siehe F9 | wie F9 | SW/HW | ✔ (SW) |
| A8 | OTA-Update durchführbar | siehe F8 | wie F8 | HW | [ ] |

---

## 4. Zusätzliche Robustheits-Tests

| Nr. | Testfall | Durchführung | Erwartetes Ergebnis | Art | Ergebnis |
| --- | --- | --- | --- | --- | --- |
| R1 | WLAN-Ausfall & Reconnect | Access-Point kurz abschalten | Firmware verbindet automatisch neu (≤ 30 s) | HW | [ ] |
| R2 | Fallback-Setup-AP | Ohne erreichbares WLAN starten | AP „Fassade-Setup" erscheint, App erreichbar | HW | [ ] |
| R3 | Ungültige Zeit (RTC leer, kein NTP) | RTC entfernen/entladen, offline starten | Automatik fährt sicheren Grundzustand (`SAFE_DEFAULT_BRIGHTNESS`) | HW | [ ] |
| R4 | LED-Selbsttest beim Start | Gerät einschalten | Rot/Grün/Blau/Weiß durchlaufen, dann Betrieb | HW | [ ] |
| R5 | Override-Rückkehr (Variante a) | In Automatik Regler verstellen → Zeitfenster-Wechsel abwarten | Nach nächstem Fensterwechsel zurück in Automatik | SW | ✔ |
| R6 | Ungültige WebSocket-Nachricht | Fehlerhaftes/zu langes JSON senden | Nachricht verworfen, kein Absturz | SW | ✔ |
| R7 | Modus „Aus" | Modus Aus wählen | Bereiche dunkel, Banner in App, Controller erreichbar | SW | ✔ |

---

## 5. Zusammenfassung

- **Per Simulator/Mock bestätigt (SW):** F1, F2, F3, F4, F9 (SW), A1, A2, A3,
  A7 (SW), R5, R6, R7.
- **Am Gerät abzunehmen (HW):** F3-HW, F5, F6, F7, F8, F8-Sec, A3-HW, A4, A5, A6,
  A8, R1–R4.

Die HW-Zeilen sind bei der Endabnahme am aufgebauten System auszuführen und mit
Datum/Prüfer abzuzeichnen.

Datum: ____________   Prüfer: ____________   Unterschrift: ____________
