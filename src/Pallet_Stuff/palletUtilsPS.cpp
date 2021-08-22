#include "palletUtilsPS.h"

using namespace palletUtilsPS;

void palletUtilsPS::setColor(palletPS pallet, CRGB color, uint8_t index){
    pallet.palletArr[ index % pallet.length ] = color;
}

void palletUtilsPS::randomize(palletPS pallet){
    for(int i = 0; i < pallet.length; i++){
        setColor(pallet, segDrawUtils::randColor(), i);
    }
}

CRGB palletUtilsPS::getPalletColor(palletPS pallet, uint8_t index){
    return pallet.palletArr[ index % pallet.length ];
}

CRGB palletUtilsPS::crossFadePallet(palletPS pallet, uint8_t startIndex, uint8_t endIndex, uint8_t step, uint8_t totalSteps){
    CRGB startColor = getPalletColor(pallet, startIndex);
    CRGB endColor = getPalletColor(pallet, endIndex);
    return segDrawUtils::getCrossFadeColor(startColor, endColor, step, totalSteps);
}