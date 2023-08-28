#ifndef Fire2012SegUtilsPS_h
#define Fire2012SegUtilsPS_h

#if ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WConstants.h"
    #include "WProgram.h"
    #include "pins_arduino.h"
#endif

#include "Include_Lists/PaletteFiles.h"
#include "ColorUtils/colorUtilsPS.h"

//common functions for Fire2012 (and possibly other) effects
namespace fire2012SegUtilsPS {

    CRGB
        getPixelHeatColorPalette(palettePS *palette, uint8_t paletteLength, uint8_t paletteSecLen, CRGB *bgColor, uint8_t temperature, bool blend);

    static uint8_t
        secHeatLimit,
        palettelength,
        colorIndex;

    static bool
        doBg;

    static CRGB
        targetColor,
        startColor;

}
#endif
