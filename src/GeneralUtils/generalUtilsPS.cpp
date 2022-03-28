#include "generalUtilsPS.h"

using namespace generalUtilsPS;

//sets the pattern to match the passed in pallet
//ex: for pallet length 5, the output patternArr would be {0, 1, 2, 3, 4}
//ie all the colors in the pallet in order
//!!Make sure you delete the patternArr when you're done with the pallet
//since it is created with new
patternPS generalUtilsPS::setPalletAsPattern(palletPS *pallet){
    patternPS newPattern;
    uint8_t patternLength = pallet->length;
    uint8_t *pattern_arr = new uint8_t[patternLength];
    for(int i = 0; i < patternLength; i++){
        pattern_arr[i] = i;
    }
    newPattern = {pattern_arr, patternLength};
    return newPattern;
}
