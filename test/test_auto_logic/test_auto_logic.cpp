// Unit-Tests der Automatik-Logik (include/auto_logic.h).
// Auf dem PC (braucht einen C++-Compiler):  pio test -e native
// Auf dem angeschlossenen ESP32:            pio test -e esp32dev

#include <unity.h>
#include "auto_logic.h"

// Standard-Tagesprofil wie in config.h: 6 / 8 / 18 / 23 Uhr, 90 / 90 / 60 / 25 %.
static AutoProfile profil() {
    AutoProfile p;
    p.tMorning = 6;  p.tDay = 8;  p.tEvening = 18; p.tNight = 23;
    p.bMorning = 90; p.bDay = 90; p.bEveStart = 60; p.bEveEnd = 25;
    return p;
}

void setUp(void) {}
void tearDown(void) {}

// Nachts (23:00-06:00) ist alles aus.
void test_nacht_ist_aus(void) {
    AutoProfile p = profil();
    TEST_ASSERT_EQUAL_INT(0, autoBrightnessAt(0, p));             // Mitternacht
    TEST_ASSERT_EQUAL_INT(0, autoBrightnessAt(5 * 60 + 59, p));   // kurz vor 06:00
    TEST_ASSERT_EQUAL_INT(0, autoBrightnessAt(23 * 60, p));       // 23:00
    TEST_ASSERT_EQUAL_INT(0, autoBrightnessAt(23 * 60 + 30, p));  // 23:30
}

// Morgenrampe 06:00 -> 08:00: linear von 0 auf 90 %.
void test_morgen_rampe(void) {
    AutoProfile p = profil();
    TEST_ASSERT_EQUAL_INT(0,  autoBrightnessAt(6 * 60, p));   // 06:00 -> 0
    TEST_ASSERT_EQUAL_INT(45, autoBrightnessAt(7 * 60, p));   // 07:00 -> Haelfte
    TEST_ASSERT_EQUAL_INT(90, autoBrightnessAt(8 * 60, p));   // 08:00 -> voll (Tagbeginn)
}

// Tag 08:00 -> 18:00: konstant 90 %.
void test_tag_konstant(void) {
    AutoProfile p = profil();
    TEST_ASSERT_EQUAL_INT(90, autoBrightnessAt(12 * 60, p));
    TEST_ASSERT_EQUAL_INT(90, autoBrightnessAt(17 * 60 + 59, p));
}

// Abendrampe 18:00 -> 23:00: linear von 60 % auf 25 %.
void test_abend_rampe(void) {
    AutoProfile p = profil();
    TEST_ASSERT_EQUAL_INT(60, autoBrightnessAt(18 * 60, p));  // 18:00 -> 60
    TEST_ASSERT_EQUAL_INT(46, autoBrightnessAt(20 * 60, p));  // 20:00 -> 46
    TEST_ASSERT_EQUAL_INT(32, autoBrightnessAt(22 * 60, p));  // 22:00 -> 32
}

// Zeitfenster: 0 Nacht, 1 Morgen, 2 Tag, 3 Abend.
void test_zeitfenster(void) {
    AutoProfile p = profil();
    TEST_ASSERT_EQUAL_INT(0, autoWindowAt(0, p));            // Nacht
    TEST_ASSERT_EQUAL_INT(1, autoWindowAt(7 * 60, p));       // Morgen
    TEST_ASSERT_EQUAL_INT(2, autoWindowAt(12 * 60, p));      // Tag
    TEST_ASSERT_EQUAL_INT(3, autoWindowAt(20 * 60, p));      // Abend
    TEST_ASSERT_EQUAL_INT(0, autoWindowAt(23 * 60 + 30, p)); // wieder Nacht
}

// Prozent -> 8-Bit und Begrenzung 0..100.
void test_prozent_umrechnung(void) {
    TEST_ASSERT_EQUAL_INT(0,   pctTo8bit(0));
    TEST_ASSERT_EQUAL_INT(255, pctTo8bit(100));
    TEST_ASSERT_EQUAL_INT(204, pctTo8bit(80));
    TEST_ASSERT_EQUAL_INT(0,   pctTo8bit(-10));   // wird auf 0 begrenzt
    TEST_ASSERT_EQUAL_INT(255, pctTo8bit(150));   // wird auf 100 begrenzt
    TEST_ASSERT_EQUAL_INT(0,   clampPct(-5));
    TEST_ASSERT_EQUAL_INT(100, clampPct(200));
}

static int run_all(void) {
    UNITY_BEGIN();
    RUN_TEST(test_nacht_ist_aus);
    RUN_TEST(test_morgen_rampe);
    RUN_TEST(test_tag_konstant);
    RUN_TEST(test_abend_rampe);
    RUN_TEST(test_zeitfenster);
    RUN_TEST(test_prozent_umrechnung);
    return UNITY_END();
}

#ifdef ARDUINO
// Auf dem ESP32: Ergebnisse ueber die serielle Ausgabe (115200 Baud).
#include <Arduino.h>
void setup(void) { delay(1500); run_all(); }
void loop(void) {}
#else
// Auf dem PC.
int main(int, char **) { return run_all(); }
#endif
