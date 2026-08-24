#pragma once

// Zentrale Konfiguration: Netzwerk, Pinbelegung, LED-/PWM-Parameter, Grenzwerte
// und die Aufzaehlung der Betriebsmodi. Wird von main.cpp eingebunden.

static const char* FIRMWARE_VERSION = "2.4.3";
#define HOSTNAME           "led-fassade"   // mDNS-Name -> http://led-fassade.local

// WLAN-Zugangsdaten - fest im Code, nicht über die App änderbar.
#define WIFI_SSID          "Museum-Arbeitswelt"
#define WIFI_PASS          "willkommen"
// Fallback-Accesspoint, falls das WLAN nicht erreichbar ist.
#define SETUP_AP_SSID      "Fassade-Setup"
#define SETUP_AP_PASS      "fassade2026"

// GPIO-Belegung (Details siehe Pinbelegung in README.md).
#define PIN_LED_LINKS      23   // Datenausgang Segment Links
#define PIN_LED_RECHTS     13   // Datenausgang Segment Rechts
#define PIN_LOGO_PWM       14   // PWM zum Logo-MOSFET
#define PIN_I2C_SDA        21   // I2C zum RTC-Modul DS3231
#define PIN_I2C_SCL        22

// LED-Streifen je Segment. Typ/Anzahl an die reale Hardware anpassen
// (Pflichtenheft: WS2811, 12 V; hier Testaufbau mit WS2812).
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

// Automatik-Helligkeiten (Prozent) fuer die Tageskurve. Bewusst FEST im Code:
// nur hier aenderbar, nicht ueber die App. Die Uhrzeiten der Uebergaenge bleiben
// dagegen in der App einstellbar.
//   Morgen  = Zielhelligkeit am Ende des Hochfahrens
//   Tag     = konstante Tageshelligkeit
//   EveStart/EveEnd = Abend-Rampe (Beginn -> Ende, wird zur Nacht ausgeblendet)
#define AUTO_B_MORNING     90
#define AUTO_B_DAY         90
#define AUTO_B_EVE_START   60
#define AUTO_B_EVE_END     25

// Automatik-Uhrzeiten (Stunde 0-23) der Zeitfenster-Uebergaenge. Ebenfalls FEST
// im Code, nicht ueber die App aenderbar. Muessen aufsteigend sein:
// MORNING < DAY < EVENING < NIGHT.
//   MORNING = Beginn Hochfahren   DAY     = Beginn Tageshelligkeit
//   EVENING = Beginn Abendrampe   NIGHT   = Nachtabschaltung (bis MORNING aus)
#define AUTO_T_MORNING      6
#define AUTO_T_DAY          8
#define AUTO_T_EVENING     18
#define AUTO_T_NIGHT       23

// PWM-Kanal fuer das dimmbare Logo (alte LEDC-API, Arduino-Core 2.x).
#define LOGO_PWM_CHANNEL   0
#define LOGO_PWM_FREQ      5000
#define LOGO_PWM_RES       8

// Optischer LED-Selbsttest beim Start (0 = aus).
#define ENABLE_SELFTEST    1
#define SELFTEST_ROUNDS    1

// Betriebsmodi. Die Zahlenwerte sind die Modus-IDs im App-Protokoll und muessen
// mit web_page.h/web_page_demo.html uebereinstimmen. MODE_LAST begrenzt die
// gueltigen Werte in onWebSocketEvent().
enum OperatingMode {
    MODE_OFF       = 0,   // alles aus (Controller bleibt erreichbar)
    MODE_STATIC    = 1,   // feste Helligkeit je Bereich
    MODE_EFFECT    = 2,   // Lauflicht
    MODE_AUTOMATIC = 3,   // tageszeitabhaengig (Standardmodus)
    MODE_PULSE     = 4,   // schnelles Pulsieren
    MODE_BREATH    = 5,   // langsames Atmen

    // Stimmungs-Modi mit festen Werten, Nachtabschaltung greift.
    MODE_DAUERLICHT   = 6,
    MODE_KERZENLICHT  = 7,
    MODE_STUFENLICHT  = 8,
    MODE_DAEMMERLICHT = 9,
    MODE_WAVE         = 10,  // raeumliche Sinuswelle
    MODE_FEUERSCHEIN  = 11,
    MODE_NACHTLICHT   = 12,
    MODE_LAST         = MODE_NACHTLICHT
};

// Feste Parameter der Stimmungs-Modi (8-Bit-Helligkeitsgrenzen, Perioden bzw.
// Flacker-Geschwindigkeiten). Werden von den render*-Funktionen in main.cpp
// genutzt; MIN/MAX begrenzen die Helligkeit, PERIOD/SPEED die Dynamik.

// Dauerlicht: ruhiges Atmen auf hohem Niveau.
#define DAUER_MIN            186
#define DAUER_MAX            208
#define DAUER_PERIOD_MS      18000UL

// Kerzenlicht: sanftes, unabhaengiges Flackern beider Segmente.
#define KERZEN_MIN       108
#define KERZEN_MAX       172
#define KERZEN_SPEED_DIV 20
#define KERZEN_LOGO      100

// Stufenlicht: Aufbau in acht Schritten (STEP) mit Haltezeit (HOLD).
#define STUFEN_LEVEL     220
#define STUFEN_STEP_MS   1500
#define STUFEN_HOLD_MS   3000

// Daemmerlicht: sehr gedaempftes, langsames Schwingen; Logo anteilig (LOGO_PCT).
#define DAEMMER_MIN       14
#define DAEMMER_MAX       46
#define DAEMMER_PERIOD_MS 150000UL
#define DAEMMER_LOGO_PCT  60

// Feuerschein: kraeftiges, ruhiges Lodern.
#define FEUER_MIN        150
#define FEUER_MAX        230
#define FEUER_SPEED_DIV  12
#define FEUER_LOGO       190

// Nachtlicht: ruhige, niedrige Kerze.
#define NACHT_MIN          30
#define NACHT_MAX          54
#define NACHT_SPEED_DIV    28
#define NACHT_LOGO         44
