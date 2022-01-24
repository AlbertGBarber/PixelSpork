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

//namespace for general functions that are used in multiple effects
namespace EffectUtilsPS {

    uint8_t 
        shuffleIndex(patternPS *pattern, uint8_t currentPatternVal);

    palletPS 
        makeSingleColorpallet(CRGB Color),
        makeRandomPallet(uint8_t length);
    
    patternPS
        setPalletAsPattern(palletPS *pallet);
};
#endif