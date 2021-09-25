#include "palletUtilsPS.h"

using namespace palletUtilsPS;

//sets the pallet color at the specified index
//the index wraps, so running off the end of the pallet, will put you back at the start
void palletUtilsPS::setColor(palletPS *pallet, CRGB color, uint8_t index){
    pallet->palletArr[ index % pallet->length ] = color;
}

//randomize the colors in a pallet
void palletUtilsPS::randomize(palletPS *pallet){
    for(int i = 0; i < pallet->length; i++){
        setColor(pallet, segDrawUtils::randColor(), i);
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

//blends one color of a pallet into another
//not sure why this exists, since it doesn't seem very useful
//commented out for posterity
// CRGB palletUtilsPS::crossFadePallet(palletPS *pallet, uint8_t startIndex, uint8_t endIndex, uint8_t step, uint8_t totalSteps){
//     CRGB startColor = getPalletColor(pallet, startIndex);
//     CRGB endColor = getPalletColor(pallet, endIndex);
//     return segDrawUtils::getCrossFadeColor(startColor, endColor, step, totalSteps);
// }