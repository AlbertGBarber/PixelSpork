#include "generalUtilsPS.h"

using namespace generalUtilsPS;

//sets the pattern to match the passed in palette
//ex: for palette length 5, the output patternArr would be {0, 1, 2, 3, 4}
//ie all the colors in the palette in order
//!!Make sure you free() the patternArr when you're done with the palette since it is created with malloc()
patternPS generalUtilsPS::setPaletteAsPattern(palettePS *palette){
    patternPS newPattern;
    uint8_t patternLength = palette->length;
    uint8_t *pattern_arr = (uint8_t*) malloc( patternLength * sizeof(uint8_t) );
    for(uint16_t i = 0; i < patternLength; i++){
        pattern_arr[i] = i;
    }
    newPattern = {pattern_arr, patternLength};
    return newPattern;
}

//sets the pattern to match the passed in palette, with each pattern color being colorLength in length
//ex: for palette length 5, colorLength 2, the output patternArr would be {0, 0, 1, 1, 2, 2, 3, 3, 4, 4}
//ie all the colors in the palette in order, in colorLength lengths
//!!Make sure you free() the patternArr when you're done with the palette since it is created with malloc()
patternPS generalUtilsPS::setPaletteAsPattern(palettePS *palette, uint16_t colorLength){
    patternPS newPattern;
    uint16_t patternLength = palette->length * colorLength;
    uint8_t *pattern_arr = (uint8_t*) malloc( patternLength * sizeof(uint8_t) );
    for(uint16_t i = 0; i < patternLength; i++){
        //We want i to increment every colorLength steps, so we divide by colorLength 
        //(taking advantage that interger division alaways rounds down)
        pattern_arr[i] = i / colorLength;
    }
    newPattern = {pattern_arr, patternLength};
    return newPattern;
}

