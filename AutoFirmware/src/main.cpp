#include <Arduino.h>
#include "grad2rad.h"
#include "rad2grad.h"
#include "JahresTag.h"
#include "MEZ.h"
#include "MOZ.h"
#include "StundenWinkel.h"
#include "Zeitgleichung.h"
#include "Deklination.h"
#include "HoehenWinkel.h"
#include "Azimut.h"
#include "SonnenStand.h"

void setup() {
    Serial.begin(115200);

    // grad2rad
    Serial.println(grad2rad(0.0f), 7);             // 0.0000000
    Serial.println(grad2rad(45.0f), 7);            // 0.7853982
    Serial.println(grad2rad(90.0f), 7);            // 1.5707963
    Serial.println(grad2rad(180.0f), 7);           // 3.1415927

    // rad2grad
    Serial.println(rad2grad(1.0f), 7);             // 57.2957763
    Serial.println(rad2grad(grad2rad(45.0f)), 7);  // 45.0000000

    // JahresTag
    Serial.println(JahresTag(1, 1));               // 1
    Serial.println(JahresTag(3, 1));               // 60
    Serial.println(JahresTag(10, 26));             // 299
    Serial.println(JahresTag(12, 31));             // 365

    // MEZ
    Serial.println(MEZ(12, 0, 0));                 // 720
    Serial.println(MEZ(13, 0, 1));                 // 720
    Serial.println(MEZ(15, 30, 0));                // 930
    Serial.println(MEZ(0, 30, 1));                 // -30

    // MOZ
    Serial.println(MOZ(720, 15.0f), 4);            // 720.0000  (on the 15 deg line: no shift)
    Serial.println(MOZ(720, 9.66f), 4);            // 698.6400
    Serial.println(MOZ(930, 9.66f), 4);            // 908.6400
    Serial.println(MOZ(MEZ(12, 0, 0), 9.66f), 4);  // 698.6400  (MEZ -> MOZ chained)

    // StundenWinkel
    Serial.println(StundenWinkel(720.0f), 4);      // 0.0000    noon
    Serial.println(StundenWinkel(540.0f), 4);      // 45.0000   09:00
    Serial.println(StundenWinkel(900.0f), 4);      // -45.0000  15:00
    Serial.println(StundenWinkel(0.0f), 4);        // 180.0000  midnight
    Serial.println(StundenWinkel(698.64f), 4);     // 5.3400    our MOZ at 12:00 MEZ

    // Zeitgleichung
    Serial.println(Zeitgleichung(0.0f), 4);                    // ~ -2.7749   1 Jan
    Serial.println(Zeitgleichung(360.0f), 4);                  // ~ -2.7749   same as 0 (one full year)
    Serial.println(Zeitgleichung(300.0f / 365.0f * 360.0f), 4); // ~ 16.2271  27 Oct (today)

    // Deklination
    Serial.println(Deklination(1), 4);     // 1 Jan
    Serial.println(Deklination(80), 4);    // ~21 Mar, should be near 0
    Serial.println(Deklination(172), 4);   // ~21 Jun, should be near +23.4
    Serial.println(Deklination(355), 4);   // ~21 Dec, should be near -23.4
    Serial.println(Deklination(300), 4);   // 27 Oct (today)

    // HoehenWinkel
    Serial.println(HoehenWinkel(0.0f, 47.81f, 23.4437f), 4);   // noon, June: ~65.6
    Serial.println(HoehenWinkel(0.0f, 47.81f, -23.4139f), 4);  // noon, December: ~18.8
    Serial.println(HoehenWinkel(90.0f, 47.81f, 0.0f), 4);      // 6h before noon, equinox: ~0 (horizon)
    Serial.println(HoehenWinkel(StundenWinkel(MOZ(MEZ(12, 0, 0), 9.66f)
                   + Zeitgleichung(300.0f / 365.0f * 360.0f)),
                   47.81f, Deklination(300)), 4);              // today 12:00 MEZ, full chain

    // Azimut
    Serial.println(Azimut(300, 47.81f, 540.0f), 4);   // 27 Oct, 09:00 WOZ: south-east
    Serial.println(Azimut(300, 47.81f, 720.0f), 4);   // 27 Oct, exactly noon: 180 (guard test)
    Serial.println(Azimut(300, 47.81f, 900.0f), 4);   // 27 Oct, 15:00 WOZ: south-west
    Serial.println(Azimut(172, 47.81f, 720.0f), 4);   // June, noon: 180 (guard test)
    Serial.println(Azimut(300, 47.81f, MOZ(MEZ(12, 0, 0), 9.66f)
                   + Zeitgleichung(300.0f / 365.0f * 360.0f)), 4);  // today 12:00 MEZ, full chain

    // SonnenStand
    float a, g;

    SonnenStand(47.81f, 9.66f, 26, 10, 12, 0, 0, a, g);   // 26 Oct 12:00, ref: 178.53 / 29.96
    Serial.print(a, 4); Serial.print("  "); Serial.println(g, 4);

    SonnenStand(47.81f, 9.66f, 26, 10, 15, 0, 0, a, g);   // 26 Oct 15:00, ref: 225.41 / 18.53
    Serial.print(a, 4); Serial.print("  "); Serial.println(g, 4);

    SonnenStand(47.81f, 9.66f, 27, 10, 12, 0, 0, a, g);   // today 12:00, should match your chain tests
    Serial.print(a, 4); Serial.print("  "); Serial.println(g, 4);

    SonnenStand(47.81f, 9.66f, 21, 6, 13, 0, 1, a, g);    // 21 Jun 13:00 summer time
    Serial.print(a, 4); Serial.print("  "); Serial.println(g, 4);
}

void loop() {}