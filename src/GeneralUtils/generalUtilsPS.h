#ifndef generalUtilsPS_h
#define generalUtilsPS_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WConstants.h"
#include "WProgram.h"
#include "pins_arduino.h"
#endif

#include "PalletFiles.h"
#include "PatternFiles.h"
#include "ColorUtils/colorUtilsPS.h"

//namespace for general functions that are used in multiple effects
//but don't fit into color or math catagories
//or require both pallet and patterns
namespace generalUtilsPS {

    patternPS
        setPalletAsPattern(palletPS *pallet);
};
#endif