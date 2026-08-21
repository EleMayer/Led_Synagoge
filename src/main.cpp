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

OperatingMode currentMode = MODE_AUTOMATIC;

bool overrideActive = false;
int  overrideWindow = -1;

int brightnessLeft  = 80;
int brightnessRight = 80;
int brightnessLogo  = 80;
int globalBrightness = 80;

int nightStartHour   = 23;
int nightEndHour     = 6;
int morningStartHour = 6;
int morningEndHour   = 8;
int dayStartHour     = 8;
int dayEndHour       = 18;
int eveningStartHour = 18;
int eveningEndHour   = 23;

int morningBrightness      = 90;
int dayBrightness          = 90;
int eveningStartBrightness = 60;
int eveningEndBrightness   = 25;
int currentAutoBrightness  = 0;

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

#define MAX_PRESETS 6
#define PRESET_NAME_LEN 16
struct Preset {
    bool used;
    char name[PRESET_NAME_LEN];
    int  left;
    int  right;
    int  logo;
};
Preset presets[MAX_PRESETS];

RTC_DS3231 rtc;
Preferences preferences;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

bool rtcAvailable  = false;
bool wifiConnected = false;
bool ntpSynced     = false;
bool apMode        = false;
char wifiSsid[33];
char wifiPass[65];

const char *NTP_SERVER_1 = "pool.ntp.org";
const char *NTP_SERVER_2 = "time.nist.gov";
const char *TZ_INFO = "CET-1CEST,M3.5.0,M10.5.0";

uint16_t      effectStep = 0;
unsigned long lastEffectUpdate = 0;
const uint32_t EFFECT_INTERVAL = 40;
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

int clampBrightness(int value) {
    return constrain(value, 0, 100);
}

uint8_t brightnessTo8Bit(int brightness) {
    int safe = clampBrightness(brightness);
    return map(safe, 0, 100, 0, 255);
}

CRGB whiteWithBrightness(int brightness) {
    uint8_t v = brightnessTo8Bit(brightness);
    return CRGB(v, v, v);
}

void saveSettings() {
    preferences.begin("facelight", false);
    preferences.putInt("left", brightnessLeft);
    preferences.putInt("right", brightnessRight);
    preferences.putInt("logo", brightnessLogo);
    preferences.putInt("global", globalBrightness);
    preferences.putInt("morn", morningBrightness);
    preferences.putInt("day", dayBrightness);
    preferences.putInt("eveStart", eveningStartBrightness);
    preferences.putInt("eveEnd", eveningEndBrightness);

    preferences.putInt("mStart", morningStartHour);
    preferences.putInt("mEnd", morningEndHour);
    preferences.putInt("dStart", dayStartHour);
    preferences.putInt("dEnd", dayEndHour);
    preferences.putInt("eStart", eveningStartHour);
    preferences.putInt("eEnd", eveningEndHour);
    preferences.putInt("nightStart", nightStartHour);
    preferences.putInt("nightEnd", nightEndHour);
    preferences.end();
    settingsDirty = false;
}

void markSettingsDirty() {
    settingsDirty = true;
    lastSettingsChange = millis();
}

void loadSettings() {
    preferences.begin("facelight", true);
    brightnessLeft         = preferences.getInt("left", 80);
    brightnessRight        = preferences.getInt("right", 80);
    brightnessLogo         = preferences.getInt("logo", 80);
    globalBrightness       = preferences.getInt("global", 80);
    morningBrightness      = preferences.getInt("morn", 90);
    dayBrightness          = preferences.getInt("day", 90);
    eveningStartBrightness = preferences.getInt("eveStart", 60);
    eveningEndBrightness   = preferences.getInt("eveEnd", 25);
    morningStartHour       = preferences.getInt("mStart", 6);
    morningEndHour         = preferences.getInt("mEnd", 8);
    dayStartHour           = preferences.getInt("dStart", 8);
    dayEndHour             = preferences.getInt("dEnd", 18);
    eveningStartHour       = preferences.getInt("eStart", 18);
    eveningEndHour         = preferences.getInt("eEnd", 23);
    nightStartHour         = preferences.getInt("nightStart", 23);
    nightEndHour           = preferences.getInt("nightEnd", 6);
    preferences.end();

    brightnessLeft         = clampBrightness(brightnessLeft);
    brightnessRight        = clampBrightness(brightnessRight);
    brightnessLogo         = clampBrightness(brightnessLogo);
    globalBrightness       = clampBrightness(globalBrightness);
    morningBrightness      = clampBrightness(morningBrightness);
    dayBrightness          = clampBrightness(dayBrightness);
    eveningStartBrightness = clampBrightness(eveningStartBrightness);
    eveningEndBrightness   = clampBrightness(eveningEndBrightness);
    morningStartHour       = constrain(morningStartHour, 0, 23);
    morningEndHour         = constrain(morningEndHour, 0, 23);
    dayStartHour           = constrain(dayStartHour, 0, 23);
    dayEndHour             = constrain(dayEndHour, 0, 23);
    eveningStartHour       = constrain(eveningStartHour, 0, 23);
    eveningEndHour         = constrain(eveningEndHour, 0, 23);
    nightStartHour         = constrain(nightStartHour, 0, 23);
    nightEndHour           = constrain(nightEndHour, 0, 23);
}

