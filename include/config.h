// ============================================================================
//  config.h - Zentrale Konfiguration der Fassadenbeleuchtung
//
//  Hier stehen ALLE einstellbaren Festwerte (Pins, LED-Anzahl, Grenzwerte,
//  Werte der thematischen Modi). Die Logik liegt in main.cpp. So laesst sich
//  die Anlage an neue Hardware anpassen, ohne die Steuerlogik anzufassen.
//
//  WICHTIG: bleibt auf Arduino-ESP32-Core 2.x (alte LEDC-PWM-API).
// ============================================================================
#pragma once

// ---------------------------------------------------------------------------
//  Firmware / Netzwerk-Namen
// ---------------------------------------------------------------------------
static const char* FIRMWARE_VERSION = "2.4.0";
#define HOSTNAME           "led-fassade"      // -> http://led-fassade.local

// WLAN-Startwerte. Werden in NVS gespeichert und sind per App aenderbar.
#define DEFAULT_WIFI_SSID  "Museum-Arbeitswelt"
#define DEFAULT_WIFI_PASS  "willkommen"
#define SETUP_AP_SSID      "Fassade-Setup"    // Fallback-AP bei fehlender Verbindung
#define SETUP_AP_PASS      "fassade2026"      // mindestens 8 Zeichen

// ---------------------------------------------------------------------------
//  Pins
// ---------------------------------------------------------------------------
#define PIN_LED_LINKS      23
#define PIN_LED_RECHTS     13
#define PIN_LOGO_PWM       14
#define PIN_I2C_SDA        21
#define PIN_I2C_SCL        22

// ---------------------------------------------------------------------------
//  LED-Streifen
// ---------------------------------------------------------------------------
#define NUM_LEDS_LINKS     10
#define NUM_LEDS_RECHTS    1
#define LED_TYPE           WS2812
#define LED_COLOR_ORDER    GRB

// Strom-/Waermeschutz
#define LED_VOLTS              5
#define LED_MAX_MILLIAMPS      2000
#define GLOBAL_MAX_BRIGHTNESS  200            // Software-Deckel 0..255 (Segmente + Logo)

// Logo-PWM (dimmbar ueber MOSFET)
#define LOGO_PWM_CHANNEL   0
#define LOGO_PWM_FREQ      5000
#define LOGO_PWM_RES       8

// Selbsttest beim Einschalten (0 = aus)
#define ENABLE_SELFTEST    1
#define SELFTEST_ROUNDS    1

// ---------------------------------------------------------------------------
//  Betriebsmodi
// ---------------------------------------------------------------------------
enum OperatingMode {
    MODE_OFF       = 0,
    MODE_STATIC    = 1,
    MODE_EFFECT    = 2,
    MODE_AUTOMATIC = 3,
    MODE_PULSE     = 4,
    MODE_BREATH    = 5,
    // Thematische Ausstellungs-Modi (aus led_sample3), fest hinterlegt:
    MODE_NER_TAMID = 6,
    MODE_SCHABBAT  = 7,
    MODE_CHANUKKA  = 8,
    MODE_GEDENKEN  = 9,
    MODE_LAST      = MODE_GEDENKEN
};

// ---------------------------------------------------------------------------
//  Feste Werte der thematischen Modi (0..255)
//
//  Bewusst NICHT ueber die Regler verstellbar (Ausstellungsbetrieb) -
//  Anpassung nur hier + Neu-Flash/OTA.
// ---------------------------------------------------------------------------
#define NER_MIN            186          // Ewiges Licht: unteres Atmen (~73 %)
#define NER_MAX            208          // Ewiges Licht: oberes Atmen (~82 %)
#define NER_PERIOD_MS      18000UL      // Dauer eines vollen Atemzugs

#define SCHABBAT_MIN       108          // Kerze: dunkelster Flackerpunkt
#define SCHABBAT_MAX       172          // Kerze: hellster Flackerpunkt
#define SCHABBAT_SPEED_DIV 20           // groesser = langsameres Flackern
#define SCHABBAT_LOGO      100          // Logo ruhig hinter den Kerzen

#define CHANUKKA_LEVEL     220          // Helligkeit der angezuendeten Lichter
#define CHANUKKA_STEP_MS   1500         // Zeit je zusaetzlichem Licht
#define CHANUKKA_HOLD_MS   3000         // Halten bei 8 Lichtern, dann von vorne

#define GEDENKEN_MIN       14           // dunkelster Punkt (~5 %)
#define GEDENKEN_MAX       46           // hellster Punkt (~18 %)
#define GEDENKEN_PERIOD_MS 150000UL     // Dauer eines vollen Auf-/Abschwellens
#define GEDENKEN_LOGO_PCT  60           // Logo folgt gedaempft (% des Segmentwerts)
