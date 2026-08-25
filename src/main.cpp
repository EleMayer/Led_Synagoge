// ---------------------------------------------------------------------------
// LED-Fassadenbeleuchtung - ESP32-Firmware
//
// Ziel: Arduino-ESP32 Core 2.x (espressif32@7.0.1, gepinnt in platformio.ini)
//
// Steuert:
//   - LED-Segment Links/Rechts über WS2812 + FastLED
//   - Logo über PWM/MOSFET
//   - Web-App über HTTP + WebSocket
//   - DS3231 RTC, optional NTP-Synchronisation
//   - NVS-Speicherung der manuellen Helligkeiten
//   - OTA-Update
//
// WICHTIG:
//   - Die Modus-IDs bleiben kompatibel mit config.h / web_page.h.
//   - Automatik-Zeiten und Automatik-Helligkeiten kommen aus config.h.
//   - Nur manuelle Helligkeitsänderungen während AUTOMATIC erzeugen einen
//     zeitlich begrenzten Override.
//   - Bewusst gewählte Effekt-/Stimmungsmodi werden NICHT beim nächsten
//     Automatik-Zeitfenster ungefragt verlassen.
//   - PWM ist für die Arduino-ESP32 Core 2.x LEDC-API umgesetzt.
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

// ---------------------------------------------------------------------------
// Globale Zustände
// ---------------------------------------------------------------------------

OperatingMode currentMode = MODE_AUTOMATIC;

// Manueller Helligkeits-Override der Automatik.
bool overrideActive = false;
int overrideWindow = -1;

// Manuelle Helligkeiten 0-100 %.
int brightnessLeft  = 80;
int brightnessRight = 80;
int brightnessLogo  = 80;

// Effekt-Helligkeit ist fest in config.h.
const int globalBrightness = EFFECT_BRIGHTNESS;

// Automatik-Zeitfenster aus config.h.
const int morningStartHour = AUTO_T_MORNING;
const int morningEndHour   = AUTO_T_DAY;
const int dayStartHour     = AUTO_T_DAY;
const int dayEndHour       = AUTO_T_EVENING;
const int eveningStartHour = AUTO_T_EVENING;
const int eveningEndHour   = AUTO_T_NIGHT;
const int nightStartHour   = AUTO_T_NIGHT;
const int nightEndHour     = AUTO_T_MORNING;

// Automatik-Helligkeiten aus config.h.
const int morningBrightness      = AUTO_B_MORNING;
const int dayBrightness          = AUTO_B_DAY;
const int eveningStartBrightness = AUTO_B_EVE_START;
const int eveningEndBrightness   = AUTO_B_EVE_END;

int currentAutoBrightness = 0;

// ---------------------------------------------------------------------------
// Fade-Engine
// ---------------------------------------------------------------------------

int targetLeft  = 0;
int targetRight = 0;
int targetLogo  = 0;

float shownLeft  = 0;
float shownRight = 0;
float shownLogo  = 0;

const float FADE_STEP = 2.0f;

unsigned long lastRender = 0;
const uint32_t RENDER_INTERVAL = 20;

bool prevFrameEffect = false;

// ---------------------------------------------------------------------------
// Timing
// ---------------------------------------------------------------------------

uint16_t effectStep = 0;
unsigned long lastEffectUpdate = 0;
const uint32_t EFFECT_INTERVAL = 80;

unsigned long lastAutoUpdate = 0;
const uint32_t AUTO_UPDATE_INTERVAL = 1000;

unsigned long lastStatusBroadcast = 0;
const uint32_t STATUS_INTERVAL = 2000;

unsigned long lastNtpCheck = 0;
const uint32_t NTP_CHECK_INTERVAL = 60000;

unsigned long lastWifiReconnect = 0;
const uint32_t WIFI_RECONNECT_INTERVAL = 30000;

// ---------------------------------------------------------------------------
// Hardware / Netzwerk
// ---------------------------------------------------------------------------

#define MAX_WS_MESSAGE_LEN 512

RTC_DS3231 rtc;
Preferences preferences;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

CRGB ledsLinks[NUM_LEDS_LINKS];
CRGB ledsRechts[NUM_LEDS_RECHTS];

bool rtcAvailable = false;
bool wifiConnected = false;
bool ntpSynced = false;
bool apMode = false;

const char *NTP_SERVER_1 = "pool.ntp.org";
const char *NTP_SERVER_2 = "time.nist.gov";

// Wien / Österreich:
const char *TZ_INFO = "CET-1CEST,M3.5.0,M10.5.0";

// ---------------------------------------------------------------------------
// Vorwärtsdeklarationen
// ---------------------------------------------------------------------------

void broadcastStatus();
void applyHardware();
void applyAutomatic();

void setupRTC();
void setupLEDs();
void setupWebServer();

void connectWiFi();
void startAccessPoint();
void updateWiFi();

void setupNTP();
void updateNTP();

void saveSettings();
void loadSettings();

void updateOverrideReturn();

// ---------------------------------------------------------------------------
// Helligkeit
// ---------------------------------------------------------------------------

int clampBrightness(int value) {
    return constrain(value, 0, 100);
}

uint8_t brightnessTo8Bit(int brightness) {
    int safe = clampBrightness(brightness);
    return (uint8_t)map(safe, 0, 100, 0, 255);
}

CRGB whiteWithBrightness(int brightness) {
    uint8_t v = brightnessTo8Bit(brightness);
    return CRGB(v, v, v);
}

CRGB whiteRaw(uint8_t v) {
    return CRGB(v, v, v);
}

// ---------------------------------------------------------------------------
// Logo-PWM
//
// Arduino-ESP32 Core 2.x (LEDC-API):
//   ledcSetup(channel, frequency, resolution)
//   ledcAttachPin(pin, channel)
//   ledcWrite(channel, duty)
// ---------------------------------------------------------------------------

void setupLogoPWM() {
    pinMode(PIN_LOGO_PWM, OUTPUT);

    // Rueckgabe ist die tatsaechliche Frequenz, 0 bei Fehler.
    double freq = ledcSetup(
        LOGO_PWM_CHANNEL,
        LOGO_PWM_FREQ,
        LOGO_PWM_RES
    );

    if (freq == 0) {
        Serial.println("FEHLER: Logo-PWM konnte nicht eingerichtet werden.");
        return;
    }

    ledcAttachPin(PIN_LOGO_PWM, LOGO_PWM_CHANNEL);

    ledcWrite(LOGO_PWM_CHANNEL, 0);

    Serial.println("Logo-PWM eingerichtet.");
}

