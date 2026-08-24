#pragma once

static const char* FIRMWARE_VERSION = "2.4.1";
#define HOSTNAME           "led-fassade"

// WLAN-Zugangsdaten - fest im Code, nicht über die App änderbar.
#define WIFI_SSID          "Museum-Arbeitswelt"
#define WIFI_PASS          "willkommen"
#define SETUP_AP_SSID      "Fassade-Setup"
#define SETUP_AP_PASS      "fassade2026"

#define PIN_LED_LINKS      23
#define PIN_LED_RECHTS     13
#define PIN_LOGO_PWM       14
#define PIN_I2C_SDA        21
#define PIN_I2C_SCL        22

#define NUM_LEDS_LINKS     60
#define NUM_LEDS_RECHTS    60
#define LED_TYPE           WS2812
#define LED_COLOR_ORDER    GRB

#define LED_VOLTS              5
// TODO Hardware: bei 60+60 LEDs zu niedrig (Vollweiss ~7,2 A). Vor dem
// echten Einsatz an das Netzteil anpassen, sonst dimmt FastLED herunter.
#define LED_MAX_MILLIAMPS      2000
#define GLOBAL_MAX_BRIGHTNESS  200

// Pflichtenheft Kap. 8.3: Ist keine gueltige Zeit verfuegbar (leere RTC-Batterie
// und noch keine NTP-Synchronisation), faehrt die Automatik in einen sicheren,
// gedimmten Grundzustand, statt komplett hell oder dunkel zu schalten. Sobald
// NTP verfuegbar ist, korrigiert sich das System selbst.
#define SAFE_DEFAULT_BRIGHTNESS  25

#define LOGO_PWM_CHANNEL   0
#define LOGO_PWM_FREQ      5000
#define LOGO_PWM_RES       8

#define ENABLE_SELFTEST    1
#define SELFTEST_ROUNDS    1

enum OperatingMode {
    MODE_OFF       = 0,
    MODE_STATIC    = 1,
    MODE_EFFECT    = 2,
    MODE_AUTOMATIC = 3,
    MODE_PULSE     = 4,
    MODE_BREATH    = 5,

    MODE_DAUERLICHT   = 6,
    MODE_KERZENLICHT  = 7,
    MODE_STUFENLICHT  = 8,
    MODE_DAEMMERLICHT = 9,
    MODE_WAVE         = 10,
    MODE_FEUERSCHEIN  = 11,
    MODE_NACHTLICHT   = 12,
    MODE_LAST         = MODE_NACHTLICHT
};

#define DAUER_MIN            186
#define DAUER_MAX            208
#define DAUER_PERIOD_MS      18000UL

#define KERZEN_MIN       108
#define KERZEN_MAX       172
#define KERZEN_SPEED_DIV 20
#define KERZEN_LOGO      100

#define STUFEN_LEVEL     220
#define STUFEN_STEP_MS   1500
#define STUFEN_HOLD_MS   3000

#define DAEMMER_MIN       14
#define DAEMMER_MAX       46
#define DAEMMER_PERIOD_MS 150000UL
#define DAEMMER_LOGO_PCT  60

#define FEUER_MIN        150
#define FEUER_MAX        230
#define FEUER_SPEED_DIV  12
#define FEUER_LOGO       190

#define NACHT_MIN          30
#define NACHT_MAX          54
#define NACHT_SPEED_DIV    28
#define NACHT_LOGO         44
