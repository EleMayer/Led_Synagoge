// ---------------------------------------------------------------------------
// LED-Fassadenbeleuchtung - ESP32-Firmware
//
// Steuert zwei weisse LED-Segmente (Links/Rechts, WS2812 ueber FastLED) und ein
// dimmbares Logo (PWM/MOSFET). Bedient wird lokal ueber eine Web-App per WLAN
// (WebSocket fuer Live-Steuerung, REST fuer Status). Zeitbasis liefert ein
// DS3231-RTC, bei Internet zusaetzlich NTP.
//
// Steuermodell: Der aktive Modus setzt fuer jeden Bereich einen Zielwert
// (targetLeft/Right/Logo in Prozent). renderSolid() fuehrt die tatsaechlich
// angezeigten Werte (shownLeft/...) in kleinen Schritten sanft an das Ziel
// heran (weiche Uebergaenge). Effekt-Modi rendern jeden Frame direkt.
//
// Alle Pins, LED-Anzahlen, Grenzwerte und die Modus-Aufzaehlung stehen in
// include/config.h.
// ---------------------------------------------------------------------------

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <RTClib.h>
#include <FastLED.h>
#include <Preferences.h>
#include <Update.h>
#include <time.h>

#include "config.h"
#include "web_page.h"
#include "icons.h"

// Aktueller Betriebsmodus (siehe OperatingMode in config.h).
OperatingMode currentMode = MODE_AUTOMATIC;

// Override: ein manueller Eingriff hat die Automatik uebersteuert.
// overrideWindow merkt sich das Zeitfenster des Eingriffs, damit beim naechsten
// Fensterwechsel automatisch in die Automatik zurueckgekehrt wird.
bool overrideActive = false;
int  overrideWindow = -1;

// Manuelle Helligkeiten je Bereich (0-100 %), wirken im Modus Statisch.
int brightnessLeft  = 80;
int brightnessRight = 80;
int brightnessLogo  = 80;
// Gemeinsame Helligkeit fuer die Effekt-Modi (Lauflicht, Welle, ...).
int globalBrightness = 80;

// Zeitprofil der Automatik (Stunden der Uebergaenge). Parametrierbar per App.
// Zeitfenster der Automatik: fest aus config.h abgeleitet, nicht ueber die App
// aenderbar. Jedes Fenster endet dort, wo das naechste beginnt.
const int morningStartHour = AUTO_T_MORNING;
const int morningEndHour   = AUTO_T_DAY;
const int dayStartHour     = AUTO_T_DAY;
const int dayEndHour       = AUTO_T_EVENING;
const int eveningStartHour = AUTO_T_EVENING;
const int eveningEndHour   = AUTO_T_NIGHT;
const int nightStartHour   = AUTO_T_NIGHT;
const int nightEndHour     = AUTO_T_MORNING;

// Automatik-Helligkeiten: fest aus config.h, nicht ueber die App aenderbar.
const int morningBrightness      = AUTO_B_MORNING;
const int dayBrightness          = AUTO_B_DAY;
const int eveningStartBrightness = AUTO_B_EVE_START;
const int eveningEndBrightness   = AUTO_B_EVE_END;
int currentAutoBrightness  = 0;

// Fade-Engine: target* ist der Sollwert des Modus, shown* der aktuell
// angezeigte Wert. renderSolid() naehert shown* pro Frame um FADE_STEP an
// target* an, damit Helligkeitswechsel weich statt sprunghaft erfolgen.
int   targetLeft  = 0;
int   targetRight = 0;
int   targetLogo  = 0;
float shownLeft   = 0;
float shownRight  = 0;
float shownLogo   = 0;
const float FADE_STEP = 2.0f;

unsigned long lastRender = 0;
const uint32_t RENDER_INTERVAL = 20;
bool prevFrameEffect = false;

CRGB ledsLinks[NUM_LEDS_LINKS];
CRGB ledsRechts[NUM_LEDS_RECHTS];

#define MAX_WS_MESSAGE_LEN 512

RTC_DS3231 rtc;           // Echtzeituhr (netzunabhaengige Zeitbasis)
Preferences preferences;  // NVS-Speicher fuer die Einstellungen
AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // Live-Steuerung und Statuspush an die App

bool rtcAvailable  = false; // DS3231 am I2C-Bus gefunden
bool wifiConnected = false; // mit dem konfigurierten WLAN verbunden
bool ntpSynced     = false; // Uhrzeit mindestens einmal per NTP geholt
bool apMode        = false; // Fallback: eigener Setup-Accesspoint aktiv

const char *NTP_SERVER_1 = "pool.ntp.org";
const char *NTP_SERVER_2 = "time.nist.gov";
const char *TZ_INFO = "CET-1CEST,M3.5.0,M10.5.0";

uint16_t      effectStep = 0;
unsigned long lastEffectUpdate = 0;
const uint32_t EFFECT_INTERVAL = 80;
unsigned long lastAutoUpdate = 0;
const uint32_t AUTO_UPDATE_INTERVAL = 1000;
unsigned long lastStatusBroadcast = 0;
const uint32_t STATUS_INTERVAL = 2000;
unsigned long lastNtpCheck = 0;
const uint32_t NTP_CHECK_INTERVAL = 60000;

bool settingsDirty = false;
unsigned long lastSettingsChange = 0;
const uint32_t SAVE_DEBOUNCE_MS = 1500;

void broadcastStatus();
void applyHardware();
void applyAutomatic();

// --- Helligkeits-Hilfsfunktionen -------------------------------------------

// Begrenzt einen Prozentwert sicher auf den Bereich 0-100.
int clampBrightness(int value) {
    return constrain(value, 0, 100);
}

// Rechnet Prozent (0-100) in einen 8-Bit-Wert (0-255) fuer LED/PWM um.
uint8_t brightnessTo8Bit(int brightness) {
    int safe = clampBrightness(brightness);
    return map(safe, 0, 100, 0, 255);
}