void setLogoBrightness(int brightness) {
    brightness = clampBrightness(brightness);

    uint32_t duty = brightnessTo8Bit(brightness);

    duty = duty * GLOBAL_MAX_BRIGHTNESS / 255;

    ledcWrite(LOGO_PWM_CHANNEL, duty);
}

void setLogoRaw(uint8_t level) {
    uint32_t duty =
        (uint32_t)level * GLOBAL_MAX_BRIGHTNESS / 255;

    ledcWrite(LOGO_PWM_CHANNEL, duty);
}

// ---------------------------------------------------------------------------
// NVS
// ---------------------------------------------------------------------------

void saveSettings() {
    preferences.begin("facelight", false);

    preferences.putInt("left", brightnessLeft);
    preferences.putInt("right", brightnessRight);
    preferences.putInt("logo", brightnessLogo);

    preferences.end();

    Serial.println("Einstellungen gespeichert.");
}

bool settingsDirty = false;
unsigned long lastSettingsChange = 0;
const uint32_t SAVE_DEBOUNCE_MS = 1500;

// Der WebSocket-Handler laeuft im AsyncTCP-Task, nicht in loop(). Er darf daher
// NICHT selbst FastLED.show()/applyHardware() aufrufen (WS2812-Timing mit
// deaktivierten Interrupts aus zwei Tasks -> Flackern/Absturz). Er setzt nur
// dieses Flag; loop() sendet daraufhin sofort den aktuellen Status.
volatile bool statusUpdateRequested = false;

void markSettingsDirtyReal() {
    settingsDirty = true;
    lastSettingsChange = millis();
}

void loadSettings() {
    preferences.begin("facelight", true);

    brightnessLeft =
        preferences.getInt("left", 80);

    brightnessRight =
        preferences.getInt("right", 80);

    brightnessLogo =
        preferences.getInt("logo", 80);

    preferences.end();

    brightnessLeft = clampBrightness(brightnessLeft);
    brightnessRight = clampBrightness(brightnessRight);
    brightnessLogo = clampBrightness(brightnessLogo);

    Serial.printf(
        "Einstellungen: Links=%d%% Rechts=%d%% Logo=%d%%\n",
        brightnessLeft,
        brightnessRight,
        brightnessLogo
    );
}

// ---------------------------------------------------------------------------
// RTC / Zeit
// ---------------------------------------------------------------------------

bool isRTCValid() {
    if (!rtcAvailable) {
        return false;
    }

    DateTime now = rtc.now();

    return now.year() >= 2024 &&
           now.year() <= 2099;
}

bool getCurrentTime(struct tm &out) {
    memset(&out, 0, sizeof(out));

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

    snprintf(
        buf,
        sizeof(buf),
        "%02d:%02d:%02d",
        t.tm_hour,
        t.tm_min,
        t.tm_sec
    );

    return String(buf);
}

String getDateString() {
    struct tm t;

    if (!getCurrentTime(t)) {
        return "--.--.----";
    }

    char buf[16];

    snprintf(
        buf,
        sizeof(buf),
        "%02d.%02d.%04d",
        t.tm_mday,
        t.tm_mon + 1,
        t.tm_year + 1900
    );

    return String(buf);
}

// ---------------------------------------------------------------------------
// Automatik
// ---------------------------------------------------------------------------

int currentAutoWindow() {
    struct tm t;

    if (!getCurrentTime(t)) {
        return -1;
    }

    int minutes = t.tm_hour * 60 + t.tm_min;

    // Nacht
    if (
        minutes >= nightStartHour * 60 ||
        minutes < nightEndHour * 60
    ) {
        return 0;
    }

    // Morgen
    if (
        minutes >= morningStartHour * 60 &&
        minutes < morningEndHour * 60
    ) {
        return 1;
    }

    // Tag
    if (
        minutes >= dayStartHour * 60 &&
        minutes < dayEndHour * 60
    ) {
        return 2;
    }

    // Abend
    if (
        minutes >= eveningStartHour * 60 &&
        minutes < eveningEndHour * 60
    ) {
        return 3;
    }

    return 0;
}

int calculateAutomaticBrightness() {
    struct tm t;

    if (!getCurrentTime(t)) {
        return SAFE_DEFAULT_BRIGHTNESS;
    }

    float minutes =
        t.tm_hour * 60.0f +
        t.tm_min +
        t.tm_sec / 60.0f;

    // Nacht
    if (
        minutes >= nightStartHour * 60 ||
        minutes < nightEndHour * 60
    ) {
        return 0;
    }

    // Morgenrampe
    if (
        minutes >= morningStartHour * 60 &&
        minutes < morningEndHour * 60
    ) {
        float start = morningStartHour * 60.0f;
        float end   = morningEndHour * 60.0f;

        float p = (minutes - start) / (end - start);

        p = constrain(p, 0.0f, 1.0f);

        return round(
            p * morningBrightness
        );
    }

    // Tag
    if (
        minutes >= dayStartHour * 60 &&
        minutes < dayEndHour * 60
    ) {
        return dayBrightness;
    }

    // Abendrampe
    if (
        minutes >= eveningStartHour * 60 &&
        minutes < eveningEndHour * 60
    ) {
        float start = eveningStartHour * 60.0f;
        float end   = eveningEndHour * 60.0f;

        float p = (minutes - start) / (end - start);

        p = constrain(p, 0.0f, 1.0f);

        return round(
            eveningStartBrightness +
            (
                eveningEndBrightness -
                eveningStartBrightness
            ) * p
        );
    }

    return 0;
}

// ---------------------------------------------------------------------------
// Optionaler Daemmerungssensor
//
// Nur aktiv bei USE_LIGHT_SENSOR == 1. Liefert die gemessene Umgebungshelligkeit
// als 0..100 % (0 = dunkel, 100 = hell) und senkt damit die Automatik bei
// Tageslicht ab. Ist der Sensor aus, bleibt der Automatikwert unveraendert.
// ---------------------------------------------------------------------------

#if USE_LIGHT_SENSOR

int readLightPercent() {
    // Nicht jeden Frame lesen - ein zwischengespeicherter Wert reicht.
    static unsigned long lastRead = 0;
    static int cached = 100;

    if (millis() - lastRead < 500) {
        return cached;
    }

    lastRead = millis();

    // Mehrere Messungen mitteln (ruhigerer Wert).
    uint32_t sum = 0;

    for (int i = 0; i < 8; i++) {
        sum += analogRead(LIGHT_SENSOR_PIN);
    }

    int raw = sum / 8;

    int pct = map(
        raw,
        LIGHT_ADC_DARK,
        LIGHT_ADC_BRIGHT,
        0,
        100
    );

    cached = constrain(pct, 0, 100);

    return cached;
}

