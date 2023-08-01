#include "generalUtilsPS.h"

using namespace generalUtilsPS;

//Adjusts the passed in pattern's length to match the sizeNeeded
//If the existing pattern isn't large enough a new array will be created for the pattern.
//!!!!Make sure you free the array after you are done with the pattern by calling free(pattern.patternArr).
//Note that the function sets the length of the pattern,
//but it will ONLY re-size the pattern array if needs to be larger, or if alwaysResizeObjPS is true (see pixelSpork.h)
//If the pattern is re-sized, its new memory length will be recorded in the pattern's maxLength property
void generalUtilsPS::resizePattern(patternPS &pattern, uint16_t sizeNeeded){

    //if the pattern array doesn't have enough memory, create a new pattern array dynamically 
    if( alwaysResizeObjPS || (sizeNeeded > pattern.maxLength) ){
        free(pattern.patternArr);
        uint8_t *pattern_arr = (uint8_t*) malloc( sizeNeeded * sizeof(uint8_t) );
        pattern.patternArr = pattern_arr; 
        pattern.maxLength = sizeNeeded;
    }

    //Adjust the length of the pattern match the size needed
    //This "hides" any extra pattern entries from the rest of the code
    pattern.length = sizeNeeded;
}

//Sets the pattern to match the passed in palette
//ex: for palette length 5, the output patternArr would be {0, 1, 2, 3, 4}
//ie all the colors in the palette in order
//Note, if the pattern is not large enough to store the palette pattern, it will be re-sized dynamically.
//!!!Because of this, make sure you free() the patternArr when you're done by calling free(pattern.patternArr).
void generalUtilsPS::setPaletteAsPattern(patternPS &pattern, palettePS &palette){
    uint8_t patternLength = palette.length;
    
    //Adjust the pattern size if needed to store the palette pattern
    resizePattern(pattern, patternLength);

    for(uint16_t i = 0; i < patternLength; i++){
       pattern.patternArr[i] = i;
    }
}

//sets the pattern to match the passed in palette, with each pattern color being colorLength in length
//ex: for palette length 5, colorLength 2, the output patternArr would be {0, 0, 1, 1, 2, 2, 3, 3, 4, 4}
//ie all the colors in the palette in order, in colorLength lengths
//Note, if the pattern is not large enough to store the palette pattern, it will be re-sized dynamically.
//!!!Because of this, make sure you free() the patternArr when you're done by calling free(pattern.patternArr).
void generalUtilsPS::setPaletteAsPattern(patternPS &pattern, palettePS &palette, uint16_t colorLength){

    uint16_t patternLength = palette.length * colorLength;
    
    //Adjust the pattern size if needed to store the palette pattern
    resizePattern(pattern, patternLength);

    for(uint16_t i = 0; i < patternLength; i++){
        //We want i to increment every colorLength steps, so we divide by colorLength 
        //(taking advantage that integer division always rounds down)
        pattern.patternArr[i] = i / colorLength;
    }
}

//Sets the passed in palette to be the a pattern (using all colors in the palette) using the 
//passed in colorLength and spacing
//ex: for palette of length 3, and a colorLength of 2, and spacing of 1
//the final streamer pattern would be : {0, 0, 255, 1, 1, 255, 2, 2, 255}.
//255 is used for the spaces, allowing them to be filled with background color.
//(Note that only a few effects actually recognize 255 as being background, but they're mostly the effects
//where you'd want to use this sort of extended pattern anyway)
//Note, if the pattern is not large enough to store the palette pattern, it will be re-sized dynamically.
//!!!Because of this, make sure you free() the patternArr when you're done by calling free(pattern.patternArr).
void generalUtilsPS::setPaletteAsPattern(patternPS &pattern, palettePS &palette, uint8_t colorLength, uint8_t spacing){
   
    uint8_t repeatLength = (colorLength + spacing);
    uint8_t paletteLength = palette.length;
    uint16_t totalPatternLength = paletteLength * repeatLength; //the total length taken up by a single color and spacing
    
    //Adjust the pattern size if needed to store the palette pattern
    resizePattern(pattern, totalPatternLength);

    //for each color in the palette, we fill in the color and spacing for the output pattern
    for(uint16_t i = 0; i < paletteLength; i++){
        //for each color in the palette we run over the length of the color and spacing
        //for the indexes up to color length, we set them as the current palette index
        //after that we set them as spacing (255)
        for(uint8_t j = 0; j < repeatLength; j++){
            if(j < colorLength){
                //we do i*repeatLength to account for how many color sections we've 
                //filled in already
                pattern.patternArr[i * repeatLength + j] = i;
            } else {
                pattern.patternArr[i * repeatLength + j] = 255;
            }
        }
    }
}

//Takes a simple passed in pattern (of palette color indexes) 
//and creates a new pattern with lengths of color and spaces.
//This mostly useful for creating patterns for the Streamer and SegWave effects.
//ex : inputPattern is {1, 2, 4} with color length 2, and 1 spacing
//the streamer pattern would be: {1, 1, 255, 2, 2, 255, 4, 4, 255}.
//255 is used for the spaces, allowing them to be filled with background color.
//(Note that only a few effects actually recognize 255 as being background, but they're mostly the effects
//where you'd want to use this sort of extended pattern anyway)
//Note, if the pattern is not large enough to store the palette pattern, it will be re-sized dynamically.
//!!!Because of this, make sure you free() the patternArr when you're done by calling free(pattern.patternArr).
void generalUtilsPS::setPatternAsPattern(patternPS &outputPattern, patternPS &inputPattern, uint8_t colorLength, uint8_t spacing){
    
    uint8_t patternIndex;
    uint8_t repeatLength = (colorLength + spacing); //the total length taken up by a single color and spacing
    uint16_t patternLength = inputPattern.length;
    uint16_t totalPatternLength = patternLength * repeatLength; 
    
    //Adjust the pattern size if needed to store the palette pattern
    resizePattern(outputPattern, totalPatternLength);

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
                outputPattern.patternArr[i * repeatLength + j] = patternIndex;
            } else {
                outputPattern.patternArr[i * repeatLength + j] = 255;
            }
        }
    }
}


