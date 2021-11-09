#ifndef PixelSpork_h
#define PixelSpork_h

//#define FASTLED_INTERNAL
#include "FastLED.h"

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WConstants.h"
#include "WProgram.h"
#include "pins_arduino.h"
#endif

#include "UtilsList.h"

#include "SegmentFiles.h"

#include "PalletFiles.h"

#include "EffectsList.h"


//class PixelStrip2 {
//public:
    //PixelStrip2(uint16_t n, uint8_t p, uint8_t t);
    //~PixelStrip();  
    
//};

#endif