CRGB whiteWithBrightness(int brightness) {
    uint8_t v = brightnessTo8Bit(brightness);
    return CRGB(v, v, v);
}

// --- Persistenz im NVS-Flash -----------------------------------------------
// Die Einstellungen ueberstehen einen Stromausfall (Pflichtenheft F5).

// Schreibt Helligkeiten und Zeitprofil in den NVS.
void saveSettings() {
    preferences.begin("facelight", false);
    preferences.putInt("left", brightnessLeft);
    preferences.putInt("right", brightnessRight);
    preferences.putInt("logo", brightnessLogo);
    preferences.putInt("global", globalBrightness);
    // Automatik-Zeitprofil (Uhrzeiten und Helligkeiten) ist fest im Code
    // (config.h) - wird bewusst nicht im NVS gespeichert.
    preferences.end();
    settingsDirty = false;
}

// Merkt eine Aenderung vor; loop() speichert sie zeitverzoegert (entprellt),
// damit schnelle Reglerbewegungen nicht bei jedem Schritt ins Flash schreiben.
void markSettingsDirty() {
    settingsDirty = true;
    lastSettingsChange = millis();
}

// Laedt die Einstellungen aus dem NVS; fehlt ein Wert, greift der Default.
void loadSettings() {
    preferences.begin("facelight", true);
    brightnessLeft         = preferences.getInt("left", 80);
    brightnessRight        = preferences.getInt("right", 80);
    brightnessLogo         = preferences.getInt("logo", 80);
    globalBrightness       = preferences.getInt("global", 80);
    // Automatik-Zeitprofil steht fest in config.h und wird nicht geladen.
    preferences.end();

    brightnessLeft         = clampBrightness(brightnessLeft);
    brightnessRight        = clampBrightness(brightnessRight);
    brightnessLogo         = clampBrightness(brightnessLogo);
    globalBrightness       = clampBrightness(globalBrightness);
}

// --- Zeitbasis (RTC / NTP) -------------------------------------------------

// Plausibilitaetscheck: eine leere DS3231-Pufferbatterie liefert ein unsinniges
// Jahr. Nur ein Jahr im erwarteten Bereich gilt als gueltige Zeit.
bool isRTCValid() {
    if (!rtcAvailable) {
        return false;
    }
    DateTime now = rtc.now();
    return now.year() >= 2024 && now.year() <= 2099;
}

// Liefert die aktuelle Zeit. Bevorzugt die RTC (auch ohne Netz verfuegbar),
// sonst die per NTP gesetzte Systemzeit. false = keine gueltige Zeit.
bool getCurrentTime(struct tm &out) {
    if (rtcAvailable && isRTCValid()) {
        DateTime now = rtc.now();
        out.tm_year = now.year() - 1900;
        out.tm_mon  = now.month() - 1;
        out.tm_mday = now.day();
        out.tm_hour = now.hour();
        out.tm_min  = now.minute();
        out.tm_sec  = now.second();
        return true;
    }
    return getLocalTime(&out, 5);
}

String getTimeString() {
    struct tm t;
    if (!getCurrentTime(t)) {
        return "--:--:--";
    }
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
    return String(buf);
}

String getDateString() {
    struct tm t;
    if (!getCurrentTime(t)) {
        return "--.--.----";
    }
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d.%02d.%04d", t.tm_mday, t.tm_mon + 1, t.tm_year + 1900);
    return String(buf);
}

// --- Automatik (tageszeitabhaengige Helligkeit) ----------------------------

// Ordnet die aktuelle Uhrzeit einem Zeitfenster zu:
// 0 = Nacht/aus, 1 = Hochfahren, 2 = Tag, 3 = Abend. -1 = keine gueltige Zeit.
// Dient dem Override, um den Fensterwechsel als Rueckkehrpunkt zu erkennen.
int currentAutoWindow() {
    struct tm t;
    if (!getCurrentTime(t)) {
        return -1;
    }
    int m = t.tm_hour * 60 + t.tm_min;

    if (m >= nightStartHour * 60 || m < nightEndHour * 60) {
        return 0;
    }
    if (m >= morningStartHour * 60 && m < morningEndHour * 60) {
        return 1;
    }
    if (m >= dayStartHour * 60 && m < dayEndHour * 60) {
        return 2;
    }
    if (m >= eveningStartHour * 60 && m < eveningEndHour * 60) {
        return 3;
    }
    return 0;
}

// Berechnet die Zielhelligkeit der Automatik fuer die aktuelle Uhrzeit.
// Hochfahren und Abend werden linear ueberblendet, Tag ist konstant, nachts 0.
int calculateAutomaticBrightness() {
    struct tm t;
    if (!getCurrentTime(t)) {
        // Keine gueltige Zeitbasis -> sicherer, gedimmter Default (Kap. 8.3).
        return SAFE_DEFAULT_BRIGHTNESS;
    }
    float m = t.tm_hour * 60.0f + t.tm_min;

    if (m >= nightStartHour * 60 || m < nightEndHour * 60) {
        return 0;
    }

    if (m >= morningStartHour * 60 && m < morningEndHour * 60) {
        float start = morningStartHour * 60.0f;
        float end   = morningEndHour * 60.0f;
        float p = (m - start) / (end - start);
        p = constrain(p, 0.0f, 1.0f);
        return round(p * morningBrightness);
    }

    if (m >= dayStartHour * 60 && m < dayEndHour * 60) {
        return dayBrightness;
    }

    if (m >= eveningStartHour * 60 && m < eveningEndHour * 60) {
        float start = eveningStartHour * 60.0f;
        float end   = eveningEndHour * 60.0f;
        float p = (m - start) / (end - start);
        p = constrain(p, 0.0f, 1.0f);
        return round(eveningStartBrightness + (eveningEndBrightness - eveningStartBrightness) * p);
    }

    return 0;
}

