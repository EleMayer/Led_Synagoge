#pragma once

// ---------------------------------------------------------------------------
// Reine, hardwareunabhaengige Automatik-Logik (kein Arduino, FastLED oder RTC).
//
// Diese Funktionen berechnen nur mit Zahlen und werden von zwei Seiten genutzt:
//   - src/main.cpp     (die echte Firmware)
//   - test/            (die Unit-Tests, siehe test/test_auto_logic.cpp)
//
// So testen die Unit-Tests genau denselben Code, der auf dem Geraet laeuft.
// ---------------------------------------------------------------------------

// Das Automatik-Tagesprofil: vier Uebergangs-Stunden (0..23) und vier
// Helligkeiten (0..100 %). Muss aufsteigend sein: tMorning < tDay < tEvening < tNight.
struct AutoProfile {
    int tMorning;    // Beginn Hochfahren
    int tDay;        // Beginn Tageshelligkeit
    int tEvening;    // Beginn Abendrampe
    int tNight;      // Nachtabschaltung (bis tMorning aus)
    int bMorning;    // Zielhelligkeit am Ende des Hochfahrens
    int bDay;        // konstante Tageshelligkeit
    int bEveStart;   // Abendrampe Beginn
    int bEveEnd;     // Abendrampe Ende
};

// Begrenzt einen Prozentwert auf 0..100.
inline int clampPct(int value) {
    if (value < 0)   return 0;
    if (value > 100) return 100;
    return value;
}

// Prozent (0..100) -> 8-Bit-Helligkeit (0..255), wie Arduinos map(v,0,100,0,255).
inline int pctTo8bit(int pct) {
    int v = clampPct(pct);
    return v * 255 / 100;
}

// Rundet einen nicht-negativen Wert kaufmaennisch auf eine ganze Zahl.
inline int roundToInt(double x) {
    return (int)(x + 0.5);
}

// Aktuelles Zeitfenster fuer eine Uhrzeit (Minuten seit Mitternacht):
//   0 = Nacht, 1 = Morgen (Hochfahren), 2 = Tag, 3 = Abend.
inline int autoWindowAt(int minutes, const AutoProfile& p) {
    if (minutes >= p.tNight * 60 || minutes < p.tMorning * 60) return 0;  // Nacht
    if (minutes < p.tDay * 60)     return 1;   // Morgen
    if (minutes < p.tEvening * 60) return 2;   // Tag
    return 3;                                   // Abend
}

// Automatik-Helligkeit (0..100 %) fuer eine Uhrzeit (Minuten seit Mitternacht,
// darf Nachkommateil fuer Sekunden enthalten).
//   Nacht  -> 0
//   Morgen -> lineare Rampe 0 .. bMorning
//   Tag    -> bDay
//   Abend  -> lineare Rampe bEveStart .. bEveEnd
inline int autoBrightnessAt(double minutes, const AutoProfile& p) {
    // Nacht
    if (minutes >= p.tNight * 60.0 || minutes < p.tMorning * 60.0) {
        return 0;
    }

    // Morgenrampe
    if (minutes < p.tDay * 60.0) {
        double start = p.tMorning * 60.0;
        double end   = p.tDay * 60.0;
        double f = (minutes - start) / (end - start);
        if (f < 0.0) f = 0.0;
        if (f > 1.0) f = 1.0;
        return roundToInt(f * p.bMorning);
    }

    // Tag
    if (minutes < p.tEvening * 60.0) {
        return p.bDay;
    }

    // Abendrampe
    double start = p.tEvening * 60.0;
    double end   = p.tNight * 60.0;
    double f = (minutes - start) / (end - start);
    if (f < 0.0) f = 0.0;
    if (f > 1.0) f = 1.0;
    return roundToInt(p.bEveStart + (p.bEveEnd - p.bEveStart) * f);
}