#endif

int applyLightSensorToAuto(int timeBrightness) {
#if USE_LIGHT_SENSOR
    int lightPct = readLightPercent();   // 0 = dunkel ... 100 = hell
    int darkness = 100 - lightPct;

    // Zeitkurve ist die Obergrenze, die Dunkelheit blendet ein.
    return timeBrightness * darkness / 100;
#else
    // Sensor aus: Automatik bleibt rein zeitgesteuert.
    return timeBrightness;
#endif
}

void applyAutomatic() {
    int value = calculateAutomaticBrightness();

    value = applyLightSensorToAuto(value);

    currentAutoBrightness = value;

    targetLeft = value;
    targetRight = value;
    targetLogo = value;
}

// ---------------------------------------------------------------------------
// Fade
// ---------------------------------------------------------------------------

float approach(float current, int target) {
    if (current < target) {
        current += FADE_STEP;

        if (current > target) {
            current = target;
        }
    }
    else if (current > target) {
        current -= FADE_STEP;

        if (current < target) {
            current = target;
        }
    }

    return current;
}

void renderSolid() {
    if (millis() - lastRender < RENDER_INTERVAL) {
        return;
    }

    lastRender = millis();

    shownLeft =
        approach(shownLeft, targetLeft);

    shownRight =
        approach(shownRight, targetRight);

    shownLogo =
        approach(shownLogo, targetLogo);

    fill_solid(
        ledsLinks,
        NUM_LEDS_LINKS,
        whiteWithBrightness(round(shownLeft))
    );

    fill_solid(
        ledsRechts,
        NUM_LEDS_RECHTS,
        whiteWithBrightness(round(shownRight))
    );

    FastLED.show();

    setLogoBrightness(round(shownLogo));
}

void allLEDsOff() {
    targetLeft = 0;
    targetRight = 0;
    targetLogo = 0;
}

void applyStatic() {
    targetLeft = brightnessLeft;
    targetRight = brightnessRight;
    targetLogo = brightnessLogo;
}

// ---------------------------------------------------------------------------
// Effekte
// ---------------------------------------------------------------------------

bool frameReady(uint32_t interval) {
    if (millis() - lastEffectUpdate < interval) {
        return false;
    }

    lastEffectUpdate = millis();

    return true;
}

uint8_t breathLevel(
    uint32_t periodMs,
    uint8_t lo,
    uint8_t hi
) {
    uint32_t divisor = periodMs / 256UL;

    if (divisor == 0) {
        divisor = 1;
    }

    uint8_t s =
        sin8(millis() / divisor);

    return map(
        s,
        0,
        255,
        lo,
        hi
    );
}

uint8_t flickerLevel(
    uint16_t speedDiv,
    uint16_t seed,
    uint8_t lo,
    uint8_t hi
) {
    if (speedDiv == 0) {
        speedDiv = 1;
    }

    uint8_t n =
        inoise8(
            millis() / speedDiv,
            seed
        );

    return map(
        n,
        0,
        255,
        lo,
        hi
    );
}

// ---------------------------------------------------------------------------
// Lauflicht
// ---------------------------------------------------------------------------

void applyEffect() {
    if (!frameReady(EFFECT_INTERVAL)) {
        return;
    }

    uint8_t base =
        brightnessTo8Bit(globalBrightness);

    fadeToBlackBy(
        ledsLinks,
        NUM_LEDS_LINKS,
        60
    );

    fadeToBlackBy(
        ledsRechts,
        NUM_LEDS_RECHTS,
        60
    );

    ledsLinks[
        effectStep % NUM_LEDS_LINKS
    ] = CRGB(base, base, base);

    ledsRechts[
        effectStep % NUM_LEDS_RECHTS
    ] = CRGB(base, base, base);

    FastLED.show();

    setLogoBrightness(globalBrightness);

    effectStep++;
}

// ---------------------------------------------------------------------------
// Pulsieren / Atmen
// ---------------------------------------------------------------------------

void applyWave(
    uint8_t bpm,
    uint8_t low
) {
    if (!frameReady(RENDER_INTERVAL)) {
        return;
    }

    uint8_t wave =
        beatsin8(
            bpm,
            low,
            255
        );

    int level =
        (globalBrightness * wave) / 255;

    CRGB color =
        whiteWithBrightness(level);

    fill_solid(
        ledsLinks,
        NUM_LEDS_LINKS,
        color
    );

    fill_solid(
        ledsRechts,
        NUM_LEDS_RECHTS,
        color
    );

    FastLED.show();

    setLogoBrightness(level);
}

// ---------------------------------------------------------------------------
// Gemeinsame Ausgabe
// ---------------------------------------------------------------------------

void showAll(
    uint8_t vLinks,
    uint8_t vRechts,
    uint8_t vLogo
) {
    fill_solid(
        ledsLinks,
        NUM_LEDS_LINKS,
        whiteRaw(vLinks)
    );

    fill_solid(
        ledsRechts,
        NUM_LEDS_RECHTS,
        whiteRaw(vRechts)
    );

    FastLED.show();

    setLogoRaw(vLogo);
}

// ---------------------------------------------------------------------------
// Stimmungsmodi
// ---------------------------------------------------------------------------

void renderDauerlicht() {
    if (!frameReady(RENDER_INTERVAL)) {
        return;
    }

    uint8_t v =
        breathLevel(
            DAUER_PERIOD_MS,
            DAUER_MIN,
            DAUER_MAX
        );

    showAll(
        v,
        v,
        qadd8(v, 20)
    );
}

void renderKerzenlicht() {
    if (!frameReady(RENDER_INTERVAL)) {
        return;
    }

    uint8_t vLinks =
        flickerLevel(
            KERZEN_SPEED_DIV,
            0,
            KERZEN_MIN,
            KERZEN_MAX
        );

    uint8_t vRechts =
        flickerLevel(
            KERZEN_SPEED_DIV,
            30000,
            KERZEN_MIN,
            KERZEN_MAX
        );

    showAll(
        vLinks,
        vRechts,
        KERZEN_LOGO
    );
}