void loadWifi() {
    preferences.begin("wifi", true);
    String s = preferences.getString("ssid", DEFAULT_WIFI_SSID);
    String p = preferences.getString("pass", DEFAULT_WIFI_PASS);
    preferences.end();

    strncpy(wifiSsid, s.c_str(), sizeof(wifiSsid) - 1);
    wifiSsid[sizeof(wifiSsid) - 1] = 0;
    strncpy(wifiPass, p.c_str(), sizeof(wifiPass) - 1);
    wifiPass[sizeof(wifiPass) - 1] = 0;
}

void saveWifi(const char *ssid, const char *pass) {
    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("pass", pass);
    preferences.end();
}

void savePresets() {
    preferences.begin("presets", false);
    preferences.putBytes("data", presets, sizeof(presets));
    preferences.end();
}

void loadPresets() {
    preferences.begin("presets", true);
    size_t stored = preferences.getBytesLength("data");

    if (stored == sizeof(presets)) {
        preferences.getBytes("data", presets, sizeof(presets));
    } else {

        for (int i = 0; i < MAX_PRESETS; i++) {
            presets[i].used = false;
            presets[i].name[0] = '\0';
            presets[i].left = 80;
            presets[i].right = 80;
            presets[i].logo = 80;
        }
    }
    preferences.end();

    for (int i = 0; i < MAX_PRESETS; i++) {
        presets[i].left  = clampBrightness(presets[i].left);
        presets[i].right = clampBrightness(presets[i].right);
        presets[i].logo  = clampBrightness(presets[i].logo);
        presets[i].name[PRESET_NAME_LEN - 1] = '\0';
    }
}

int findPresetByName(const char *name) {
    for (int i = 0; i < MAX_PRESETS; i++) {
        if (!presets[i].used) {
            continue;
        }
        if (strncmp(presets[i].name, name, PRESET_NAME_LEN - 1) == 0) {
            return i;
        }
    }
    return -1;
}

int storeCurrentAsPreset(const char *name) {
    int slot = findPresetByName(name);

    if (slot < 0) {
        for (int i = 0; i < MAX_PRESETS; i++) {
            if (!presets[i].used) {
                slot = i;
                break;
            }
        }
    }

    if (slot < 0) {
        return -1;
    }

    presets[slot].used = true;
    strncpy(presets[slot].name, name, PRESET_NAME_LEN - 1);
    presets[slot].name[PRESET_NAME_LEN - 1] = '\0';
    presets[slot].left  = brightnessLeft;
    presets[slot].right = brightnessRight;
    presets[slot].logo  = brightnessLogo;
    savePresets();
    return slot;
}

void deletePreset(int slot) {
    if (slot < 0 || slot >= MAX_PRESETS) {
        return;
    }
    presets[slot].used = false;
    presets[slot].name[0] = '\0';
    savePresets();
}

bool isRTCValid() {
    if (!rtcAvailable) {
        return false;
    }
    DateTime now = rtc.now();
    return now.year() >= 2024 && now.year() <= 2099;
}

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

