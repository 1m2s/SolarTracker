#include "JahresTag.h"

// From MATLAB: d = Tag + sum of MTage(1..Monat-1)
int JahresTag(int Monat, int Tag)
{
    const int MTage[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int d = Tag;

    if (Monat > 1) {
        for (int i = 0; i < Monat - 1; i++) {
            d = d + MTage[i];
        }
    }
    return d;
}

/* Notes
 * - MATLAB indexes from 1, C++ from 0: MTage(1) -> MTage[0].
 * - Leap years ignored, same as the original (Feb = 28).
 * - The if is redundant (loop runs 0 times when Monat = 1),
 *   kept to mirror MATLAB 1:1.
 * - Returns int: SonnenStand must use d/365.0f, not d/365.
 */