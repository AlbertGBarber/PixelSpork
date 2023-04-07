#ifndef colorUtilsPS_h
#define colorUtilsPS_h

//TODO: add color dim function (use crossFadeColor with 0 as target)

#include "FastLED.h"
#include "MathUtils/mathUtilsPS.h"

//Various functions for manipulating colors, see function comments in the .cpp file for info on each function
namespace colorUtilsPS{
    
    CRGB
        randColor(),
        wheel(uint16_t wheelPos, uint16_t offset, uint8_t satur, uint8_t value ),
        wheel(uint16_t wheelPos, uint16_t rainbowOffset),
        getCrossFadeColor(const CRGB &startColor, const CRGB &endColor, uint8_t blendStep, uint8_t totalSteps),
        getCrossFadeColor(const CRGB &startColor, const CRGB &endColor, uint8_t ratio),
        dimColor(const CRGB &startColor, uint8_t ratio);
    
    uint8_t
        getCrossFadeColorComp(uint8_t startColor, uint8_t endColor, uint8_t ratio);
    
    //pre-allocated variables
    static uint8_t
        ratio;
    
    static CRGB
        black = CRGB(0,0,0),
        colorFinal;

};

#endif