#include "EffectUtilsPS.h"

using namespace EffectUtilsPS;

//returns a pallet of length 1 containing the passed in color
//!!Make sure you delete the palletArr when you're done with the pallet
//since it is created with new
palletPS EffectUtilsPS::makeSingleColorpallet(CRGB Color){
    palletPS newPallet;
    CRGB *newPallet_arr = new CRGB[1];
    newPallet_arr[0] = Color;
    newPallet = {newPallet_arr, 1};
    return newPallet;
}

//returns a pallet of the specified length full of random colors
//!!Make sure you delete the palletArr when you're done with the pallet
//since it is created with new
palletPS EffectUtilsPS::makeRandomPallet(uint8_t length){
    palletPS newPallet;
    CRGB *newPallet_arr = new CRGB[length];
    for(uint8_t i = 0; i < length; i++){
        newPallet_arr[i] = segDrawUtils::randColor();
    }                                 
    newPallet = {newPallet_arr, length};
    return newPallet;
}

//sets the pattern to match the passed in pallet
//ex: for pallet length 5, the output patternArr would be {0, 1, 2, 3, 4}
//ie all the colors in the pallet in order
//!!Make sure you delete the patternArr when you're done with the pallet
//since it is created with new
patternPS EffectUtilsPS::setPalletAsPattern(palletPS *pallet){
    patternPS newPattern;
    uint8_t patternLength = pallet->length;
    uint8_t *pattern_arr = new uint8_t[patternLength];
    for(int i = 0; i < patternLength; i++){
        pattern_arr[i] = i;
    }
    newPattern = {pattern_arr, patternLength};
    return newPattern;
}