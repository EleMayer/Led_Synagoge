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
    MODE_PULSE     = 4,   // ruhiges Pulsieren (fassadentauglich)
    MODE_BREATH    = 5,   // langsames Atmen

    // Stimmungs-Modi mit festen Werten, Nachtabschaltung greift.
    MODE_DAUERLICHT   = 6,
    MODE_KERZENLICHT  = 7,
    MODE_STUFENLICHT  = 8,
    MODE_DAEMMERLICHT = 9,
    MODE_WAVE         = 10,  // raeumliche Sinuswelle
    MODE_FEUERSCHEIN  = 11,
    MODE_NACHTLICHT   = 12,

    // Weitere Fassaden-Effekte (nutzen die Effekt-Helligkeit, keine Nachtabschaltung).
    MODE_STERNEN      = 13,  // Sternenfunkeln
    MODE_TREFFPUNKT   = 14,  // zwei Lichter treffen sich in der Mitte
    MODE_HERZSCHLAG   = 15,  // ruhiger Doppelschlag der ganzen Fassade
    MODE_WECHSEL      = 16,  // Segmente Links/Rechts gegenlaeufig
    MODE_LAST         = MODE_WECHSEL
};

// Feste Parameter der Stimmungs-Modi (8-Bit-Helligkeitsgrenzen, Perioden bzw.
// Flacker-Geschwindigkeiten). Werden von den render*-Funktionen in main.cpp
// genutzt; MIN/MAX begrenzen die Helligkeit, PERIOD/SPEED die Dynamik.
//
// FASSADEN-ABSTIMMUNG: Die Werte sind auf eine Aussenwand ausgelegt, die aus
// Distanz ruhig und gleichmaessig wirken soll - also enge Helligkeitsbereiche
// (wenig Kontrast), langsame Bewegungen (grosse PERIOD / grosser SPEED_DIV) und
// angehobene Grundhelligkeit (kein nervoeses Flackern, keine tiefen Dunkelphasen).

// Dauerlicht: gleichmaessiges, sehr langsam atmendes Licht auf hohem Niveau.
#define DAUER_MIN            190
#define DAUER_MAX            214
#define DAUER_PERIOD_MS      24000UL

// Kerzenlicht: langsames, wuerdevolles Schimmern (statt nervoesem Flackern).
#define KERZEN_MIN       165
#define KERZEN_MAX       200
#define KERZEN_SPEED_DIV 34
#define KERZEN_LOGO      175

// Stufenlicht: langsamer, ruhiger Aufbau in acht Schritten mit langer Haltezeit.
#define STUFEN_LEVEL     220
#define STUFEN_STEP_MS   2200
#define STUFEN_HOLD_MS   5000

// Daemmerlicht: sanfter, niedriger Abendglanz - noch aus Distanz sichtbar.
#define DAEMMER_MIN       55
#define DAEMMER_MAX       95
#define DAEMMER_PERIOD_MS 180000UL
#define DAEMMER_LOGO_PCT  70

// Feuerschein: warmes, ruhiges Lodern - langsam und wenig kontrastreich.
#define FEUER_MIN        175
#define FEUER_MAX        215
#define FEUER_SPEED_DIV  24
#define FEUER_LOGO       200

// Nachtlicht: ruhiger, niedriger Grundglanz (sehr langsam).
#define NACHT_MIN          55
#define NACHT_MAX          80
#define NACHT_SPEED_DIV    40
#define NACHT_LOGO         60

// Zusaetzliche Fassaden-Effekte. Ihre Helligkeit richtet sich nach der
// Effekt-Helligkeit (globalBrightness), die Prozentwerte sind Anteile davon.

// Sternenfunkeln: dezenter Grundglanz mit einzelnen, langsam verglimmenden Funken.
#define TWINKLE_CHANCE     16     // Wahrscheinlichkeit je Frame (0-255) fuer einen neuen Funken
#define TWINKLE_FADE       10     // Abkling-Geschwindigkeit der Funken
#define TWINKLE_BASE_PCT   18     // Grundglanz in % der Effekt-Helligkeit

// Herzschlag: zwei kurze Schlaege je Zyklus, dazwischen ruhiges Grundniveau.
#define HEART_PERIOD_MS    2200UL
#define HEART_LOW_PCT      25     // Grundniveau in % der Effekt-Helligkeit

// Wechsellicht: Segmente Links/Rechts schwellen langsam gegenlaeufig.
#define WECHSEL_PERIOD_MS  9000UL
#define WECHSEL_LOW_PCT    35     // dunklere Seite in % der Effekt-Helligkeit
