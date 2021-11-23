#ifndef EffectUtilsPS_h
#define EffectUtilsPS_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WConstants.h"
#include "WProgram.h"
#include "pins_arduino.h"
#endif

#include "PalletFiles.h"
#include "PatternFiles.h"

namespace EffectUtilsPS {

    palletPS 
        makeSingleColorpallet(CRGB Color),
        makeRandomPallet(uint8_t length);
    
    patternPS
        setPalletAsPattern(palletPS *pallet);
};
#endif