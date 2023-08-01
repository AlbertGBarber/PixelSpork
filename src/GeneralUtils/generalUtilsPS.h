#ifndef generalUtilsPS_h
#define generalUtilsPS_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WConstants.h"
#include "WProgram.h"
#include "pins_arduino.h"
#endif

#include "./Include_Lists/GlobalVars/GlobalVars.h" //need alwaysResizeObjPS
#include "Include_Lists/PaletteFiles.h"
#include "Include_Lists/PatternFiles.h"
#include "ColorUtils/colorUtilsPS.h"

//namespace for general functions that are used in multiple effects
//but don't fit into color or math catagories
//or require both palette and patterns
namespace generalUtilsPS {

    void
        resizePattern(patternPS &pattern, uint16_t sizeNeeded),
        setPaletteAsPattern(patternPS &pattern, palettePS &palette),
        setPaletteAsPattern(patternPS &pattern, palettePS &palette, uint16_t colorLength),
        setPaletteAsPattern(patternPS &pattern, palettePS &palette, uint8_t colorLength, uint8_t spacing),
        setPatternAsPattern(patternPS &outputPattern, patternPS &inputPattern, uint8_t colorLength, uint8_t spacing);
};
#endif