void renderStufenlicht() {
    if (!frameReady(RENDER_INTERVAL)) {
        return;
    }

    uint32_t cycle =
        8UL * STUFEN_STEP_MS +
        STUFEN_HOLD_MS;

    uint32_t t =
        millis() % cycle;

    uint8_t lights;

    if (t < 8UL * STUFEN_STEP_MS) {
        lights =
            t / STUFEN_STEP_MS + 1;
    }
    else {
        lights = 8;
    }

    uint16_t litLinks =
        (uint32_t)NUM_LEDS_LINKS *
        lights / 8;

    uint16_t litRechts =
        (uint32_t)NUM_LEDS_RECHTS *
        lights / 8;

    // Links: von außen nach innen.
    for (
        uint16_t i = 0;
        i < NUM_LEDS_LINKS;
        i++
    ) {
        if (
            i >= NUM_LEDS_LINKS -
            litLinks
        ) {
            ledsLinks[i] =
                whiteRaw(STUFEN_LEVEL);
        }
        else {
            ledsLinks[i] =
                CRGB::Black;
        }
    }

    // Rechts: von außen nach innen.
    for (
        uint16_t i = 0;
        i < NUM_LEDS_RECHTS;
        i++
    ) {
        if (i < litRechts) {
            ledsRechts[i] =
                whiteRaw(STUFEN_LEVEL);
        }
        else {
            ledsRechts[i] =
                CRGB::Black;
        }
    }

    FastLED.show();

    setLogoRaw(STUFEN_LEVEL);
}

void renderDaemmerlicht() {
    if (!frameReady(RENDER_INTERVAL)) {
        return;
    }

    uint8_t v =
        breathLevel(
            DAEMMER_PERIOD_MS,
            DAEMMER_MIN,
            DAEMMER_MAX
        );

    showAll(
        v,
        v,
        (uint16_t)v *
        DAEMMER_LOGO_PCT /
        100
    );
}

void renderWave() {
    if (!frameReady(RENDER_INTERVAL)) {
        return;
    }

    uint8_t base =
        brightnessTo8Bit(globalBrightness);

    uint8_t floorLevel =
        base * 45 / 100;

    uint16_t phase =
        millis() / 16;

    for (
        uint16_t i = 0;
        i < NUM_LEDS_LINKS;
        i++
    ) {
        uint8_t w =
            sin8(i * 16 + phase);

        ledsLinks[i] =
            whiteRaw(
                map(
                    w,
                    0,
                    255,
                    floorLevel,
                    base
                )
            );
    }

    for (
        uint16_t i = 0;
        i < NUM_LEDS_RECHTS;
        i++
    ) {
        uint8_t w =
            sin8(i * 16 + phase);

        ledsRechts[i] =
            whiteRaw(
                map(
                    w,
                    0,
                    255,
                    floorLevel,
                    base
                )
            );
    }

    FastLED.show();

    setLogoBrightness(globalBrightness);
}

void renderFeuerschein() {
    if (!frameReady(RENDER_INTERVAL)) {
        return;
    }

    uint8_t v =
        flickerLevel(
            FEUER_SPEED_DIV,
            0,
            FEUER_MIN,
            FEUER_MAX
        );

    showAll(
        v,
        v,
        FEUER_LOGO
    );
}

void renderNachtlicht() {
    if (!frameReady(RENDER_INTERVAL)) {
        return;
    }

    uint8_t v =
        flickerLevel(
            NACHT_SPEED_DIV,
            40000,
            NACHT_MIN,
            NACHT_MAX
        );

    showAll(
        v,
        v,
        NACHT_LOGO
    );
}

// ---------------------------------------------------------------------------
// Sternenfunkeln
// ---------------------------------------------------------------------------

void renderSternenfunkeln() {
    if (!frameReady(RENDER_INTERVAL)) {
        return;
    }

    uint8_t base =
        brightnessTo8Bit(globalBrightness);

    uint8_t floorLevel =
        (uint16_t)base *
        TWINKLE_BASE_PCT /
        100;

    fadeToBlackBy(
        ledsLinks,
        NUM_LEDS_LINKS,
        TWINKLE_FADE
    );

    fadeToBlackBy(
        ledsRechts,
        NUM_LEDS_RECHTS,
        TWINKLE_FADE
    );

    for (
        uint16_t i = 0;
        i < NUM_LEDS_LINKS;
        i++
    ) {
        if (
            ledsLinks[i].r <
            floorLevel
        ) {
            ledsLinks[i] =
                whiteRaw(floorLevel);
        }
    }

    for (
        uint16_t i = 0;
        i < NUM_LEDS_RECHTS;
        i++
    ) {
        if (
            ledsRechts[i].r <
            floorLevel
        ) {
            ledsRechts[i] =
                whiteRaw(floorLevel);
        }
    }

    if (random8() < TWINKLE_CHANCE) {
        ledsLinks[
            random16(NUM_LEDS_LINKS)
        ] = whiteRaw(base);
    }

    if (random8() < TWINKLE_CHANCE) {
        ledsRechts[
            random16(NUM_LEDS_RECHTS)
        ] = whiteRaw(base);
    }

    FastLED.show();

    setLogoBrightness(
        globalBrightness *
        TWINKLE_BASE_PCT /
        100
    );
}

// ---------------------------------------------------------------------------
// Treffpunkt
// ---------------------------------------------------------------------------

void renderTreffpunkt() {
    if (!frameReady(EFFECT_INTERVAL)) {
        return;
    }

    uint8_t base =
        brightnessTo8Bit(globalBrightness);

    fadeToBlackBy(
        ledsLinks,
        NUM_LEDS_LINKS,
        55
    );

    fadeToBlackBy(
        ledsRechts,
        NUM_LEDS_RECHTS,
        55
    );

    ledsLinks[
        effectStep % NUM_LEDS_LINKS
    ] = CRGB(base, base, base);

    uint16_t posR =
        (NUM_LEDS_RECHTS - 1) -
        (
            effectStep %
            NUM_LEDS_RECHTS
        );

    ledsRechts[posR] =
        CRGB(base, base, base);

    FastLED.show();

    setLogoBrightness(globalBrightness);

    effectStep++;
}

// ---------------------------------------------------------------------------
// Herzschlag
// ---------------------------------------------------------------------------

uint8_t heartBump(
    uint32_t t,
    uint32_t center,
    uint32_t width,
    uint8_t peak
) {
    uint32_t d =
        (t > center)
        ? (t - center)
        : (center - t);

    if (d >= width) {
        return 0;
    }

    return peak -
        (uint32_t)peak *
        d /
        width;
}

