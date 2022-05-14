#ifndef palletUtilsPS_h
#define palletUtilsPS_h

//TODO
//Add direction to pallets? Ie you can set them to be read backwards or forwards

#include "palletPS.h"
#include "ColorUtils/colorUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

//series of utility functions for interacting with pallets
//use these to change pallets
//!!DO NOT try to pallets without using these functions
namespace palletUtilsPS{
    void
        setColor(palletPS *pallet, CRGB color, uint8_t index),
        randomize(palletPS *pallet),
        randomize(palletPS *pallet, uint8_t index),
        shuffle(palletPS *pallet);
    
    CRGB 
        getBlendedPalletColor(palletPS *pallet, uint8_t startIndex, uint8_t endIndex, uint8_t step, uint8_t totalSteps),
        getPalletColor(palletPS *pallet, uint8_t index),
        getShuffleIndex(palletPS *pallet, CRGB currentPalletVal),
        *getColorPtr(palletPS *pallet, uint8_t index),
        getPalletGradColor(palletPS *pallet, uint16_t num, uint16_t offset, uint16_t totalLength),
        getPalletGradColor(palletPS *pallet, uint16_t num, uint16_t offset, uint16_t totalLength, uint16_t gradLength);
    
    palletPS
        makeSingleColorPallet(CRGB Color),
        makeSingleColorPallet(CRGB Color, uint8_t length),
        makeRandomPallet(uint8_t length);

    static uint8_t
        uint8One,
        uint8Two,
        uint8Three;
    
    static uint16_t
        uint16One,
        uint16Two;
    
    static CRGB
        colorOne,
        colorTwo;
};

#endif