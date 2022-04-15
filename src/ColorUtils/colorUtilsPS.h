#ifndef colorUtilsPS_h
#define colorUtilsPS_h

#include "FastLED.h"
#include "MathUtils/mathUtilsPS.h"

namespace colorUtilsPS{
    
    CRGB
        randColor(),
        wheel(uint16_t wheelPos, uint16_t offset, uint8_t satur, uint8_t value ),
        wheel(uint16_t wheelPos, uint16_t rainbowOffset),
        getCrossFadeColor(CRGB startColor, CRGB endColor, uint8_t blendStep, uint8_t totalSteps),
        getCrossFadeColor(CRGB startColor, CRGB endColor, uint8_t ratio);
    
    uint8_t
        getCrossFadeColorComp(uint8_t startColor, uint8_t endColor, uint8_t ratio);
    
    static uint8_t
        ratio;
    
    static CRGB
        colorFinal;

};

#endif