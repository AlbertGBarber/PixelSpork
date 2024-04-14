#ifndef colorUtilsPS_h
#define colorUtilsPS_h

//TODO: add color dim function (use crossFadeColor with 0 as target)

#include "FastLED.h"
#include "MathUtils/mathUtilsPS.h"

//Various functions for manipulating colors, see function comments in the .cpp file for info on each function
//Note that randSatMin and randSatMax control the saturation range for random colors.
//Their defaults produce a good mix of colors, trending towards the pastel range,
//but you may adjust them to your liking. 
namespace colorUtilsPS {

    CRGB
        randColor(),
        randColor(uint8_t satMin, uint8_t satMax, uint8_t valMin, uint8_t valMax),
        getCompColor(uint8_t baseHue, uint8_t numColors, uint8_t num, uint8_t sat, uint8_t val),
        wheel(uint16_t hue, uint16_t hueOffset, uint8_t sat, uint8_t val),
        wheel(uint16_t hue, uint16_t hueOffset),
        getCrossFadeColor(const CRGB &startColor, const CRGB &endColor, uint8_t blendStep, uint8_t totalSteps),
        getCrossFadeColor(const CRGB &startColor, const CRGB &endColor, uint8_t ratio),
        dimColor(const CRGB &color, uint8_t ratio);

    uint8_t
        getCrossFadeColorComp(uint8_t startColor, uint8_t endColor, uint8_t ratio);

    //pre-allocated variables
    static uint8_t
        randSatMin = 100,
        randSatMax = 255,
        ratio;

};

#endif