#include "displayConfig.h"

uint8_t rgbMode = 0;
uint8_t rgbModeLoaded = 0;

// I needed to change these from RGB to RC (Red Color), BC, GC to prevent
// conflicting with the TFT_SPI Library.
const uint8_t rgbOrder[3 * 6] = {
    RC, BC, GC,  // rgbMode 0
    GC, RC, BC,  // rgbMode 1
    BC, GC, RC   // rgbMode 2
    RC, GC, BC,  // rgbMode 3
    BC, RC, GC,  // rgbMode 4
    GC, BC, RC,  // rgbMode 5
};
