#include "generalUtilsPS.h"

using namespace generalUtilsPS;

//sets the pattern to match the passed in palette
//ex: for palette length 5, the output patternArr would be {0, 1, 2, 3, 4}
//ie all the colors in the palette in order
//!!Make sure you free() the patternArr when you're done with the palette since it is created with malloc()
patternPS generalUtilsPS::setPaletteAsPattern(palettePS &palette){
    patternPS newPattern;
    uint8_t patternLength = palette.length;
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
patternPS generalUtilsPS::setPaletteAsPattern(palettePS &palette, uint16_t colorLength){
    patternPS newPattern;
    uint16_t patternLength = palette.length * colorLength;
    uint8_t *pattern_arr = (uint8_t*) malloc( patternLength * sizeof(uint8_t) );
    for(uint16_t i = 0; i < patternLength; i++){
        //We want i to increment every colorLength steps, so we divide by colorLength 
        //(taking advantage that interger division alaways rounds down)
        pattern_arr[i] = i / colorLength;
    }
    newPattern = {pattern_arr, patternLength};
    return newPattern;
}

//Takes a simple passed in pattern (of palette color indexes) 
//and creates a new pattern with lengths of color and spaces.
//This mostly useful for creating patterns for the Streamer and SegWave effects.
//ex : inputPattern is {1, 2, 4} with color length 2, and 1 spacing
//the streamer pattern would be: {1, 1, 255, 2, 2, 255, 4, 4, 255}.
//255 is used for the spaces, allowing them to be filled with background color.
//(Note that only a few effects actually recognize 255 as being background, but they're mostly the effects
//where you'd want to use this sort of extended pattern anyway)
//!!Make sure you free() the patternArr when you're done with the palette since it is created with malloc()
patternPS generalUtilsPS::setPatternAsPattern(patternPS &inputPattern, uint8_t colorLength, uint8_t spacing){
    patternPS newPattern;
    uint8_t patternIndex;
    uint8_t repeatLength = (colorLength + spacing); //the total length taken up by a single color and spacing
    uint16_t patternLength = inputPattern.length;
    uint16_t totalPatternLength = patternLength * repeatLength; 
    //create new storage for the pattern array
    uint8_t *pattern_arr = (uint8_t*) malloc(totalPatternLength*sizeof(uint8_t));

    //for each color in the inputPattern, we fill in the color and spacing for the output pattern
    for(uint16_t i = 0; i < patternLength; i++){
        patternIndex = patternUtilsPS::getPatternVal(inputPattern, i);
        //for each color in the pattern we run over the length of the color and spacing
        //for the indexes up to color length, we set them as the current patternIndex
        //after that we set them as spacing (255)
        for(uint8_t j = 0; j < repeatLength; j++){
            if(j < colorLength){
                //we do i*repeatLength to account for how many color sections we've 
                //filled in already
                pattern_arr[i * repeatLength + j] = patternIndex;
            } else {
                pattern_arr[i * repeatLength + j] = 255;
            }
        }
    }

    newPattern = {pattern_arr, totalPatternLength};
    return newPattern;
}

//Sets the passed in palette to be the a pattern (using all colors in the palette) using the 
//passed in colorLength and spacing
//ex: for palette of length 3, and a colorLength of 2, and spacing of 1
//the final streamer pattern would be : {0, 0, 255, 1, 1, 255, 2, 2, 255}.
//255 is used for the spaces, allowing them to be filled with background color.
//(Note that only a few effects actually recognize 255 as being background, but they're mostly the effects
//where you'd want to use this sort of extended pattern anyway)
//!!Make sure you free() the patternArr when you're done with the palette since it is created with malloc()
patternPS generalUtilsPS::setPaletteAsPattern(palettePS &palette, uint8_t colorLength, uint8_t spacing){
    patternPS newPattern;
    uint8_t repeatLength = (colorLength + spacing);
    uint8_t palettelength = palette.length;
    uint16_t totalPatternLength = palettelength * repeatLength; //the total length taken up by a single color and spacing
    //create new storage for the pattern array
    uint8_t *pattern_arr = (uint8_t*) malloc(totalPatternLength*sizeof(uint8_t));

    //for each color in the palette, we fill in the color and spacing for the output pattern
    for(uint16_t i = 0; i < palettelength; i++){
        //for each color in the palette we run over the length of the color and spacing
        //for the indexes up to color length, we set them as the current palette index
        //after that we set them as spacing (255)
        for(uint8_t j = 0; j < repeatLength; j++){
            if(j < colorLength){
                //we do i*repeatLength to account for how many color sections we've 
                //filled in already
                pattern_arr[i * repeatLength + j] = i;
            } else {
                pattern_arr[i * repeatLength + j] = 255;
            }
        }
    }

    newPattern = {pattern_arr, totalPatternLength};
    return newPattern;
}

