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