// --- Ausgabe: Fade-Engine und einfache Modi --------------------------------

// Setzt die Logo-Helligkeit (Prozent) am PWM-Kanal. Zusaetzlich auf
// GLOBAL_MAX_BRIGHTNESS begrenzt, damit das Logo zur Streifenhelligkeit passt.
void setLogoBrightness(int brightness) {
    uint32_t duty = brightnessTo8Bit(clampBrightness(brightness));
    duty = duty * GLOBAL_MAX_BRIGHTNESS / 255;
    ledcWrite(LOGO_PWM_CHANNEL, duty);
}

// Bewegt einen Wert um einen FADE_STEP in Richtung Ziel, ohne darueber hinaus.
float approach(float current, int target) {
    if (current < target) {
        current += FADE_STEP;
        if (current > target) {
            current = target;
        }
    } else if (current > target) {
        current -= FADE_STEP;
        if (current < target) {
            current = target;
        }
    }
    return current;
}

// Zeichnet die Bereiche mit sanftem Uebergang zum jeweiligen Zielwert.
// Wird fuer alle nicht-animierten Modi (Aus, Statisch, Automatik) genutzt.
void renderSolid() {
    if (millis() - lastRender < RENDER_INTERVAL) {
        return;
    }
    lastRender = millis();

    shownLeft  = approach(shownLeft,  targetLeft);
    shownRight = approach(shownRight, targetRight);
    shownLogo  = approach(shownLogo,  targetLogo);

    fill_solid(ledsLinks,  NUM_LEDS_LINKS,  whiteWithBrightness(round(shownLeft)));
    fill_solid(ledsRechts, NUM_LEDS_RECHTS, whiteWithBrightness(round(shownRight)));
    FastLED.show();
    setLogoBrightness(round(shownLogo));
}

// Modus Aus: alle Zielwerte auf 0 (Streifen faden sanft aus).
void allLEDsOff() {
    targetLeft  = 0;
    targetRight = 0;
    targetLogo  = 0;
}

// Modus Statisch: Zielwerte aus den per App eingestellten Helligkeiten.
void applyStatic() {
    targetLeft  = brightnessLeft;
    targetRight = brightnessRight;
    targetLogo  = brightnessLogo;
}

// Modus Automatik: alle Bereiche auf die tageszeitabhaengige Helligkeit.
void applyAutomatic() {
    int value = calculateAutomaticBrightness();
    currentAutoBrightness = value;
    targetLeft  = value;
    targetRight = value;
    targetLogo  = value;
}

// --- Animierte Modi (Effekte und Stimmungslicht) ---------------------------
// Effekt-Parameter (Grenzen, Geschwindigkeiten) stehen in config.h.

// Taktet die Animationen: liefert nur true, wenn seit dem letzten Frame
// mindestens interval ms vergangen sind (begrenzt die Bildrate).
bool frameReady(uint32_t interval) {
    if (millis() - lastEffectUpdate < interval) {
        return false;
    }
    lastEffectUpdate = millis();
    return true;
}

// Weiches Auf-/Abschwellen (Sinus) zwischen lo und hi mit Periode periodMs.
uint8_t breathLevel(uint32_t periodMs, uint8_t lo, uint8_t hi) {
    uint8_t s = sin8(millis() / (periodMs / 256));
    return map(s, 0, 255, lo, hi);
}

// Ruhiges, zufaelliges Flackern (Perlin-Noise) zwischen lo und hi.
// seed trennt mehrere Kanaele, damit sie unabhaengig flackern.
uint8_t flickerLevel(uint16_t speedDiv, uint16_t seed, uint8_t lo, uint8_t hi) {
    uint8_t n = inoise8(millis() / speedDiv, seed);
    return map(n, 0, 255, lo, hi);
}

// Lauflicht: ein weiches Licht gleitet ueber beide Segmente. Statt eines harten
// Einzelpunkts klingt der bestehende Streifen langsam ab (fadeToBlackBy) und der
// Kopf wird neu gesetzt - so entsteht ein ruhig gleitender Lichtschweif, der auf
// einer Fassade deutlich eleganter wirkt als ein blinkender Punkt.
void applyEffect() {
    if (!frameReady(EFFECT_INTERVAL)) return;

    uint8_t base = brightnessTo8Bit(globalBrightness);

    fadeToBlackBy(ledsLinks,  NUM_LEDS_LINKS,  60);
    fadeToBlackBy(ledsRechts, NUM_LEDS_RECHTS, 60);

    ledsLinks[effectStep % NUM_LEDS_LINKS]   = CRGB(base, base, base);
    ledsRechts[effectStep % NUM_LEDS_RECHTS] = CRGB(base, base, base);
    FastLED.show();

    setLogoBrightness(globalBrightness);
    effectStep++;
}

// Gemeinsames Auf-/Abschwellen aller Bereiche im Takt bpm (fuer Pulsieren und
// Atmen, die sich nur in Geschwindigkeit und Mindesthelligkeit unterscheiden).
void applyWave(uint8_t bpm, uint8_t low) {
    if (!frameReady(RENDER_INTERVAL)) return;

    uint8_t wave = beatsin8(bpm, low, 255);
    int level = (globalBrightness * wave) / 255;
    CRGB color = whiteWithBrightness(level);

    fill_solid(ledsLinks,  NUM_LEDS_LINKS,  color);
    fill_solid(ledsRechts, NUM_LEDS_RECHTS, color);
    FastLED.show();

    setLogoBrightness(level);
}

// Weisse Farbe direkt aus einem 8-Bit-Wert (fuer Effekte, die roh in 0-255
// rechnen statt in Prozent).
CRGB whiteRaw(uint8_t v) {
    return CRGB(v, v, v);
}

// Logo direkt mit 8-Bit-Wert ansteuern, ebenfalls auf GLOBAL_MAX_BRIGHTNESS
// begrenzt.
void setLogoRaw(uint8_t level) {
    uint32_t duty = (uint32_t)level * GLOBAL_MAX_BRIGHTNESS / 255;
    ledcWrite(LOGO_PWM_CHANNEL, duty);
}

