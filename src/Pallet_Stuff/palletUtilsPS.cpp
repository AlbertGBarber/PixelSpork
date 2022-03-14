#include "palletUtilsPS.h"

//TODO: add blendColorFromPallet where you just pass in a step (total steps assumed to be 255) and it works out how far in the pallet you should be
using namespace palletUtilsPS;

//sets the pallet color at the specified index
//the index wraps, so running off the end of the pallet, will put you back at the start
void palletUtilsPS::setColor(palletPS *pallet, CRGB color, uint8_t index){
    pallet->palletArr[ index % pallet->length ] = color;
}

//randomizes all the colors in a pallet
void palletUtilsPS::randomize(palletPS *pallet){
    for(int i = 0; i < pallet->length; i++){
        randomize(pallet, i);
    }
}

//randomize the color in pallet at the specified index
void palletUtilsPS::randomize(palletPS *pallet, uint8_t index){
    setColor(pallet, segDrawUtils::randColor(), index);
}

//randomizes the order of colors in a pallet
void palletUtilsPS::shuffle(palletPS *pallet){
    CRGB randColor, tempColor;
    uint8_t randIndex;
    uint8_t loopStart = pallet->length - 1;
    // Start from the last element and swap
    // one by one. We don't need to run for
    // the first element that's why i > 0
    for(int i = loopStart; i > 0; i--){
        // Pick a random index from 0 to i
        randIndex = random(i + 1);
        // Swap arr[i] with the element
        // at random index
        tempColor = getPalletColor(pallet, i);
        randColor = getPalletColor(pallet, randIndex);
        setColor(pallet, tempColor, randIndex);
        setColor(pallet, randColor, i);
    }
}

//returns the color at a specified index
//the index wraps, so running off the end of the pallet, will put you back at the start
CRGB palletUtilsPS::getPalletColor(palletPS *pallet, uint8_t index){
    return pallet->palletArr[ index % pallet->length ];
}

//returns a pointer to the color in the pallet array at the specified index (wrapping if needed)
//useful for background color syncing
//Note: maybe remove the wrapping?, 
CRGB* palletUtilsPS::getColorPtr(palletPS *pallet, uint8_t index){
    return &(pallet->palletArr[ index % pallet->length ]);
}

//returns the blended result of two pallet colors
CRGB palletUtilsPS::getBlendedPalletColor(palletPS *pallet, uint8_t startIndex, uint8_t endIndex, uint8_t step, uint8_t totalSteps){

    return segDrawUtils::getCrossFadeColor(getPalletColor(pallet, startIndex), getPalletColor(pallet, endIndex), step, totalSteps);
    
}