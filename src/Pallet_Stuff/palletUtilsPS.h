#ifndef palletUtilsPS_h
#define palletUtilsPS_h

#include "palletPS.h"
#include "SegmentFiles.h"

//series of utility functions for interacting with pallets
//use these to change pallets
//!!DO NOT try to pallets without using these functions
namespace palletUtilsPS{
    void
        setColor(palletPS *pallet, CRGB color, uint8_t index),
        randomize(palletPS *pallet);
    
    CRGB 
        //crossFadePallet(palletPS *pallet, uint8_t startIndex, uint8_t endIndex, uint8_t step, uint8_t totalSteps),
        getPalletColor(palletPS *pallet, uint8_t index),
        *getColorPtr(palletPS *pallet, uint8_t index);
    
    
};

#endif