// true, wenn die aktuelle Uhrzeit in der Nachtabschaltung liegt.
bool isNightOff() {
    struct tm t;
    if (!getCurrentTime(t)) {
        return false;
    }
    int m = t.tm_hour * 60 + t.tm_min;
    return m >= nightStartHour * 60 || m < nightEndHour * 60;
}

// Setzt beide Segmente und das Logo in einem Schritt auf feste 8-Bit-Werte.
void showAll(uint8_t vLinks, uint8_t vRechts, uint8_t vLogo) {
    fill_solid(ledsLinks,  NUM_LEDS_LINKS,  whiteRaw(vLinks));
    fill_solid(ledsRechts, NUM_LEDS_RECHTS, whiteRaw(vRechts));
    FastLED.show();
    setLogoRaw(vLogo);
}

// Dauerlicht: ruhig atmendes Licht auf hohem Niveau, Logo etwas heller.
void renderDauerlicht() {
    if (!frameReady(RENDER_INTERVAL)) return;
    uint8_t v = breathLevel(DAUER_PERIOD_MS, DAUER_MIN, DAUER_MAX);
    showAll(v, v, qadd8(v, 20));
}

// Kerzenlicht: beide Segmente flackern sanft und unabhaengig voneinander.
void renderKerzenlicht() {
    if (!frameReady(RENDER_INTERVAL)) return;
    uint8_t vLinks  = flickerLevel(KERZEN_SPEED_DIV, 0,     KERZEN_MIN, KERZEN_MAX);
    uint8_t vRechts = flickerLevel(KERZEN_SPEED_DIV, 30000, KERZEN_MIN, KERZEN_MAX);
    showAll(vLinks, vRechts, KERZEN_LOGO);
}

// Stufenlicht: die Segmente fuellen sich in acht Schritten von aussen nach
// innen, halten kurz und beginnen dann von vorn.
void renderStufenlicht() {
    if (!frameReady(RENDER_INTERVAL)) return;

    uint32_t cycle = 8UL * STUFEN_STEP_MS + STUFEN_HOLD_MS;
    uint32_t t = millis() % cycle;
    uint8_t lights;
    if (t < 8UL * STUFEN_STEP_MS) {
        lights = t / STUFEN_STEP_MS + 1;
    } else {
        lights = 8;
    }

    uint16_t litLinks  = (uint32_t)NUM_LEDS_LINKS  * lights / 8;
    uint16_t litRechts = (uint32_t)NUM_LEDS_RECHTS * lights / 8;

    for (uint16_t i = 0; i < NUM_LEDS_LINKS; i++) {
        if (i >= NUM_LEDS_LINKS - litLinks) {
            ledsLinks[i] = whiteRaw(STUFEN_LEVEL);
        } else {
            ledsLinks[i] = whiteRaw(0);
        }
    }

    for (uint16_t i = 0; i < NUM_LEDS_RECHTS; i++) {
        if (i < litRechts) {
            ledsRechts[i] = whiteRaw(STUFEN_LEVEL);
        } else {
            ledsRechts[i] = whiteRaw(0);
        }
    }

    FastLED.show();
    setLogoRaw(STUFEN_LEVEL);
}

// Daemmerlicht: sehr gedaempftes Licht, das ueber Minuten sanft schwingt.
void renderDaemmerlicht() {
    if (!frameReady(RENDER_INTERVAL)) return;
    uint8_t v = breathLevel(DAEMMER_PERIOD_MS, DAEMMER_MIN, DAEMMER_MAX);
    showAll(v, v, (uint16_t)v * DAEMMER_LOGO_PCT / 100);
}

// Welle: eine Sinuswelle laeuft raeumlich ueber die Streifen (pixelbasiert).
// Fuer die Fassade wandert sie langsam (grosser Teiler) und die Wellentaeler
// bleiben mit ca. 45 % der Helligkeit erhalten - so entstehen sanfte Hell-Dunkel-
// Baender statt harter dunkler Luecken.
void renderWave() {
    if (!frameReady(RENDER_INTERVAL)) return;

    uint8_t base = brightnessTo8Bit(globalBrightness);
    uint8_t floorLevel = base * 45 / 100;   // untere Grenze der Welle
    uint16_t phase = millis() / 16;

    for (uint16_t i = 0; i < NUM_LEDS_LINKS; i++) {
        uint8_t w = sin8(i * 16 + phase);
        ledsLinks[i] = whiteRaw(map(w, 0, 255, floorLevel, base));
    }
    for (uint16_t i = 0; i < NUM_LEDS_RECHTS; i++) {
        uint8_t w = sin8(i * 16 + phase);
        ledsRechts[i] = whiteRaw(map(w, 0, 255, floorLevel, base));
    }
    FastLED.show();
    setLogoBrightness(globalBrightness);
}

// Feuerschein: kraeftiges, ruhig loderndes Flackern auf hohem Niveau.
void renderFeuerschein() {
    if (!frameReady(RENDER_INTERVAL)) return;
    uint8_t v = flickerLevel(FEUER_SPEED_DIV, 0, FEUER_MIN, FEUER_MAX);
    showAll(v, v, FEUER_LOGO);
}

// Nachtlicht: ruhige, stetig brennende Kerze auf niedrigem Niveau.
void renderNachtlicht() {
    if (!frameReady(RENDER_INTERVAL)) return;
    uint8_t v = flickerLevel(NACHT_SPEED_DIV, 40000, NACHT_MIN, NACHT_MAX);
    showAll(v, v, NACHT_LOGO);
}