void renderHerzschlag() {
    if (!frameReady(RENDER_INTERVAL)) {
        return;
    }

    uint8_t base =
        brightnessTo8Bit(globalBrightness);

    uint8_t low =
        (uint16_t)base *
        HEART_LOW_PCT /
        100;

    uint32_t t =
        millis() % HEART_PERIOD_MS;

    int level = low;

    int b1 =
        heartBump(
            t,
            120,
            170,
            base
        );

    int b2 =
        heartBump(
            t,
            430,
            210,
            base * 4 / 5
        );

    if (b1 > level) {
        level = b1;
    }

    if (b2 > level) {
        level = b2;
    }

    showAll(
        level,
        level,
        level
    );
}

// ---------------------------------------------------------------------------
// Wechsellicht
// ---------------------------------------------------------------------------

void renderWechsellicht() {
    if (!frameReady(RENDER_INTERVAL)) {
        return;
    }

    uint8_t base =
        brightnessTo8Bit(globalBrightness);

    uint8_t low =
        (uint16_t)base *
        WECHSEL_LOW_PCT /
        100;

    uint32_t divisor =
        WECHSEL_PERIOD_MS / 256UL;

    if (divisor == 0) {
        divisor = 1;
    }

    uint8_t phase =
        sin8(millis() / divisor);

    uint8_t vLinks =
        map(
            phase,
            0,
            255,
            low,
            base
        );

    uint8_t vRechts =
        map(
            phase,
            0,
            255,
            base,
            low
        );

    showAll(
        vLinks,
        vRechts,
        (vLinks + vRechts) / 2
    );
}

// ---------------------------------------------------------------------------
// Modus-Klassifizierung
// ---------------------------------------------------------------------------

bool isEffectMode(OperatingMode m) {
    return
        m != MODE_OFF &&
        m != MODE_STATIC &&
        m != MODE_AUTOMATIC;
}

bool isThematic(OperatingMode m) {
    return
        m == MODE_DAUERLICHT ||
        m == MODE_KERZENLICHT ||
        m == MODE_STUFENLICHT ||
        m == MODE_DAEMMERLICHT ||
        m == MODE_FEUERSCHEIN ||
        m == MODE_NACHTLICHT;
}

// ---------------------------------------------------------------------------
// Nachtabschaltung
// ---------------------------------------------------------------------------

bool isNightOff() {
    struct tm t;

    // Ohne gültige Zeit lieber sicher ausschalten.
    if (!getCurrentTime(t)) {
        return true;
    }

    int minutes =
        t.tm_hour * 60 +
        t.tm_min;

    return
        minutes >= nightStartHour * 60 ||
        minutes < nightEndHour * 60;
}

// ---------------------------------------------------------------------------
// Hardware-Dispatch
// ---------------------------------------------------------------------------

void applyHardware() {
    // Stimmungsmodi werden nachts abgeschaltet.
    if (
        isThematic(currentMode) &&
        isNightOff()
    ) {
        showAll(0, 0, 0);
        return;
    }

    switch (currentMode) {

        case MODE_OFF:
            allLEDsOff();
            break;

        case MODE_STATIC:
            applyStatic();
            break;

        case MODE_EFFECT:
            applyEffect();
            break;

        case MODE_AUTOMATIC:
            applyAutomatic();
            break;

        case MODE_PULSE:
            applyWave(20, 120);
            break;

        case MODE_BREATH:
            applyWave(6, 100);
            break;

        case MODE_WAVE:
            renderWave();
            break;

        case MODE_DAUERLICHT:
            renderDauerlicht();
            break;

        case MODE_KERZENLICHT:
            renderKerzenlicht();
            break;

        case MODE_STUFENLICHT:
            renderStufenlicht();
            break;

        case MODE_DAEMMERLICHT:
            renderDaemmerlicht();
            break;

        case MODE_FEUERSCHEIN:
            renderFeuerschein();
            break;

        case MODE_NACHTLICHT:
            renderNachtlicht();
            break;

        case MODE_STERNEN:
            renderSternenfunkeln();
            break;

        case MODE_TREFFPUNKT:
            renderTreffpunkt();
            break;

        case MODE_HERZSCHLAG:
            renderHerzschlag();
            break;

        case MODE_WECHSEL:
            renderWechsellicht();
            break;

        default:
            allLEDsOff();
            break;
    }
}

// ---------------------------------------------------------------------------
// Override
// ---------------------------------------------------------------------------

void enterOverrideIfAutomatic() {
    if (currentMode != MODE_AUTOMATIC) {
        return;
    }

    int window =
        currentAutoWindow();

    currentMode = MODE_STATIC;

    overrideActive = true;
    overrideWindow = window;

    // Der aktuelle Wert bleibt zunächst sichtbar und wird nicht schlagartig
    // verändert.
    applyStatic();

    Serial.printf(
        "Automatik-Override aktiviert. Fenster=%d\n",
        overrideWindow
    );
}

void updateOverrideReturn() {
    if (!overrideActive) {
        return;
    }

    if (currentMode != MODE_STATIC) {
        overrideActive = false;
        overrideWindow = -1;
        return;
    }

    int window =
        currentAutoWindow();

    if (
        window >= 0 &&
        overrideWindow >= 0 &&
        window != overrideWindow
    ) {
        currentMode = MODE_AUTOMATIC;

        overrideActive = false;
        overrideWindow = -1;

        applyAutomatic();

        broadcastStatus();

        Serial.println(
            "Automatik-Rueckkehr."
        );
    }
}

// ---------------------------------------------------------------------------
// JSON Status
// ---------------------------------------------------------------------------

