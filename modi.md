# Betriebsmodi – LED-Fassade

Übersicht und Erklärung aller **20 Leuchtmodi**. Das Licht ist ausschließlich
**weiß** – die Modus-Namen beschreiben also die *Bewegung/Dynamik*, nicht die
Farbe.

**Grundsätzliches**

- **Grundmodi** (Aus, Statisch, Automatik): der normale Betrieb.
- **Effekte**: animierte Modi. `EFFECT_BRIGHTNESS` in `config.h` ist die interne
  Grundstärke; die tatsächliche Helligkeit stellst du je Segment über die Regler.
  Keine Nachtabschaltung.
- **Stimmungs-Modi**: feste Lichtstimmungen mit im Code hinterlegten Werten. Sie
  schalten sich **zwischen 23:00 und 06:00 automatisch ab**.
- **Regler in allen Modi**: außer in der Automatik bestimmen die Regler
  (Links/Rechts/Logo) die Helligkeit. Bei jedem **Moduswechsel starten sie bei
  90 %** und sind dann frei verstellbar.
- **Fassaden-Abstimmung**: Alle Modi außer Automatik sind auf die Wirkung an
  einer Außenwand ausgelegt – enge Helligkeitsbereiche (wenig Kontrast),
  langsame Bewegung und angehobene Grundhelligkeit, damit die Fassade aus
  Distanz ruhig und gleichmäßig wirkt.
- **Regler im Automatik-Modus**: dort steuert die Uhrzeit die Helligkeit, daher
  sind die Regler in der App gesperrt.

Die konkreten Werte stehen in [`include/config.h`](include/config.h)
(Stimmungs-Modi und Effekt-Parameter) bzw. in den `applyWave(...)`-Aufrufen in
[`src/main.cpp`](src/main.cpp) (Pulsieren/Atmen).

## Übersicht

| Nr. | Modus | Art | Wirkung | Helligkeit |
| --- | ----- | --- | ------- | ---------- |
| 0  | Aus            | Grundmodus | alles aus | – |
| 1  | Statisch       | Grundmodus | konstantes Weiß je Bereich | Regler je Bereich |
| 2  | Lauflicht      | Effekt     | weicher, gleitender Lichtschweif | Effekt-Helligkeit |
| 3  | Automatik      | Grundmodus | tageszeitabhängig | fest (Tageskurve) |
| 4  | Pulsieren      | Effekt     | ruhiges gemeinsames Schwellen | Effekt-Helligkeit |
| 5  | Atmen          | Effekt     | sehr langsames Ein-/Ausatmen | Effekt-Helligkeit |
| 10 | Welle          | Effekt     | langsam wandernde Hell-Dunkel-Bänder | Effekt-Helligkeit |
| 6  | Dauerlicht     | Stimmung   | gleichmäßig, kaum atmend | ~75–84 % |
| 7  | Kerzenlicht    | Stimmung   | zwei langsam schimmernde Lichter | ~65–78 % |
| 8  | Stufenlicht    | Stimmung   | Aufbau in acht Schritten | ~86 % |
| 9  | Dämmerlicht    | Stimmung   | sanfter, niedriger Abendglanz | ~22–37 % |
| 11 | Feuerschein    | Stimmung   | ruhiges, warmes Lodern | ~69–84 % |
| 12 | Nachtlicht     | Stimmung   | ruhiger, niedriger Grundglanz | ~22–31 % |
| 13 | Sternenfunkeln | Effekt     | Grundglanz mit verglimmenden Funken | Effekt-Helligkeit |
| 14 | Treffpunkt     | Effekt     | zwei Lichter treffen sich in der Mitte | Effekt-Helligkeit |
| 15 | Herzschlag     | Effekt     | ruhiger Doppelschlag der Fassade | Effekt-Helligkeit |
| 16 | Wechsellicht   | Effekt     | Links/Rechts gegenläufig | Effekt-Helligkeit |
| 17 | Ausstrahlung   | Effekt     | Welle vom Logo/Zentrum nach außen | Effekt-Helligkeit |
| 18 | Wolkenzug      | Effekt     | organische Helligkeit wie ziehende Wolken | Effekt-Helligkeit |
| 19 | Leuchtturm     | Effekt     | weiches Lichtband wandert über die Linie | Effekt-Helligkeit |