int calculateAutomaticBrightness() {
    struct tm t;
    if (!getCurrentTime(t)) {
        return 25;
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

void setLogoBrightness(int brightness) {
    uint32_t duty = brightnessTo8Bit(clampBrightness(brightness));
    duty = duty * GLOBAL_MAX_BRIGHTNESS / 255;
    ledcWrite(LOGO_PWM_CHANNEL, duty);
}

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

void allLEDsOff() {
    targetLeft  = 0;
    targetRight = 0;
    targetLogo  = 0;
}

void applyStatic() {
    targetLeft  = brightnessLeft;
    targetRight = brightnessRight;
    targetLogo  = brightnessLogo;
}

void applyAutomatic() {
    int value = calculateAutomaticBrightness();
    currentAutoBrightness = value;
    targetLeft  = value;
    targetRight = value;
    targetLogo  = value;
}

bool frameReady(uint32_t interval) {
    if (millis() - lastEffectUpdate < interval) {
        return false;
    }
    lastEffectUpdate = millis();
    return true;
}

uint8_t breathLevel(uint32_t periodMs, uint8_t lo, uint8_t hi) {
    uint8_t s = sin8(millis() / (periodMs / 256));
    return map(s, 0, 255, lo, hi);
}

uint8_t flickerLevel(uint16_t speedDiv, uint16_t seed, uint8_t lo, uint8_t hi) {
    uint8_t n = inoise8(millis() / speedDiv, seed);
    return map(n, 0, 255, lo, hi);
}

void applyEffect() {
    if (!frameReady(EFFECT_INTERVAL)) return;

    uint8_t base = brightnessTo8Bit(globalBrightness);

    fill_solid(ledsLinks,  NUM_LEDS_LINKS,  CRGB::Black);
    fill_solid(ledsRechts, NUM_LEDS_RECHTS, CRGB::Black);

    ledsLinks[effectStep % NUM_LEDS_LINKS]   = CRGB(base, base, base);
    ledsRechts[effectStep % NUM_LEDS_RECHTS] = CRGB(base, base, base);
    FastLED.show();

    setLogoBrightness(globalBrightness);
    effectStep++;
}

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

CRGB whiteRaw(uint8_t v) {
    return CRGB(v, v, v);
}

void setLogoRaw(uint8_t level) {
    uint32_t duty = (uint32_t)level * GLOBAL_MAX_BRIGHTNESS / 255;
    ledcWrite(LOGO_PWM_CHANNEL, duty);
}

bool isNightOff() {
    struct tm t;
    if (!getCurrentTime(t)) {
        return false;
    }
    int m = t.tm_hour * 60 + t.tm_min;
    return m >= nightStartHour * 60 || m < nightEndHour * 60;
}

void showAll(uint8_t vLinks, uint8_t vRechts, uint8_t vLogo) {
    fill_solid(ledsLinks,  NUM_LEDS_LINKS,  whiteRaw(vLinks));
    fill_solid(ledsRechts, NUM_LEDS_RECHTS, whiteRaw(vRechts));
    FastLED.show();
    setLogoRaw(vLogo);
}

void renderDauerlicht() {
    if (!frameReady(RENDER_INTERVAL)) return;
    uint8_t v = breathLevel(DAUER_PERIOD_MS, DAUER_MIN, DAUER_MAX);
    showAll(v, v, qadd8(v, 20));
}

void renderKerzenlicht() {
    if (!frameReady(RENDER_INTERVAL)) return;
    uint8_t vLinks  = flickerLevel(KERZEN_SPEED_DIV, 0,     KERZEN_MIN, KERZEN_MAX);
    uint8_t vRechts = flickerLevel(KERZEN_SPEED_DIV, 30000, KERZEN_MIN, KERZEN_MAX);
    showAll(vLinks, vRechts, KERZEN_LOGO);
}

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

void renderDaemmerlicht() {
    if (!frameReady(RENDER_INTERVAL)) return;
    uint8_t v = breathLevel(DAEMMER_PERIOD_MS, DAEMMER_MIN, DAEMMER_MAX);
    showAll(v, v, (uint16_t)v * DAEMMER_LOGO_PCT / 100);
}

void renderWave() {
    if (!frameReady(RENDER_INTERVAL)) return;

    uint8_t base = brightnessTo8Bit(globalBrightness);
    uint16_t phase = millis() / 8;

    for (uint16_t i = 0; i < NUM_LEDS_LINKS; i++) {
        uint8_t w = sin8(i * 16 + phase);
        ledsLinks[i] = whiteRaw(scale8(base, w));
    }
    for (uint16_t i = 0; i < NUM_LEDS_RECHTS; i++) {
        uint8_t w = sin8(i * 16 + phase);
        ledsRechts[i] = whiteRaw(scale8(base, w));
    }
    FastLED.show();
    setLogoBrightness(globalBrightness);
}

void renderFeuerschein() {
    if (!frameReady(RENDER_INTERVAL)) return;
    uint8_t v = flickerLevel(FEUER_SPEED_DIV, 0, FEUER_MIN, FEUER_MAX);
    showAll(v, v, FEUER_LOGO);
}

void renderNachtlicht() {
    if (!frameReady(RENDER_INTERVAL)) return;
    uint8_t v = flickerLevel(NACHT_SPEED_DIV, 40000, NACHT_MIN, NACHT_MAX);
    showAll(v, v, NACHT_LOGO);
}

bool isEffectMode(OperatingMode m) {
    return m != MODE_OFF && m != MODE_STATIC && m != MODE_AUTOMATIC;
}

bool isThematic(OperatingMode m) {
    return m == MODE_DAUERLICHT || m == MODE_KERZENLICHT ||
           m == MODE_STUFENLICHT || m == MODE_DAEMMERLICHT ||
           m == MODE_FEUERSCHEIN || m == MODE_NACHTLICHT;
}

void applyHardware() {
    if (isThematic(currentMode) && isNightOff()) {
        showAll(0, 0, 0);
        return;
    }

    switch (currentMode) {
        case MODE_OFF:          allLEDsOff();         break;
        case MODE_STATIC:       applyStatic();        break;
        case MODE_EFFECT:       applyEffect();        break;
        case MODE_PULSE:        applyWave(50, 15);    break;
        case MODE_BREATH:       applyWave(8, 3);      break;
        case MODE_AUTOMATIC:    applyAutomatic();     break;
        case MODE_WAVE:         renderWave();         break;
        case MODE_DAUERLICHT:   renderDauerlicht();   break;
        case MODE_KERZENLICHT:  renderKerzenlicht();  break;
        case MODE_STUFENLICHT:  renderStufenlicht();  break;
        case MODE_DAEMMERLICHT: renderDaemmerlicht(); break;
        case MODE_FEUERSCHEIN:  renderFeuerschein();  break;
        case MODE_NACHTLICHT:   renderNachtlicht();   break;
    }
}

void enterOverrideIfAutomatic() {
    if (currentMode == MODE_AUTOMATIC) {
        currentMode = MODE_STATIC;
        overrideActive = true;
        overrideWindow = currentAutoWindow();
    }
}

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
    doc["ssid"] = wifiSsid;
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

    JsonArray array = doc["presets"].to<JsonArray>();
    for (int i = 0; i < MAX_PRESETS; i++) {
        if (!presets[i].used) {
            continue;
        }
        JsonObject p = array.add<JsonObject>();
        p["slot"]  = i;
        p["name"]  = presets[i].name;
        p["left"]  = presets[i].left;
        p["right"] = presets[i].right;
        p["logo"]  = presets[i].logo;
    }

    String out;
    serializeJson(doc, out);
    return out;
}

void broadcastStatus() {
    ws.textAll(createStatusJson());
}

void onWebSocketEvent(AsyncWebSocket *serverPtr, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {

    if (type == WS_EVT_CONNECT) {
        client->text(createStatusJson());
        return;
    }
    if (type != WS_EVT_DATA) {
        return;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, data, len);
    if (err) {
        Serial.println("JSON Fehler");
        return;
    }

    if (doc["wifiSsid"].is<const char *>()) {
        const char *s = doc["wifiSsid"].as<const char *>();
        const char *p = doc["wifiPass"] | "";
        if (s && strlen(s) > 0) {
            saveWifi(s, p);
            if (settingsDirty) {
                saveSettings();
            }
            client->text("{\"reboot\":true}");
            delay(300);
            ESP.restart();
        }
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

    if (doc["tMorning"].is<int>()) {
        int h = constrain(doc["tMorning"].as<int>(), 0, 23);
        morningStartHour = h;
        nightEndHour     = h;
        changed = true;
    }
    if (doc["tDay"].is<int>()) {
        int h = constrain(doc["tDay"].as<int>(), 0, 23);
        morningEndHour = h;
        dayStartHour   = h;
        changed = true;
    }
    if (doc["tEvening"].is<int>()) {
        int h = constrain(doc["tEvening"].as<int>(), 0, 23);
        dayEndHour       = h;
        eveningStartHour = h;
        changed = true;
    }
    if (doc["tNight"].is<int>()) {
        int h = constrain(doc["tNight"].as<int>(), 0, 23);
        eveningEndHour = h;
        nightStartHour = h;
        changed = true;
    }

    if (doc["bMorning"].is<int>()) {
        morningBrightness = clampBrightness(doc["bMorning"].as<int>());
        changed = true;
    }
    if (doc["bDay"].is<int>()) {
        dayBrightness = clampBrightness(doc["bDay"].as<int>());
        changed = true;
    }
    if (doc["bEveStart"].is<int>()) {
        eveningStartBrightness = clampBrightness(doc["bEveStart"].as<int>());
        changed = true;
    }
    if (doc["bEveEnd"].is<int>()) {
        eveningEndBrightness = clampBrightness(doc["bEveEnd"].as<int>());
        changed = true;
    }

    if (doc["savePreset"].is<const char *>()) {
        const char *name = doc["savePreset"].as<const char *>();
        if (name && strlen(name) > 0) {
            storeCurrentAsPreset(name);
            changed = true;
        }
    }

    if (doc["applyPreset"].is<int>()) {
        int slot = doc["applyPreset"].as<int>();
        if (slot >= 0 && slot < MAX_PRESETS && presets[slot].used) {
            brightnessLeft  = presets[slot].left;
            brightnessRight = presets[slot].right;
            brightnessLogo  = presets[slot].logo;
            currentMode = MODE_STATIC;
            overrideActive = true;
            overrideWindow = currentAutoWindow();
            changed = true;
        }
    }

    if (doc["deletePreset"].is<int>()) {
        deletePreset(doc["deletePreset"].as<int>());
        changed = true;
    }

    if (changed) {
        markSettingsDirty();
        applyHardware();
        broadcastStatus();
    }
}

void startAccessPoint() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(SETUP_AP_SSID, SETUP_AP_PASS);
    apMode = true;
    wifiConnected = false;
    Serial.printf("Setup-AP \"%s\" -> http://%s/\n",
                  SETUP_AP_SSID, WiFi.softAPIP().toString().c_str());
}

void connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(HOSTNAME);
    WiFi.begin(wifiSsid, wifiPass);
    Serial.printf("WLAN verbinde mit \"%s\" ", wifiSsid);

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

void setupWebServer() {
    ws.onEvent(onWebSocketEvent);
    server.addHandler(&ws);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *r) {
        r->send(200, "text/html", index_html);
    });

    server.on("/manifest.json", HTTP_GET, [](AsyncWebServerRequest *r) {
        r->send(200, "application/manifest+json", manifest_json);
    });

    server.on("/icon.svg", HTTP_GET, [](AsyncWebServerRequest *r) {
        r->send(200, "image/svg+xml", icon_svg);
    });

    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *r) {
        r->send(200, "application/json", createStatusJson());
    });

    server.on("/api/schedule", HTTP_GET, [](AsyncWebServerRequest *r) {
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
        r->send(200, "application/json", out);
    });

    server.on("/update", HTTP_POST,
        [](AsyncWebServerRequest *request) {
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
        },
        [](AsyncWebServerRequest *request, String filename, size_t index,
           uint8_t *data, size_t len, bool final) {
            if (!index) {
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
        });

    server.begin();

    if (MDNS.begin(HOSTNAME)) {
        MDNS.addService("http", "tcp", 80);
        Serial.printf("mDNS: http://%s.local/\n", HOSTNAME);
    }
    Serial.println("Webserver gestartet.");
}

void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.printf("\n=== Fassadenbeleuchtung ESP32 - Firmware %s ===\n", FIRMWARE_VERSION);

    loadSettings();
    loadPresets();
    loadWifi();

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

void loop() {
    ws.cleanupClients();
    updateNTP();

    if (currentMode == MODE_AUTOMATIC && millis() - lastAutoUpdate >= AUTO_UPDATE_INTERVAL) {
        lastAutoUpdate = millis();
        applyAutomatic();
    }

    bool nowEffect = isEffectMode(currentMode);

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