// Sternenfunkeln: dezenter Grundglanz, auf dem einzelne Funken kurz aufleuchten
// und langsam verglimmen. Nutzt die Effekt-Helligkeit.
void renderSternenfunkeln() {
    if (!frameReady(RENDER_INTERVAL)) return;

    uint8_t base = brightnessTo8Bit(globalBrightness);
    uint8_t floorLevel = (uint16_t)base * TWINKLE_BASE_PCT / 100;

    fadeToBlackBy(ledsLinks,  NUM_LEDS_LINKS,  TWINKLE_FADE);
    fadeToBlackBy(ledsRechts, NUM_LEDS_RECHTS, TWINKLE_FADE);

    // Grundglanz halten (nichts faellt unter das Grundniveau).
    for (uint16_t i = 0; i < NUM_LEDS_LINKS; i++) {
        if (ledsLinks[i].r < floorLevel) ledsLinks[i] = whiteRaw(floorLevel);
    }
    for (uint16_t i = 0; i < NUM_LEDS_RECHTS; i++) {
        if (ledsRechts[i].r < floorLevel) ledsRechts[i] = whiteRaw(floorLevel);
    }

    // Gelegentlich einen neuen Funken auf voller Effekt-Helligkeit setzen.
    if (random8() < TWINKLE_CHANCE) ledsLinks[random16(NUM_LEDS_LINKS)]   = whiteRaw(base);
    if (random8() < TWINKLE_CHANCE) ledsRechts[random16(NUM_LEDS_RECHTS)] = whiteRaw(base);

    FastLED.show();
    setLogoBrightness(globalBrightness * TWINKLE_BASE_PCT / 100);
}

// Treffpunkt: je ein Lichtschweif laeuft von aussen nach innen; beide treffen
// sich in der Mitte (beim Logo/Eingang) und beginnen dann von vorn.
void renderTreffpunkt() {
    if (!frameReady(EFFECT_INTERVAL)) return;

    uint8_t base = brightnessTo8Bit(globalBrightness);

    fadeToBlackBy(ledsLinks,  NUM_LEDS_LINKS,  55);
    fadeToBlackBy(ledsRechts, NUM_LEDS_RECHTS, 55);

    ledsLinks[effectStep % NUM_LEDS_LINKS] = CRGB(base, base, base);
    uint16_t posR = (NUM_LEDS_RECHTS - 1) - (effectStep % NUM_LEDS_RECHTS);
    ledsRechts[posR] = CRGB(base, base, base);

    FastLED.show();
    setLogoBrightness(globalBrightness);
    effectStep++;
}

// Kurze, dreieckige Helligkeitsspitze um 'center' mit Breite 'width' (fuer den
// Herzschlag). Ergibt am Zentrum 'peak', an den Raendern 0.
uint8_t heartBump(uint32_t t, uint32_t center, uint32_t width, uint8_t peak) {
    uint32_t d = (t > center) ? (t - center) : (center - t);
    if (d >= width) return 0;
    return peak - (uint32_t)peak * d / width;
}

// Herzschlag: zwei kurze Schlaege je Zyklus, dazwischen ruhiges Grundniveau.
void renderHerzschlag() {
    if (!frameReady(RENDER_INTERVAL)) return;

    uint8_t base = brightnessTo8Bit(globalBrightness);
    uint8_t low  = (uint16_t)base * HEART_LOW_PCT / 100;
    uint32_t t   = millis() % HEART_PERIOD_MS;

    int level = low;
    int b1 = heartBump(t, 120, 170, base);          // erster Schlag
    int b2 = heartBump(t, 430, 210, base * 4 / 5);  // zweiter, etwas schwaecher
    if (b1 > level) level = b1;
    if (b2 > level) level = b2;

    showAll(level, level, level);
}

// Wechsellicht: Segmente Links/Rechts schwellen langsam gegenlaeufig - waehrend
// die eine Seite heller wird, dimmt die andere ab.
void renderWechsellicht() {
    if (!frameReady(RENDER_INTERVAL)) return;

    uint8_t base = brightnessTo8Bit(globalBrightness);
    uint8_t low  = (uint16_t)base * WECHSEL_LOW_PCT / 100;

    uint8_t phase   = sin8(millis() / (WECHSEL_PERIOD_MS / 256));
    uint8_t vLinks  = map(phase, 0, 255, low, base);
    uint8_t vRechts = map(phase, 0, 255, base, low);

    showAll(vLinks, vRechts, (vLinks + vRechts) / 2);
}

// Modus ist animiert (alles ausser Aus, Statisch, Automatik) -> Frame pro loop.
bool isEffectMode(OperatingMode m) {
    return m != MODE_OFF && m != MODE_STATIC && m != MODE_AUTOMATIC;
}

// Stimmungs-Modi mit festen Werten, die der Nachtabschaltung unterliegen.
bool isThematic(OperatingMode m) {
    return m == MODE_DAUERLICHT || m == MODE_KERZENLICHT ||
           m == MODE_STUFENLICHT || m == MODE_DAEMMERLICHT ||
           m == MODE_FEUERSCHEIN || m == MODE_NACHTLICHT;
}

// --- Modus-Dispatch und Override -------------------------------------------

// Fuehrt den zum aktuellen Modus passenden Ausgabeschritt aus. Stimmungs-Modi
// werden in der Nachtabschaltung hart ausgeschaltet.
void applyHardware() {
    if (isThematic(currentMode) && isNightOff()) {
        showAll(0, 0, 0);
        return;
    }

    switch (currentMode) {
        case MODE_OFF:          allLEDsOff();         break;
        case MODE_STATIC:       applyStatic();        break;
        case MODE_EFFECT:       applyEffect();        break;
        case MODE_PULSE:        applyWave(20, 120);   break;
        case MODE_BREATH:       applyWave(6, 100);    break;
        case MODE_AUTOMATIC:    applyAutomatic();     break;
        case MODE_WAVE:         renderWave();         break;
        case MODE_DAUERLICHT:   renderDauerlicht();   break;
        case MODE_KERZENLICHT:  renderKerzenlicht();  break;
        case MODE_STUFENLICHT:  renderStufenlicht();  break;
        case MODE_DAEMMERLICHT: renderDaemmerlicht(); break;
        case MODE_FEUERSCHEIN:  renderFeuerschein();  break;
        case MODE_NACHTLICHT:   renderNachtlicht();   break;
        case MODE_STERNEN:      renderSternenfunkeln(); break;
        case MODE_TREFFPUNKT:   renderTreffpunkt();   break;
        case MODE_HERZSCHLAG:   renderHerzschlag();   break;
        case MODE_WECHSEL:      renderWechsellicht(); break;
    }
}

