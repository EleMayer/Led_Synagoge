// Unit-Tests der Sonnenstand-Logik (include/sun.h).
// Auf dem PC:  pio test -e native      Auf dem ESP32:  pio test -e esp32dev

#include <unity.h>
#include "sun.h"

void setUp(void) {}
void tearDown(void) {}

// Tag im Jahr.
void test_dayOfYear(void) {
    TEST_ASSERT_EQUAL_INT(1,   dayOfYear(2026, 1, 1));
    TEST_ASSERT_EQUAL_INT(172, dayOfYear(2026, 6, 21));
    TEST_ASSERT_EQUAL_INT(365, dayOfYear(2026, 12, 31));  // kein Schaltjahr
    TEST_ASSERT_EQUAL_INT(366, dayOfYear(2024, 12, 31));  // Schaltjahr
}

// Europaeische Sommerzeit (2026: letzter So Maerz = 29., letzter So Okt = 25.).
void test_dst(void) {
    TEST_ASSERT_TRUE(isEuropeDST(2026, 7, 1));    // Sommer
    TEST_ASSERT_FALSE(isEuropeDST(2026, 1, 1));   // Winter
    TEST_ASSERT_TRUE(isEuropeDST(2026, 3, 29));   // ab letztem So Maerz
    TEST_ASSERT_FALSE(isEuropeDST(2026, 3, 28));
    TEST_ASSERT_TRUE(isEuropeDST(2026, 10, 24));  // bis vor letztem So Okt
    TEST_ASSERT_FALSE(isEuropeDST(2026, 10, 25));
}

// Sonnenauf-/-untergang fuer Steyr (48.04 N, 14.42 O).
void test_sun_steyr(void) {
    double srSommer = sunHour(false, 172, 48.043, 14.420, 2.0);  // 21.06, MESZ
    double srWinter = sunHour(false, 355, 48.043, 14.420, 1.0);  // 21.12, MEZ
    double ssSommer = sunHour(true,  172, 48.043, 14.420, 2.0);

    TEST_ASSERT_TRUE(srSommer > 4.5 && srSommer < 5.5);   // ~05:02
    TEST_ASSERT_TRUE(srWinter > 7.3 && srWinter < 8.3);   // ~07:49
    TEST_ASSERT_TRUE(srSommer < srWinter);                // Sommer frueher hell
    TEST_ASSERT_TRUE(ssSommer > 20.5 && ssSommer < 21.5); // ~21:06
}

static int run_all(void) {
    UNITY_BEGIN();
    RUN_TEST(test_dayOfYear);
    RUN_TEST(test_dst);
    RUN_TEST(test_sun_steyr);
    return UNITY_END();
}

#ifdef ARDUINO
#include <Arduino.h>
void setup(void) { delay(1500); run_all(); }
void loop(void) {}
#else
int main(int, char **) { return run_all(); }
#endif
