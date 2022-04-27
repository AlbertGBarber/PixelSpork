#ifndef Fire2012SegUtilsPS_h
#define Fire2012SegUtilsPS_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WConstants.h"
#include "WProgram.h"
#include "pins_arduino.h"
#endif

#include "PalletFiles.h"
#include "ColorUtils/colorUtilsPS.h"

//common functions for Fire2012 (and possibly other) effects
namespace Fire2012SegUtilsPS {

    CRGB
        getPixelHeatColorPallet(palletPS *pallet, uint8_t palletLength, uint8_t palletSecLen, CRGB *bgColor, uint8_t temperature,  bool blend);

    static uint8_t
        secHeatLimit,
        palletlength,
        colorIndex;
    
    static bool
        doBg;
    
    static CRGB
        targetColor,
        startColor;

}
#endif