// Ein manueller Helligkeitseingriff waehrend der Automatik schaltet auf
// Statisch um und merkt sich das Zeitfenster als Rueckkehrpunkt.
void enterOverrideIfAutomatic() {
    if (currentMode == MODE_AUTOMATIC) {
        currentMode = MODE_STATIC;
        overrideActive = true;
        overrideWindow = currentAutoWindow();
    }
}

// Kehrt nach einem Override selbsttaetig in die Automatik zurueck, sobald das
// naechste Zeitfenster beginnt (Pflichtenheft Kap. 7.4, Variante a).
void updateOverrideReturn() {
    if (!overrideActive) {
        return;
    }
    if (currentMode == MODE_AUTOMATIC) {
        overrideActive = false;
        return;
    }

    int window = currentAutoWindow();
    if (window >= 0 && overrideWindow >= 0 && window != overrideWindow) {
        currentMode = MODE_AUTOMATIC;
        overrideActive = false;
        applyAutomatic();
        broadcastStatus();
        Serial.println("Automatik-Rueckkehr.");
    }
}

// --- App-Schnittstelle (WebSocket + REST) ----------------------------------

// Baut den kompletten Systemzustand als JSON: Modus, Helligkeiten, Zeit/Datum,
// RTC-/NTP-/WLAN-Status und Zeitprofil. Dient App-Status und -Push.
String createStatusJson() {
    JsonDocument doc;
    doc["mode"]  = (int)currentMode;
    doc["left"]  = brightnessLeft;
    doc["right"] = brightnessRight;
    doc["logo"]  = brightnessLogo;
    doc["global"] = globalBrightness;
    doc["autoBrightness"] = currentAutoBrightness;
    doc["rtc"] = rtcAvailable && isRTCValid();
    doc["ntp"] = ntpSynced;
    doc["time"] = getTimeString();
    doc["date"] = getDateString();

    if (apMode) {
        doc["ip"] = WiFi.softAPIP().toString();
    } else {
        doc["ip"] = WiFi.localIP().toString();
    }

    doc["rssi"] = WiFi.RSSI();
    doc["wifi"] = WiFi.status() == WL_CONNECTED;
    doc["ap"]   = apMode;
    doc["ssid"] = WIFI_SSID;
    doc["firmware"] = FIRMWARE_VERSION;

    JsonObject sched = doc["sched"].to<JsonObject>();
    sched["tMorning"]  = morningStartHour;
    sched["tDay"]      = dayStartHour;
    sched["tEvening"]  = eveningStartHour;
    sched["tNight"]    = nightStartHour;
    sched["bMorning"]  = morningBrightness;
    sched["bDay"]      = dayBrightness;
    sched["bEveStart"] = eveningStartBrightness;
    sched["bEveEnd"]   = eveningEndBrightness;

    String out;
    serializeJson(doc, out);
    return out;
}

// Sendet den aktuellen Status an alle verbundenen App-Clients.
void broadcastStatus() {
    ws.textAll(createStatusJson());
}

// Verarbeitet eingehende WebSocket-Nachrichten der App. Jedes bekannte JSON-Feld
// setzt einen Zustand (Modus, Helligkeit). Danach wird die
// Aenderung angewandt, zum Speichern vorgemerkt und an alle Clients gepusht.
void onWebSocketEvent(AsyncWebSocket *serverPtr, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {

    if (type == WS_EVT_CONNECT) {
        client->text(createStatusJson());
        return;
    }
    if (type != WS_EVT_DATA) {
        return;
    }

    // Leere oder zu grosse Nachrichten ignorieren.
    if (len == 0 || len > MAX_WS_MESSAGE_LEN) {
        return;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, data, len);
    if (err) {
        Serial.println("JSON Fehler");
        return;
    }

    bool changed = false;

    if (doc["mode"].is<int>()) {
        int mode = doc["mode"].as<int>();
        if (mode >= MODE_OFF && mode <= MODE_LAST) {
            currentMode = (OperatingMode)mode;
            if (currentMode == MODE_AUTOMATIC) {
                overrideActive = false;
                applyAutomatic();
            } else {
                overrideActive = true;
                overrideWindow = currentAutoWindow();
            }
            changed = true;
        }
    }

    if (doc["left"].is<int>()) {
        brightnessLeft = clampBrightness(doc["left"].as<int>());
        enterOverrideIfAutomatic();
        changed = true;
    }
    if (doc["right"].is<int>()) {
        brightnessRight = clampBrightness(doc["right"].as<int>());
        enterOverrideIfAutomatic();
        changed = true;
    }
    if (doc["logo"].is<int>()) {
        brightnessLogo = clampBrightness(doc["logo"].as<int>());
        enterOverrideIfAutomatic();
        changed = true;
    }
    if (doc["global"].is<int>()) {
        globalBrightness = clampBrightness(doc["global"].as<int>());
        changed = true;
    }

    // Das komplette Automatik-Zeitprofil (Uhrzeiten tMorning/tDay/tEvening/
    // tNight und Helligkeiten bMorning/bDay/bEveStart/bEveEnd) ist fest im Code
    // (config.h) und wird bewusst NICHT ueber die App entgegengenommen.

    if (changed) {
        markSettingsDirty();
        applyHardware();
        broadcastStatus();
    }
}

// --- Netzwerk und Zeitsynchronisation --------------------------------------

// Fallback, wenn das konfigurierte WLAN nicht erreichbar ist: eigener
// Accesspoint, damit die App weiterhin lokal erreichbar bleibt.
void startAccessPoint() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(SETUP_AP_SSID, SETUP_AP_PASS);
    apMode = true;
    wifiConnected = false;
    Serial.printf("Setup-AP \"%s\" -> http://%s/\n",
                  SETUP_AP_SSID, WiFi.softAPIP().toString().c_str());
}