String createStatusJson() {
    JsonDocument doc;

    doc["mode"] = (int)currentMode;

    doc["left"] = brightnessLeft;
    doc["right"] = brightnessRight;
    doc["logo"] = brightnessLogo;

    doc["global"] = globalBrightness;

    doc["autoBrightness"] =
        currentAutoBrightness;

    doc["rtc"] =
        rtcAvailable &&
        isRTCValid();

    doc["ntp"] =
        ntpSynced;

    doc["time"] =
        getTimeString();

    doc["date"] =
        getDateString();

    if (apMode) {
        doc["ip"] =
            WiFi.softAPIP().toString();
    }
    else {
        doc["ip"] =
            WiFi.localIP().toString();
    }

    doc["rssi"] =
        WiFi.status() == WL_CONNECTED
        ? WiFi.RSSI()
        : 0;

    doc["wifi"] =
        WiFi.status() == WL_CONNECTED;

    doc["ap"] =
        apMode;

    doc["ssid"] =
        WIFI_SSID;

    doc["firmware"] =
        FIRMWARE_VERSION;

    doc["override"] =
        overrideActive;

    JsonObject sched =
        doc["sched"].to<JsonObject>();

    sched["tMorning"] =
        morningStartHour;

    sched["tDay"] =
        dayStartHour;

    sched["tEvening"] =
        eveningStartHour;

    sched["tNight"] =
        nightStartHour;

    sched["bMorning"] =
        morningBrightness;

    sched["bDay"] =
        dayBrightness;

    sched["bEveStart"] =
        eveningStartBrightness;

    sched["bEveEnd"] =
        eveningEndBrightness;

    String out;

    serializeJson(
        doc,
        out
    );

    return out;
}

void broadcastStatus() {
    ws.textAll(
        createStatusJson()
    );
}

// ---------------------------------------------------------------------------
// WebSocket
// ---------------------------------------------------------------------------

void onWebSocketEvent(
    AsyncWebSocket *serverPtr,
    AsyncWebSocketClient *client,
    AwsEventType type,
    void *arg,
    uint8_t *data,
    size_t len
) {
    (void)serverPtr;
    (void)arg;

    if (type == WS_EVT_CONNECT) {
        client->text(
            createStatusJson()
        );

        return;
    }

    if (type != WS_EVT_DATA) {
        return;
    }

    if (
        len == 0 ||
        len > MAX_WS_MESSAGE_LEN
    ) {
        Serial.println(
            "WebSocket: Nachricht verworfen."
        );

        return;
    }

    JsonDocument doc;

    DeserializationError err =
        deserializeJson(
            doc,
            data,
            len
        );

    if (err) {
        Serial.println(
            "WebSocket: JSON Fehler."
        );

        return;
    }

    bool changed = false;

    // ---------------------------------------------------------
    // Modus
    // ---------------------------------------------------------

    if (doc["mode"].is<int>()) {

        int mode =
            doc["mode"].as<int>();

        if (
            mode >= MODE_OFF &&
            mode <= MODE_LAST
        ) {
            OperatingMode newMode =
                (OperatingMode)mode;

            if (newMode != currentMode) {

                currentMode = newMode;

                // Bewusst gewählter Modus ist KEIN Override.
                overrideActive = false;
                overrideWindow = -1;

                // Effekt bei erneutem Start sauber von vorne beginnen.
                effectStep = 0;
                lastEffectUpdate = 0;

                // Bei Rückkehr zu Solid-Modus Fade neu starten.
                if (!isEffectMode(currentMode)) {
                    shownLeft = 0;
                    shownRight = 0;
                    shownLogo = 0;
                }

                if (
                    currentMode ==
                    MODE_AUTOMATIC
                ) {
                    applyAutomatic();
                }
                else if (
                    currentMode ==
                    MODE_STATIC
                ) {
                    applyStatic();
                }

                changed = true;
            }
        }
    }

    // ---------------------------------------------------------
    // Manuelle Helligkeiten
    // ---------------------------------------------------------

    if (doc["left"].is<int>()) {

        int value =
            clampBrightness(
                doc["left"].as<int>()
            );

        if (value != brightnessLeft) {
            brightnessLeft = value;

            enterOverrideIfAutomatic();

            changed = true;
        }
    }

    if (doc["right"].is<int>()) {

        int value =
            clampBrightness(
                doc["right"].as<int>()
            );

        if (value != brightnessRight) {
            brightnessRight = value;

            enterOverrideIfAutomatic();

            changed = true;
        }
    }

    if (doc["logo"].is<int>()) {

        int value =
            clampBrightness(
                doc["logo"].as<int>()
            );

        if (value != brightnessLogo) {
            brightnessLogo = value;

            enterOverrideIfAutomatic();

            changed = true;
        }
    }

    // Effekt-Helligkeit und Automatikprofil werden absichtlich NICHT
    // über die App angenommen.

    if (changed) {

        markSettingsDirtyReal();

        // Rendern und Status-Broadcast macht loop() - nicht dieser Async-Task.
        statusUpdateRequested = true;
    }
}

// ---------------------------------------------------------------------------
// WLAN
// ---------------------------------------------------------------------------

void startAccessPoint() {
    WiFi.mode(WIFI_AP);

    bool ok =
        WiFi.softAP(
            SETUP_AP_SSID,
            SETUP_AP_PASS
        );

    apMode = true;
    wifiConnected = false;

    if (ok) {
        Serial.printf(
            "Setup-AP \"%s\" gestartet.\n",
            SETUP_AP_SSID
        );

        Serial.print(
            "Setup-IP: "
        );

        Serial.println(
            WiFi.softAPIP()
        );
    }
    else {
        Serial.println(
            "FEHLER: Setup-AP konnte nicht gestartet werden."
        );
    }
}

void connectWiFi() {
    WiFi.mode(WIFI_STA);

    WiFi.setHostname(
        HOSTNAME
    );

    WiFi.begin(
        WIFI_SSID,
        WIFI_PASS
    );

    Serial.printf(
        "WLAN verbinde mit \"%s\" ",
        WIFI_SSID
    );

    unsigned long start =
        millis();

    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - start < 12000
    ) {
        delay(250);
        Serial.print(".");
    }

    Serial.println();

    wifiConnected =
        WiFi.status() ==
        WL_CONNECTED;

    if (wifiConnected) {

        apMode = false;

        Serial.print(
            "WLAN verbunden. IP: "
        );

        Serial.println(
            WiFi.localIP()
        );
    }
    else {
        Serial.println(
            "WLAN fehlgeschlagen -> Setup-AP."
        );

        startAccessPoint();
    }
}

void updateWiFi() {
    if (apMode) {
        return;
    }

    bool connected =
        WiFi.status() ==
        WL_CONNECTED;

    wifiConnected =
        connected;

    if (connected) {
        return;
    }

    if (
        millis() -
        lastWifiReconnect <
        WIFI_RECONNECT_INTERVAL
    ) {
        return;
    }

    lastWifiReconnect =
        millis();

    Serial.println(
        "WLAN getrennt -> Reconnect..."
    );

    WiFi.disconnect();

    delay(100);

    WiFi.begin(
        WIFI_SSID,
        WIFI_PASS
    );
}

// ---------------------------------------------------------------------------
// NTP
// ---------------------------------------------------------------------------

