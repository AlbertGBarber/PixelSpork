#include "pallet.h"

Pallet::Pallet(struct CRGB *PalletColors, uint8_t palletLength ) :
    length(palletLength), palletColors(PalletColors)

    {

    }

void Pallet::setColor(CRGB color, uint8_t index){
    palletColors[index % length] = color;
}

void Pallet::randomize(){
    for(int i = 0; i < length; i++){
        setColor(segDrawUtils::randColor(), i);
    }
}

CRGB Pallet::getPalletColor(uint8_t index){
    return palletColors[index % length];
}

CRGB Pallet::crossFadePallet(uint8_t startIndex, uint8_t endIndex, uint8_t step, uint8_t totalSteps){
    CRGB startColor = getPalletColor(startIndex);
    CRGB endColor = getPalletColor(endIndex);
    return segDrawUtils::getCrossFadeColor(startColor, endColor, step, totalSteps);
}