// Verbindet mit dem in config.h hinterlegten WLAN; nach Timeout Setup-AP.
void connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(HOSTNAME);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.printf("WLAN verbinde mit \"%s\" ", WIFI_SSID);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 12000) {
        delay(250);
        Serial.print(".");
    }
    Serial.println();

    wifiConnected = (WiFi.status() == WL_CONNECTED);
    if (wifiConnected) {
        apMode = false;
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("WLAN fehlgeschlagen -> Setup-AP");
        startAccessPoint();
    }
}

// Einmalige NTP-Synchronisation nach dem Verbinden. Bei Erfolg wird die RTC
// nachgestellt (Zeitzone/Sommerzeit ueber TZ_INFO).
void setupNTP() {
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }
    configTzTime(TZ_INFO, NTP_SERVER_1, NTP_SERVER_2);

    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 5000)) {
        ntpSynced = true;

        if (rtcAvailable) {
            time_t now;
            time(&now);
            struct tm lt;
            localtime_r(&now, &lt);
            rtc.adjust(DateTime(lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday,
                                lt.tm_hour, lt.tm_min, lt.tm_sec));
        }
    }
}

// Periodischer NTP-Abgleich im Betrieb, haelt die RTC langfristig genau.
void updateNTP() {
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }
    if (millis() - lastNtpCheck < NTP_CHECK_INTERVAL) {
        return;
    }
    lastNtpCheck = millis();

    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 1000)) {
        ntpSynced = true;

        if (rtcAvailable) {
            time_t now;
            time(&now);
            struct tm lt;
            localtime_r(&now, &lt);
            rtc.adjust(DateTime(lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday,
                                lt.tm_hour, lt.tm_min, lt.tm_sec));
        }
    }
}

// Initialisiert die DS3231 am I2C-Bus und meldet Verfuegbarkeit/Stromverlust.
void setupRTC() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    if (rtc.begin()) {
        rtcAvailable = true;
        Serial.println("DS3231 gefunden.");
        if (rtc.lostPower()) {
            Serial.println("DS3231: Stromverlust.");
        }
        if (isRTCValid()) {
            Serial.print("RTC: ");
            Serial.println(getTimeString());
        }
    } else {
        rtcAvailable = false;
        Serial.println("DS3231 nicht gefunden.");
    }
}

// --- Hardware- und Server-Setup --------------------------------------------

// Optischer Selbsttest beim Start: hilft, Verdrahtung/Pegelwandler zu pruefen.
void ledSelfTest() {
#if ENABLE_SELFTEST
    Serial.println("LED-Selbsttest (Rot/Gruen/Blau/Weiss). Nichts an? -> 5V, GND, DIN, Pegelwandler pruefen.");
    CRGB colors[4] = { CRGB::Red, CRGB::Green, CRGB::Blue, CRGB::White };

    for (int r = 0; r < SELFTEST_ROUNDS; r++) {
        for (int i = 0; i < 4; i++) {
            fill_solid(ledsLinks,  NUM_LEDS_LINKS,  colors[i]);
            fill_solid(ledsRechts, NUM_LEDS_RECHTS, colors[i]);
            FastLED.show();
            delay(400);
        }
    }

    fill_solid(ledsLinks,  NUM_LEDS_LINKS,  CRGB::Black);
    fill_solid(ledsRechts, NUM_LEDS_RECHTS, CRGB::Black);
    FastLED.show();
#endif
}

// Richtet die FastLED-Streifen und den Logo-PWM-Kanal ein. Die Strombegrenzung
// (setMaxPowerInVoltsAndMilliamps) schuetzt vor Netzteilueberlast (Kap. A6).
void setupLEDs() {
    FastLED.addLeds<LED_TYPE, PIN_LED_LINKS,  LED_COLOR_ORDER>(ledsLinks,  NUM_LEDS_LINKS);
    FastLED.addLeds<LED_TYPE, PIN_LED_RECHTS, LED_COLOR_ORDER>(ledsRechts, NUM_LEDS_RECHTS);
    FastLED.setBrightness(GLOBAL_MAX_BRIGHTNESS);
    FastLED.setMaxPowerInVoltsAndMilliamps(LED_VOLTS, LED_MAX_MILLIAMPS);

    ledSelfTest();

    fill_solid(ledsLinks,  NUM_LEDS_LINKS,  CRGB::Black);
    fill_solid(ledsRechts, NUM_LEDS_RECHTS, CRGB::Black);
    FastLED.show();

    pinMode(PIN_LOGO_PWM, OUTPUT);
    ledcSetup(LOGO_PWM_CHANNEL, LOGO_PWM_FREQ, LOGO_PWM_RES);
    ledcAttachPin(PIN_LOGO_PWM, LOGO_PWM_CHANNEL);
    ledcWrite(LOGO_PWM_CHANNEL, 0);
}

// --- Handler fuer die einzelnen Webserver-Adressen -------------------------
// Jede Adresse (URL) bekommt eine eigene, benannte Funktion. Das ist leichter
// zu lesen als anonyme Funktionen direkt bei der Registrierung.

// Liefert die Bedien-App (HTML-Seite).
void handleRoot(AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
}

// Liefert die PWA-Manifestdatei.
void handleManifest(AsyncWebServerRequest *request) {
    request->send(200, "application/manifest+json", manifest_json);
}

