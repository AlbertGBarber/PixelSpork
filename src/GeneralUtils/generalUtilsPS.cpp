#include "generalUtilsPS.h"

using namespace generalUtilsPS;

//sets the pattern to match the passed in palette
//ex: for palette length 5, the output patternArr would be {0, 1, 2, 3, 4}
//ie all the colors in the palette in order
//!!Make sure you delete the patternArr when you're done with the palette
//since it is created with new
patternPS generalUtilsPS::setPaletteAsPattern(palettePS *palette){
    patternPS newPattern;
    uint8_t patternLength = palette->length;
    uint8_t *pattern_arr = (uint8_t*) malloc( patternLength * sizeof(uint8_t) );
    for(int i = 0; i < patternLength; i++){
        pattern_arr[i] = i;
    }
    newPattern = {pattern_arr, patternLength};
    return newPattern;
}
