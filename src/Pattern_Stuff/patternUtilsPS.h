#ifndef patternUtilsPS_h
#define patternUtilsPS_h

#include "FastLED.h"
#include "patternPS.h"
#include "MathUtils/mathUtilsPS.h"

//series of utility functions for interacting with patterns
//use these to change patterns
//!!DO NOT try to patterns without using these functions
namespace patternUtilsPS{
    void
        setVal(patternPS *pattern, uint8_t val, uint16_t index),
        shuffle(patternPS *pattern),
        reverse(patternPS *pattern);
    
    uint8_t 
        getPatternVal(patternPS *pattern, uint16_t index),
        getRandVal(patternPS *pattern),
        *getValPtr(patternPS *pattern, uint16_t index),
        getShuffleIndex(patternPS *pattern, uint8_t currentPatternVal);
    
    static uint8_t
        uint8One,
        uint8Two;
    
    static uint16_t
        uint16One,
        uint16Two;
    
};

#endif