void setupNTP() {
    if (
        WiFi.status() !=
        WL_CONNECTED
    ) {
        return;
    }

    configTzTime(
        TZ_INFO,
        NTP_SERVER_1,
        NTP_SERVER_2
    );

    struct tm timeinfo;

    if (
        getLocalTime(
            &timeinfo,
            5000
        )
    ) {
        ntpSynced = true;

        if (rtcAvailable) {

            time_t now;

            time(&now);

            struct tm lt;

            localtime_r(
                &now,
                &lt
            );

            rtc.adjust(
                DateTime(
                    lt.tm_year + 1900,
                    lt.tm_mon + 1,
                    lt.tm_mday,
                    lt.tm_hour,
                    lt.tm_min,
                    lt.tm_sec
                )
            );

            Serial.println(
                "RTC mit NTP synchronisiert."
            );
        }
    }
    else {
        Serial.println(
            "NTP: keine Antwort."
        );
    }
}

void updateNTP() {
    if (
        WiFi.status() !=
        WL_CONNECTED
    ) {
        return;
    }

    if (
        millis() -
        lastNtpCheck <
        NTP_CHECK_INTERVAL
    ) {
        return;
    }

    lastNtpCheck =
        millis();

    struct tm timeinfo;

    if (
        getLocalTime(
            &timeinfo,
            1000
        )
    ) {
        ntpSynced = true;

        if (rtcAvailable) {

            time_t now;

            time(&now);

            struct tm lt;

            localtime_r(
                &now,
                &lt
            );

            rtc.adjust(
                DateTime(
                    lt.tm_year + 1900,
                    lt.tm_mon + 1,
                    lt.tm_mday,
                    lt.tm_hour,
                    lt.tm_min,
                    lt.tm_sec
                )
            );
        }
    }
}

// ---------------------------------------------------------------------------
// RTC
// ---------------------------------------------------------------------------

void setupRTC() {
    Wire.begin(
        PIN_I2C_SDA,
        PIN_I2C_SCL
    );

    if (rtc.begin()) {

        rtcAvailable = true;

        Serial.println(
            "DS3231 gefunden."
        );

        if (rtc.lostPower()) {
            Serial.println(
                "DS3231 meldet Stromverlust."
            );
        }

        if (isRTCValid()) {

            Serial.print(
                "RTC: "
            );

            Serial.println(
                getTimeString()
            );
        }
        else {
            Serial.println(
                "RTC-Zeit ungueltig."
            );
        }
    }
    else {

        rtcAvailable = false;

        Serial.println(
            "DS3231 nicht gefunden."
        );
    }
}

// ---------------------------------------------------------------------------
// LED Selbsttest
// ---------------------------------------------------------------------------

void ledSelfTest() {

#if ENABLE_SELFTEST

    Serial.println(
        "LED-Selbsttest: Rot / Gruen / Blau / Weiss"
    );

    CRGB colors[4] = {
        CRGB::Red,
        CRGB::Green,
        CRGB::Blue,
        CRGB::White
    };

    for (
        int r = 0;
        r < SELFTEST_ROUNDS;
        r++
    ) {

        for (
            int i = 0;
            i < 4;
            i++
        ) {

            fill_solid(
                ledsLinks,
                NUM_LEDS_LINKS,
                colors[i]
            );

            fill_solid(
                ledsRechts,
                NUM_LEDS_RECHTS,
                colors[i]
            );

            FastLED.show();

            delay(400);
        }
    }

    fill_solid(
        ledsLinks,
        NUM_LEDS_LINKS,
        CRGB::Black
    );

    fill_solid(
        ledsRechts,
        NUM_LEDS_RECHTS,
        CRGB::Black
    );

    FastLED.show();

#endif
}

// ---------------------------------------------------------------------------
// LED Setup
// ---------------------------------------------------------------------------

void setupLEDs() {

    FastLED.addLeds<
        LED_TYPE,
        PIN_LED_LINKS,
        LED_COLOR_ORDER
    >(
        ledsLinks,
        NUM_LEDS_LINKS
    );

    FastLED.addLeds<
        LED_TYPE,
        PIN_LED_RECHTS,
        LED_COLOR_ORDER
    >(
        ledsRechts,
        NUM_LEDS_RECHTS
    );

    FastLED.setBrightness(
        GLOBAL_MAX_BRIGHTNESS
    );

    FastLED.setMaxPowerInVoltsAndMilliamps(
        LED_VOLTS,
        LED_MAX_MILLIAMPS
    );

    fill_solid(
        ledsLinks,
        NUM_LEDS_LINKS,
        CRGB::Black
    );

    fill_solid(
        ledsRechts,
        NUM_LEDS_RECHTS,
        CRGB::Black
    );

    FastLED.show();

    setupLogoPWM();

    ledSelfTest();

    fill_solid(
        ledsLinks,
        NUM_LEDS_LINKS,
        CRGB::Black
    );

    fill_solid(
        ledsRechts,
        NUM_LEDS_RECHTS,
        CRGB::Black
    );

    FastLED.show();

    setLogoRaw(0);
}

// ---------------------------------------------------------------------------
// Webserver
// ---------------------------------------------------------------------------

void handleRoot(
    AsyncWebServerRequest *request
) {
    request->send(
        200,
        "text/html",
        index_html
    );
}

void handleManifest(
    AsyncWebServerRequest *request
) {
    request->send(
        200,
        "application/manifest+json",
        manifest_json
    );
}

void handleIconSvg(
    AsyncWebServerRequest *request
) {
    request->send(
        200,
        "image/svg+xml",
        icon_svg
    );
}

void handleIcon180(
    AsyncWebServerRequest *request
) {
    request->send(
        200,
        "image/png",
        icon_180_png,
        icon_180_png_len
    );
}

void handleIcon192(
    AsyncWebServerRequest *request
) {
    request->send(
        200,
        "image/png",
        icon_192_png,
        icon_192_png_len
    );
}

void handleIcon512(
    AsyncWebServerRequest *request
) {
    request->send(
        200,
        "image/png",
        icon_512_png,
        icon_512_png_len
    );
}

void handleApiStatus(
    AsyncWebServerRequest *request
) {
    request->send(
        200,
        "application/json",
        createStatusJson()
    );
}

