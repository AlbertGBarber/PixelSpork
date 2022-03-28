#ifndef patternUtilsPS_h
#define patternUtilsPS_h

//TODO
//Add direction to patterns? Ie you can set them to be read backwards or forwards
#include "FastLED.h"
#include "patternPS.h"
#include "MathUtils/mathUtilsPS.h"

//series of utility functions for interacting with patterns
//use these to change patterns
//!!DO NOT try to patterns without using these functions
namespace patternUtilsPS{
    void
        setVal(patternPS *pattern, uint8_t val, uint16_t index),
        shuffle(patternPS *pattern);
    
    uint8_t 
        getPatternVal(patternPS *pattern, uint16_t index),
        getRandVal(patternPS *pattern),
        *getValPtr(patternPS *pattern, uint16_t index),
        getShuffleIndex(patternPS *pattern, uint8_t currentPatternVal);
    
};

#endif