---

## Grundmodi

### 0 – Aus
Alle Bereiche aus. Der Controller bleibt im WLAN erreichbar. Die Helligkeit
fährt sanft herunter (kein harter Sprung).

### 1 – Statisch
Konstantes, ruhiges Weiß. Segment Links, Segment Rechts und das Logo sind
**einzeln** über die Regler (0–100 %) einstellbar. Helligkeitswechsel erfolgen
weich.

### 3 – Automatik (Standardmodus)
Tageszeitabhängige Helligkeit nach fester Kurve:

```text
06:00–08:00  Hochfahren  →  ~90 %
08:00–18:00  Tag             ~90 %
18:00–23:00  Abend       60 % → 25 %
23:00–06:00  Nacht           aus
```

Uhrzeiten (`AUTO_T_*`) und Helligkeiten (`AUTO_B_*`) sind **fest in `config.h`**
hinterlegt. Nach einem Stromausfall startet der Controller immer in diesem Modus
(siehe Failsafe). In der App wird das Profil als schreibgeschützte
Phasen-Übersicht angezeigt – sie erscheint **nur, wenn der Automatik-Modus aktiv
ist**.

---

## Effekte

Alle Effekte nutzen `EFFECT_BRIGHTNESS` (`config.h`) als Grundstärke und werden
über die **Regler** je Segment skaliert; sie unterliegen **keiner**
Nachtabschaltung.

### 2 – Lauflicht
Ein weicher Lichtschweif gleitet langsam über beide Segmente. Statt eines harten
Einzelpunkts klingt der Streifen leicht ab und der Kopf wird neu gesetzt – so
entsteht ein ruhig gleitender Schweif (Schritt ≈ 80 ms).

### 4 – Pulsieren
Der ganze Streifen schwillt gemeinsam heller und dunkler – ruhig (≈ 20 „Schläge"
pro Minute) und **ohne ins Dunkle abzusacken** (bleibt ≥ ~47 % der
Effekt-Helligkeit).

### 5 – Atmen
Wie Pulsieren, aber **sehr langsam** und sanft (≈ 6 pro Minute), Grundwert
≥ ~39 %. Ruhiges „Ein- und Ausatmen".

### 10 – Welle
Eine Sinuswelle wandert langsam räumlich über den Streifen. Die Wellentäler
bleiben bei ~45 % erhalten, sodass sanfte Hell-Dunkel-Bänder statt harter
dunkler Lücken entstehen.

### 13 – Sternenfunkeln
Ein dezenter Grundglanz (~18 % der Effekt-Helligkeit), auf dem einzelne Funken
kurz aufleuchten und langsam verglimmen – wie ein ruhiger Sternenhimmel.
Parameter: `TWINKLE_CHANCE` (Häufigkeit), `TWINKLE_FADE` (Abklingen),
`TWINKLE_BASE_PCT` (Grundglanz).

### 14 – Treffpunkt
Je ein Lichtschweif läuft von außen nach innen; beide **treffen sich in der
Mitte** (beim Logo/Eingang) und beginnen dann von vorn. Symmetrisch und ruhig –
betont den Eingang.

### 15 – Herzschlag
Zwei kurze Schläge je Zyklus (≈ 2,2 s), dazwischen ein ruhiges Grundniveau
(~25 %) – ein sanftes „lub-dub" der ganzen Fassade. Parameter: `HEART_PERIOD_MS`,
`HEART_LOW_PCT`.

### 16 – Wechsellicht
Segment Links und Rechts schwellen **langsam gegenläufig**: während die eine
Seite heller wird, dimmt die andere ab (Zyklus ≈ 9 s, dunklere Seite ~35 %).
Parameter: `WECHSEL_PERIOD_MS`, `WECHSEL_LOW_PCT`.