void handleApiSchedule(
    AsyncWebServerRequest *request
) {
    JsonDocument doc;

    doc["tMorning"] =
        morningStartHour;

    doc["tDay"] =
        dayStartHour;

    doc["tEvening"] =
        eveningStartHour;

    doc["tNight"] =
        nightStartHour;

    doc["bMorning"] =
        morningBrightness;

    doc["bDay"] =
        dayBrightness;

    doc["bEveStart"] =
        eveningStartBrightness;

    doc["bEveEnd"] =
        eveningEndBrightness;

    doc["autoBrightness"] =
        currentAutoBrightness;

    String out;

    serializeJson(
        doc,
        out
    );

    request->send(
        200,
        "application/json",
        out
    );
}

// ---------------------------------------------------------------------------
// OTA
// ---------------------------------------------------------------------------

void handleUpdateDone(
    AsyncWebServerRequest *request
) {
    // Ohne gueltige Anmeldung kein Update (siehe OTA_USER/OTA_PASSWORD).
    if (!request->authenticate(OTA_USER, OTA_PASSWORD)) {
        return request->requestAuthentication();
    }

    bool ok =
        !Update.hasError();

    String msg;

    if (ok) {
        msg =
            "Update erfolgreich. Neustart...";
    }
    else {
        msg =
            "Update fehlgeschlagen.";
    }

    AsyncWebServerResponse *response =
        request->beginResponse(
            200,
            "text/plain",
            msg
        );

    response->addHeader(
        "Connection",
        "close"
    );

    request->send(
        response
    );

    if (ok) {

        if (settingsDirty) {
            saveSettings();
            settingsDirty = false;
        }

        delay(500);

        ESP.restart();
    }
}

void handleUpdateUpload(
    AsyncWebServerRequest *request,
    String filename,
    size_t index,
    uint8_t *data,
    size_t len,
    bool final
) {
    (void)filename;

    // Firmware-Daten nur von einem angemeldeten Client annehmen.
    if (!request->authenticate(OTA_USER, OTA_PASSWORD)) {
        return;
    }

    if (index == 0) {

        Serial.println(
            "OTA-Update gestartet."
        );

        if (
            !Update.begin(
                UPDATE_SIZE_UNKNOWN
            )
        ) {
            Update.printError(
                Serial
            );
        }
    }

    if (
        Update.write(
            data,
            len
        ) != len
    ) {
        Update.printError(
            Serial
        );
    }

    if (final) {

        if (
            Update.end(true)
        ) {
            Serial.println(
                "OTA fertig."
            );
        }
        else {
            Update.printError(
                Serial
            );
        }
    }
}

// ---------------------------------------------------------------------------
// Webserver Setup
// ---------------------------------------------------------------------------

void setupWebServer() {

    ws.onEvent(
        onWebSocketEvent
    );

    server.addHandler(
        &ws
    );

    server.on(
        "/",
        HTTP_GET,
        handleRoot
    );

    server.on(
        "/manifest.json",
        HTTP_GET,
        handleManifest
    );

    server.on(
        "/icon.svg",
        HTTP_GET,
        handleIconSvg
    );

    server.on(
        "/apple-touch-icon.png",
        HTTP_GET,
        handleIcon180
    );

    server.on(
        "/icon-192.png",
        HTTP_GET,
        handleIcon192
    );

    server.on(
        "/icon-512.png",
        HTTP_GET,
        handleIcon512
    );

    server.on(
        "/api/status",
        HTTP_GET,
        handleApiStatus
    );

    server.on(
        "/api/schedule",
        HTTP_GET,
        handleApiSchedule
    );

    server.on(
        "/update",
        HTTP_POST,
        handleUpdateDone,
        handleUpdateUpload
    );

    server.begin();

    if (
        MDNS.begin(HOSTNAME)
    ) {

        MDNS.addService(
            "http",
            "tcp",
            80
        );

        Serial.printf(
            "mDNS: http://%s.local/\n",
            HOSTNAME
        );
    }
    else {
        Serial.println(
            "mDNS konnte nicht gestartet werden."
        );
    }

    Serial.println(
        "Webserver gestartet."
    );
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

void setup() {

    Serial.begin(115200);

    delay(300);

    Serial.printf(
        "\n=== Fassadenbeleuchtung ESP32 - Firmware %s ===\n",
        FIRMWARE_VERSION
    );

    loadSettings();

    setupRTC();

    setupLEDs();

    // Definierter Power-On-State:
    // Immer Automatik.
    currentMode =
        MODE_AUTOMATIC;

    overrideActive = false;
    overrideWindow = -1;

    applyAutomatic();

    connectWiFi();

    setupNTP();

    setupWebServer();

    broadcastStatus();

    Serial.println(
        "System bereit."
    );

    Serial.printf(
        "Zeit: %s %s\n",
        getDateString().c_str(),
        getTimeString().c_str()
    );
}

// ---------------------------------------------------------------------------
// Hauptschleife
// ---------------------------------------------------------------------------

void loop() {

    ws.cleanupClients();

    // WLAN-Zustand überwachen.
    updateWiFi();

    // NTP nachführen.
    updateNTP();

    // Automatik jede Sekunde neu berechnen.
    if (
        currentMode == MODE_AUTOMATIC &&
        millis() - lastAutoUpdate >=
        AUTO_UPDATE_INTERVAL
    ) {
        lastAutoUpdate =
            millis();

        applyAutomatic();
    }

    // Prüfen, ob Override in neues Zeitfenster zurückkehren soll.
    updateOverrideReturn();

    bool nowEffect =
        isEffectMode(currentMode);

    // Beim Wechsel Effekt -> Solid sauber neu faden.
    if (
        prevFrameEffect &&
        !nowEffect
    ) {
        shownLeft = 0;
        shownRight = 0;
        shownLogo = 0;

        lastRender = 0;
    }

    prevFrameEffect =
        nowEffect;

    // Ausgabe.
    if (nowEffect) {
        applyHardware();
    }
    else {
        applyHardware();
        renderSolid();
    }

    // Einstellungen zeitverzögert speichern.
    if (
        settingsDirty &&
        millis() - lastSettingsChange >=
        SAVE_DEBOUNCE_MS
    ) {
        saveSettings();

        settingsDirty = false;
    }

    // Auf einen WebSocket-Befehl sofort mit aktuellem Status antworten.
    if (statusUpdateRequested) {
        statusUpdateRequested = false;

        lastStatusBroadcast = millis();

        broadcastStatus();
    }

    // Status regelmäßig pushen.
    if (
        millis() - lastStatusBroadcast >=
        STATUS_INTERVAL
    ) {
        lastStatusBroadcast =
            millis();

        broadcastStatus();
    }

    delay(5);
}
