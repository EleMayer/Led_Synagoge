# Einen neuen Modus hinzufügen

Kurzanleitung mit Code-Schablone. Ein Modus wird an **fünf** Stellen eingetragen –
danach einmal bauen und flashen. Beispiel: der neue Modus bekommt die Nummer **20**
(die aktuell höchste ist 19).

| Datei | Was eintragen |
| --- | --- |
| `include/config.h` | Enum-Wert (+ optionale Parameter) |
| `src/main.cpp` | `render…()`-Funktion + `case` im `switch` |
| `web/index.html` | Button + Name (DE/EN) |
| `tools/mock-server.js` · `tools/build-demo.js` | Modus-Obergrenze anheben |

---

## Schritt 1 – `include/config.h`

Im `enum OperatingMode` einen Wert ergänzen und `MODE_LAST` hochsetzen:

```cpp
    MODE_LEUCHTTURM  = 19,
    MODE_MEINMODUS   = 20,   // NEU
    MODE_LAST        = MODE_MEINMODUS
};
```

Optional darunter Parameter definieren (Tempo, Grundhelligkeit …), damit du sie
später leicht anpassen kannst:

```cpp
// Mein Modus: kurze Beschreibung.
#define MEIN_SPEED_DIV   16     // groesser = langsamer
#define MEIN_FLOOR_PCT   45     // dunkelster Anteil in % der Effekt-Helligkeit
```

---

## Schritt 2 – `src/main.cpp`

### a) Die Render-Funktion (Schablone für einen **Effekt**)

Am besten neben die anderen `render…()`-Funktionen setzen:

```cpp
// Mein Modus: was er optisch macht.
void renderMeinModus() {
    if (!frameReady(RENDER_INTERVAL)) return;      // begrenzt die Bildrate

    uint8_t base  = brightnessTo8Bit(globalBrightness);   // Effekt-Grundstaerke (0..255)
    uint8_t floorLevel = (uint16_t)base * MEIN_FLOOR_PCT / 100;
    uint16_t phase = millis() / MEIN_SPEED_DIV;           // laeuft mit der Zeit

    for (uint16_t i = 0; i < NUM_LEDS_LINKS; i++) {
        uint8_t w = sin8(i * 8 + phase);               // Wellenform je LED
        ledsLinks[i] = whiteRaw(map(w, 0, 255, floorLevel, base));
    }
    for (uint16_t i = 0; i < NUM_LEDS_RECHTS; i++) {
        uint8_t w = sin8(i * 8 + phase);
        ledsRechts[i] = whiteRaw(map(w, 0, 255, floorLevel, base));
    }

    showScaled();                                   // mit den Reglern skalieren + ausgeben
    setLogoEffect(base);                            // Logo (auch reglerskaliert)
}
```

Wichtig für Effekte: **immer** `showScaled()` statt `FastLED.show()` und `setLogoEffect(...)`
statt `setLogoRaw(...)` verwenden – nur so wirken die Regler auch in deinem Modus.

### b) Den `case` im `switch` von `applyHardware()` ergänzen

```cpp
        case MODE_LEUCHTTURM:
            renderLeuchtturm();
            break;

        case MODE_MEINMODUS:        // NEU
            renderMeinModus();
            break;

        default:
            allLEDsOff();
            break;
```

---

## Schritt 3 – `web/index.html`

### a) Button in der Modus-Karte (bei den anderen Buttons):

```html
    <button class="mode" data-mode="20" onclick="sendMode(20)">Mein Modus</button>
```

### b) Name je Sprache im `MODE_LABEL` (DE **und** EN):

```js
  de:{ /* … */ 19:"Leuchtturm", 20:"Mein Modus" },
  en:{ /* … */ 19:"Lighthouse", 20:"My mode" }
```

---

## Schritt 4 – Mock & Demo

In `tools/mock-server.js` und `tools/build-demo.js` jeweils die Obergrenze anheben,
damit die Testumgebung den neuen Modus annimmt:

```js
if (typeof doc.mode === 'number' && doc.mode >= 0 && doc.mode <= 20) {   // war 19
```

Danach die Weboberfläche zusammenbauen und die Demo neu erzeugen:

```bash
node tools/build-web.js     # web/*  ->  src/web_page.h
node tools/build-demo.js    # aktualisiert die Demo
```

---

## Schritt 5 – Bauen und aufspielen

```bash
pio run              # nur bauen (ohne Hardware moeglich)
pio run -t upload    # auf den ESP32 flashen (per USB)
```

Testen kannst du das Umschalten vorher schon am PC über den Mock
(`node tools/mock-server.js` → `http://localhost:5598`).

---

## Nützliche Bausteine (schon vorhanden)

| Baustein | Zweck |
| --- | --- |
| `sin8(x)` | Sinus-Wellenform 0..255 (aus FastLED) |
| `beatsin8(bpm, lo, hi)` | ruhiges Schwingen zwischen `lo` und `hi` |
| `inoise8(x, y)` | Perlin-Rauschen (organisch, ohne Muster) |
| `whiteRaw(v)` | weißer Farbwert mit 8-Bit-Helligkeit `v` |
| `fill_solid(leds, n, farbe)` | ein Segment komplett füllen |
| `fadeToBlackBy(leds, n, amount)` | Segment leicht abdunkeln (für Schweif-Effekte) |
| `showScaled()` | beide Segmente reglerskaliert ausgeben |
| `setLogoEffect(base)` | Logo reglerskaliert setzen |

---

## Variante: ein **ruhiger** Modus mit weichem Übergang (statt Effekt)

Soll dein Modus wie „Statisch/Automatik" nur Zielwerte setzen und die Fade-Engine
nutzen (kein eigenes Rendern), dann:

1. Nimm ihn in `isEffectMode()` aus (wie `MODE_AUTO_MANUAL` früher), damit `loop()`
   ihn über `renderSolid()` weich blendet.
2. Deine Funktion setzt dann nur die Ziele, z. B.:

```cpp
void applyMeinRuhigerModus() {
    targetLeft  = 60;   // Prozent
    targetRight = 60;
    targetLogo  = 60;
}
```

Für **animierte** Effekte ist aber die Schablone oben (mit `showScaled()`) der Normalfall.