// Liefert das SVG-Icon (fuer Browser-Tab und Desktop).
void handleIconSvg(AsyncWebServerRequest *request) {
    request->send(200, "image/svg+xml", icon_svg);
}

// PNG-Home-Screen-Icons: iOS nutzt apple-touch-icon, Android 192/512.
void handleIcon180(AsyncWebServerRequest *request) {
    request->send(200, "image/png", icon_180_png, icon_180_png_len);
}

void handleIcon192(AsyncWebServerRequest *request) {
    request->send(200, "image/png", icon_192_png, icon_192_png_len);
}

void handleIcon512(AsyncWebServerRequest *request) {
    request->send(200, "image/png", icon_512_png, icon_512_png_len);
}

// Liefert den aktuellen Systemzustand als JSON.
void handleApiStatus(AsyncWebServerRequest *request) {
    request->send(200, "application/json", createStatusJson());
}

// Liefert das Automatik-Zeitprofil als JSON.
void handleApiSchedule(AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["tMorning"]  = morningStartHour;
    doc["tDay"]      = dayStartHour;
    doc["tEvening"]  = eveningStartHour;
    doc["tNight"]    = nightStartHour;
    doc["bMorning"]  = morningBrightness;
    doc["bDay"]      = dayBrightness;
    doc["bEveStart"] = eveningStartBrightness;
    doc["bEveEnd"]   = eveningEndBrightness;
    doc["autoBrightness"] = currentAutoBrightness;
    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
}

// OTA-Update: wird aufgerufen, wenn der Upload fertig ist (Pflichtenheft F8).
void handleUpdateDone(AsyncWebServerRequest *request) {
    bool ok = !Update.hasError();

    String msg;
    if (ok) {
        msg = "Update erfolgreich. Neustart...";
    } else {
        msg = "Update fehlgeschlagen.";
    }

    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", msg);
    response->addHeader("Connection", "close");
    request->send(response);

    if (ok) {
        if (settingsDirty) {
            saveSettings();
        }
        delay(500);
        ESP.restart();
    }
}

// OTA-Update: wird waehrend des Uploads Stueck fuer Stueck aufgerufen und
// schreibt die Daten ins Flash.
void handleUpdateUpload(AsyncWebServerRequest *request, String filename,
                        size_t index, uint8_t *data, size_t len, bool final) {
    if (index == 0) {
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            Update.printError(Serial);
        }
    }

    if (Update.write(data, len) != len) {
        Update.printError(Serial);
    }

    if (final) {
        if (Update.end(true)) {
            Serial.println("OTA fertig.");
        } else {
            Update.printError(Serial);
        }
    }
}

// Registriert alle Adressen mit ihren Handler-Funktionen und startet den
// Webserver samt mDNS (erreichbar unter HOSTNAME.local).
void setupWebServer() {
    ws.onEvent(onWebSocketEvent);
    server.addHandler(&ws);

    server.on("/", HTTP_GET, handleRoot);
    server.on("/manifest.json", HTTP_GET, handleManifest);
    server.on("/icon.svg", HTTP_GET, handleIconSvg);
    server.on("/apple-touch-icon.png", HTTP_GET, handleIcon180);
    server.on("/icon-192.png", HTTP_GET, handleIcon192);
    server.on("/icon-512.png", HTTP_GET, handleIcon512);
    server.on("/api/status", HTTP_GET, handleApiStatus);
    server.on("/api/schedule", HTTP_GET, handleApiSchedule);
    server.on("/update", HTTP_POST, handleUpdateDone, handleUpdateUpload);

    server.begin();

    if (MDNS.begin(HOSTNAME)) {
        MDNS.addService("http", "tcp", 80);
        Serial.printf("mDNS: http://%s.local/\n", HOSTNAME);
    }
    Serial.println("Webserver gestartet.");
}

// --- Einstiegspunkte -------------------------------------------------------

// Startet in einem definierten Power-On-State: Einstellungen und Zeit laden,
// dann immer im Automatik-Modus hochfahren (Pflichtenheft Kap. 8.2). So kehrt
// das System nach einem Stromausfall selbsttaetig in einen sicheren Zustand
// zurueck, unabhaengig vom vorher aktiven Modus.
void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.printf("\n=== Fassadenbeleuchtung ESP32 - Firmware %s ===\n", FIRMWARE_VERSION);

    loadSettings();

    setupRTC();
    setupLEDs();
    currentMode = MODE_AUTOMATIC;
    applyAutomatic();

    connectWiFi();
    setupNTP();
    setupWebServer();

    broadcastStatus();
    Serial.println("System bereit.");
}

// Hauptschleife: NTP-Nachfuehrung, Automatik periodisch neu berechnen, je nach
// Modus animiert rendern oder den weichen Solid-Fade fahren, Override-Rueckkehr
// pruefen, Einstellungen entprellt speichern und den Status regelmaessig pushen.
void loop() {
    ws.cleanupClients();
    updateNTP();

    if (currentMode == MODE_AUTOMATIC && millis() - lastAutoUpdate >= AUTO_UPDATE_INTERVAL) {
        lastAutoUpdate = millis();
        applyAutomatic();
    }

    bool nowEffect = isEffectMode(currentMode);

    // Beim Wechsel von einem Effekt zurueck zu einem Solid-Modus die
    // angezeigten Werte zuruecksetzen, damit der Fade sauber neu beginnt.
    if (prevFrameEffect && !nowEffect) {
        shownLeft  = 0;
        shownRight = 0;
        shownLogo  = 0;
    }
    prevFrameEffect = nowEffect;

    if (nowEffect) {
        applyHardware();
    } else {
        renderSolid();
    }

    updateOverrideReturn();

    if (settingsDirty && millis() - lastSettingsChange >= SAVE_DEBOUNCE_MS) {
        saveSettings();
    }

    if (millis() - lastStatusBroadcast >= STATUS_INTERVAL) {
        lastStatusBroadcast = millis();
        broadcastStatus();
    }

    delay(5);
}
