#ifndef generalUtilsPS_h
#define generalUtilsPS_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WConstants.h"
#include "WProgram.h"
#include "pins_arduino.h"
#endif

#include "PaletteFiles.h"
#include "PatternFiles.h"
#include "ColorUtils/colorUtilsPS.h"

//namespace for general functions that are used in multiple effects
//but don't fit into color or math catagories
//or require both palette and patterns
namespace generalUtilsPS {

    patternPS
        setPaletteAsPattern(palettePS &palette),
        setPaletteAsPattern(palettePS &palette, uint16_t colorLength),
        setPaletteAsPattern(palettePS &palette, uint8_t colorLength, uint8_t spacing),
        setPatternAsPattern(patternPS &inputPattern, uint8_t colorLength, uint8_t spacing);
};
#endif