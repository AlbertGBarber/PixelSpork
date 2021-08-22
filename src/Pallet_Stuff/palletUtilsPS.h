#ifndef palletUtilsPS_h
#define palletUtilsPS_h

#include "palletPS.h"
#include "SegmentFiles.h"

namespace palletUtilsPS{
    void
        setColor(palletPS pallet, CRGB color, uint8_t index),
        randomize(palletPS pallet);
    
    CRGB 
        getPalletColor(palletPS pallet, uint8_t index),
        crossFadePallet(palletPS pallet, uint8_t startIndex, uint8_t endIndex, uint8_t step, uint8_t totalSteps);
};

#endif