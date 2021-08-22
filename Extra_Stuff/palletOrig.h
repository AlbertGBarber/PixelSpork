#ifndef pallet_h
#define pallet_h

#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
  #include "pins_arduino.h"
  #include "WConstants.h"
#endif

#include "FastLED.h"
#include "segDrawUtils.h"

class Pallet {

    public:
        Pallet(struct CRGB *PalletColors, uint8_t palletLength );
    void 
        setColor(CRGB color, uint8_t index),
        randomize();

    uint8_t
        length;
    
    CRGB
        *palletColors,
        getPalletColor(uint8_t index),
        crossFadePallet(uint8_t startIndex, uint8_t endIndex, uint8_t step, uint8_t totalSteps);

};

#endif