### 17 – Ausstrahlung
Eine langsame Welle läuft **vom Logo bzw. der Mitte nach außen** über beide
Segmente – als würde die Fassade vom Eingang her „atmen". Das Logo pulst im
Ursprung mit. Parameter: `AUSSTR_SPACING`, `AUSSTR_SPEED_DIV`, `AUSSTR_FLOOR_PCT`.

### 18 – Wolkenzug
**Organische, unregelmäßige** Helligkeitsschwankungen ziehen langsam über die
Linie (Perlin-Rauschen) – ohne sichtbares Muster, wie vorbeiziehende Wolken.
Parameter: `WOLKEN_SCALE`, `WOLKEN_SPEED_DIV`, `WOLKEN_FLOOR_PCT`.

### 19 – Leuchtturm
Ein **weiches Lichtband** wandert langsam über die ganze Linie (beide Segmente
als eine Reihe) und kommt periodisch wieder; dazwischen ein niedriges
Grundniveau. Das Logo leuchtet auf, wenn der Strahl die Mitte passiert.
Parameter: `LEUCHTTURM_PERIOD_MS`, `LEUCHTTURM_WIDTH`, `LEUCHTTURM_FLOOR_PCT`.

---

## Stimmungs-Modi

Feste Lichtstimmungen für den Ausstellungsbetrieb: der Verlauf ist im Code
hinterlegt, die Gesamthelligkeit skalierst du über die Regler (Standard 90 %).
**Nachtabschaltung 23:00–06:00.**

### 6 – Dauerlicht
Gleichmäßiges, ganz langsam „atmendes" Licht auf hohem Niveau (~75–84 %, ein
Atemzug ≈ 24 s). Geht (außer nachts) nie ganz aus. Parameter: `DAUER_*`.

### 7 – Kerzenlicht
Zwei unabhängig voneinander **langsam schimmernde** Lichter (je eines pro
Segment, ~65–78 %). Parameter: `KERZEN_*`.

### 8 – Stufenlicht
Die Lichter bauen sich in acht Schritten auf (Schritt ≈ 2,2 s), halten kurz
(≈ 5 s) und beginnen von vorn. Ein langsamer, ruhiger Effekt. Parameter:
`STUFEN_*`.

### 9 – Dämmerlicht
Sanfter, niedriger Abendglanz, der über Minuten ganz langsam auf- und abschwillt
(~22–37 %, Durchlauf ≈ 3 min) – noch aus Distanz sichtbar. Parameter:
`DAEMMER_*`.

### 11 – Feuerschein
Ein ruhiges, warmes Lodern über beide Segmente – langsam und wenig
kontrastreich (~69–84 %). Parameter: `FEUER_*`.

### 12 – Nachtlicht
Ein ruhiger, niedriger Grundglanz, sehr langsam und kaum bewegt (~22–31 %).
Parameter: `NACHT_*`.

---

## Werte anpassen

- **Stimmungs-Modi und Effekt-Parameter:** `include/config.h`
  (`DAUER_*`, `KERZEN_*`, `STUFEN_*`, `DAEMMER_*`, `FEUER_*`, `NACHT_*`,
  `TWINKLE_*`, `HEART_*`, `WECHSEL_*`, `AUSSTR_*`, `WOLKEN_*`, `LEUCHTTURM_*`).
- **Automatik-Profil:** `include/config.h` (`AUTO_T_*`, `AUTO_B_*`).
- **Effekt-Helligkeit:** `include/config.h` (`EFFECT_BRIGHTNESS`).
- **Pulsieren/Atmen:** die `applyWave(...)`-Aufrufe in `src/main.cpp`.

Die Modus-Namen und -Nummern müssen zwischen `include/config.h`
(`enum OperatingMode`) und `src/web_page.h` (`MODE_LABEL`) übereinstimmen.
