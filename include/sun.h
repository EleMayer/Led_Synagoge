#pragma once
#include <math.h>

// ---------------------------------------------------------------------------
// Sonnenauf-/-untergang (Almanac-Algorithmus, 1990) - hardwareunabhaengig.
// Wird von der optionalen Sonnenstand-Automatik genutzt und ist per Unit-Test
// (test/) pruefbar. Rueckgabe von sunHour(): lokale Zeit in Stunden (0..24)
// oder -1, wenn es an dem Tag kein Ereignis gibt (Polartag/-nacht).
// ---------------------------------------------------------------------------

inline double _sunD2R(double d) { return d * M_PI / 180.0; }
inline double _sunR2D(double r) { return r * 180.0 / M_PI; }
inline double _sunNorm(double v, double max) {
    while (v < 0)    v += max;
    while (v >= max) v -= max;
    return v;
}

// Tag im Jahr (1..366).
inline int dayOfYear(int y, int m, int d) {
    static const int cum[12] = {0,31,59,90,120,151,181,212,243,273,304,334};
    int n = cum[m - 1] + d;
    bool leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
    if (leap && m > 2) n += 1;
    return n;
}

// Tag des letzten Sonntags im Monat (fuer die Sommerzeit-Regel).
inline int _lastSunday(int y, int m) {
    static const int mdays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int dim = mdays[m - 1];
    if (m == 2 && ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0))) dim = 29;
    static const int t[] = {0,3,2,5,0,3,5,1,4,6,2,4};   // Sakamoto
    int yy = y - (m < 3);
    int dow = (yy + yy/4 - yy/100 + yy/400 + t[m - 1] + dim) % 7;  // 0 = Sonntag
    return dim - dow;
}

// Europaeische Sommerzeit: letzter Sonntag im Maerz bis letzter Sonntag im Oktober.
inline bool isEuropeDST(int y, int m, int d) {
    if (m < 3 || m > 10) return false;
    if (m > 3 && m < 10) return true;
    int ls = _lastSunday(y, m);
    if (m == 3) return d >= ls;   // ab letztem Sonntag im Maerz
    return d < ls;                // vor letztem Sonntag im Oktober
}

// Sonnenauf- (sunset=false) bzw. -untergang (sunset=true) fuer Tag N im Jahr.
// lat/lng in Grad (Ost positiv), utcOffset in Stunden (z. B. 1 = MEZ, 2 = MESZ).
inline double sunHour(bool sunset, int N, double lat, double lng, double utcOffset) {
    const double zenith = 90.833;
    double lngHour = lng / 15.0;
    double t = sunset ? (N + ((18.0 - lngHour) / 24.0))
                      : (N + ((6.0  - lngHour) / 24.0));

    double M = (0.9856 * t) - 3.289;
    double L = M + (1.916 * sin(_sunD2R(M))) + (0.020 * sin(_sunD2R(2 * M))) + 282.634;
    L = _sunNorm(L, 360.0);

    double RA = _sunR2D(atan(0.91764 * tan(_sunD2R(L))));
    RA = _sunNorm(RA, 360.0);
    RA += (floor(L / 90.0) * 90.0) - (floor(RA / 90.0) * 90.0);   // gleiche Quadrant
    RA /= 15.0;

    double sinDec = 0.39782 * sin(_sunD2R(L));
    double cosDec = cos(asin(sinDec));
    double cosH = (cos(_sunD2R(zenith)) - (sinDec * sin(_sunD2R(lat))))
                  / (cosDec * cos(_sunD2R(lat)));
    if (cosH > 1.0 || cosH < -1.0) return -1.0;   // kein Auf-/Untergang an dem Tag

    double H = sunset ? _sunR2D(acos(cosH)) : (360.0 - _sunR2D(acos(cosH)));
    H /= 15.0;

    double T  = H + RA - (0.06571 * t) - 6.622;
    double UT = _sunNorm(T - lngHour, 24.0);
    return _sunNorm(UT + utcOffset, 24.0);
}
