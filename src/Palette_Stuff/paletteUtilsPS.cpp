#include "paletteUtilsPS.h"

//TODO: add blendColorFromPalette where you just pass in a step (total steps assumed to be 255) and it works out how far in the palette you should be
using namespace paletteUtilsPS;

//sets the palette color at the specified index
//the index wraps, so running off the end of the palette, will put you back at the start
void paletteUtilsPS::setColor(palettePS *palette, CRGB color, uint8_t index){
    palette->paletteArr[ mod8(index, palette->length) ] = color;
}

//returns the color at a specified index
//the index wraps, so running off the end of the palette, will put you back at the start
CRGB paletteUtilsPS::getPaletteColor(palettePS *palette, uint8_t index){
    return palette->paletteArr[ mod8(index, palette->length) ];
}

//returns a pointer to the color in the palette array at the specified index (wrapping if needed)
//useful for background color syncing
//Note: maybe remove the wrapping?, 
CRGB* paletteUtilsPS::getColorPtr(palettePS *palette, uint8_t index){
    return &(palette->paletteArr[ mod8(index, palette->length) ]);
}

//randomizes all the colors in a palette
void paletteUtilsPS::randomize(palettePS *palette){
    for(uint8_t i = 0; i < palette->length; i++){
        randomize(palette, i);
    }
}

//randomize the color in palette at the specified index
void paletteUtilsPS::randomize(palettePS *palette, uint8_t index){
    setColor(palette, colorUtilsPS::randColor(), index);
}

//Reverses the order of the palette colors
void paletteUtilsPS::reverse(palettePS *palette){
    uint8One = 0; //start index
    uint8Two = palette->length - 1; //end index
    
    //To reverse the palette we use a common algo
    //We basically swap pairs of colors, starting with the end and start of the array, and then moving inwards
    //Once we reach the center of the array, we stop
    while (uint8One < uint8Two){
        colorOne = getPaletteColor(palette, uint8One); //get the start index color
        colorTwo = getPaletteColor(palette, uint8Two); //get the end index color
        setColor(palette, colorTwo, uint8One); //set the end color into the start index
        setColor(palette, colorOne, uint8Two); //set the start color into the end index
        //move to the next pair of indexes
        uint8One++;
        uint8Two--;
    }
}

//randomizes the order of colors in a palette
void paletteUtilsPS::shuffle(palettePS *palette){
    uint8One = palette->length - 1; //loopStart
    // Start from the last element and swap
    // one by one. We don't need to run for
    // the first element that's why i > 0
    for(uint8_t i = uint8One; i > 0; i--){
        // Pick a random index from 0 to i
        uint8Two = random8(i + 1);
        // Swap arr[i] with the element
        // at random index
        colorOne = getPaletteColor(palette, i);
        colorTwo = getPaletteColor(palette, uint8Two); //random color
        setColor(palette, colorOne, uint8Two);
        setColor(palette, colorTwo, i);
    }
}

//retuns a random color from the palette
//the code checks to see if the random index matches the current color (passed in)
//if it does we'll just advance the index by one and return its color
//this stops the same color from being chosen again (assuming the palette doesn't repeat)
CRGB paletteUtilsPS::getShuffleIndex(palettePS *palette, CRGB &currentPaletteVal){
    uint8One = random8(palette->length); //guess an index
    colorOne = paletteUtilsPS::getPaletteColor(palette, uint8One); //get the color at the guess
    if( colorOne == currentPaletteVal ){
        return paletteUtilsPS::getPaletteColor(palette, uint8One + 1);
    } else {
        return colorOne;
    }
}

//returns the blended result of two palette colors
CRGB paletteUtilsPS::getBlendedPaletteColor(palettePS *palette, uint8_t startIndex, uint8_t endIndex, uint8_t step, uint8_t totalSteps){
    colorOne = getPaletteColor(palette, startIndex);
    colorTwo = getPaletteColor(palette, endIndex);
    return colorUtilsPS::getCrossFadeColor(colorOne, colorTwo, step, totalSteps); 
}

//returns a gradient color between palette colors based on several inputs: (see the next function below)
//This version of getPaletteGradColor does not require the gradLength and instead works it out for you
//Otherwise it's the same as the other version
CRGB paletteUtilsPS::getPaletteGradColor(palettePS *palette, uint16_t num, uint16_t offset, uint16_t totalLength) {

    //gradient steps per color in the palette
    //this is a uint16_t to allow for gradients > 255
    uint16One = totalLength / palette->length; //gradLength
    
    //divide by 0 protection
    if(!uint16One){
        uint16One = 1;
    }

    return getPaletteGradColor(palette, num, offset, totalLength, uint16One);
}

//returns a gradient color between palette colors based on several inputs:
//  num: The value of what you want to apply the gradient to (generally a pixel or segment number)
//  offset: Any offset of num (see segmentSet gradOffset)
//  totalLength: The length the entire palette gradient is spread across (usually the length of a segmentSet or similar
//  gradLength: The length of the gradient between the palette colors (ie totalLenth/paletteLength)
CRGB paletteUtilsPS::getPaletteGradColor(palettePS *palette, uint16_t num, uint16_t offset, uint16_t totalLength, uint16_t gradLength){
    //the actual gradient number we need based on the offset
    uint16One = addMod16PS( num, offset, totalLength ); //(num + offset) % totalLength;

    //the index of the palette color we're starting from (interger division always rounds down)
    uint8One = uint16One / gradLength; // (num + offset)/gradLength

    // get the crossfade step
    //uint8_t gradStep = locWOffset - (colorIndex * steps);

    //get the gradient step we're on between the two colors
    uint16One = mod16PS( uint16One, gradLength ); // (num + offset) % gradLength

    //get the blend ratio
    uint8Two = ( uint16One * 255 ) / gradLength;

    colorOne = getPaletteColor(palette, uint8One);
    colorTwo = getPaletteColor(palette, uint8One + 1);
    return colorUtilsPS::getCrossFadeColor(colorOne, colorTwo, uint8Two);
}

//returns a palette of length 1 containing the passed in color
//!!Make sure you delete the paletteArr when you're done with the palette
//since it is created with new
palettePS paletteUtilsPS::makeSingleColorPalette(CRGB Color){
    palettePS newPalette;
    CRGB *newPalette_arr = (CRGB*) malloc(1*sizeof(CRGB));
    //CRGB *newPalette_arr = new CRGB[1];
    newPalette_arr[0] = Color;
    newPalette = {newPalette_arr, 1};
    return newPalette;
}

//returns a palette of the specified length full of random colors
//!!Make sure you delete the paletteArr when you're done with the palette
//since it is created with new
palettePS paletteUtilsPS::makeRandomPalette(uint8_t length){
    palettePS newPalette;
    CRGB *newPalette_arr = (CRGB*) malloc(length*sizeof(CRGB));
    //CRGB *newPalette_arr = new CRGB[length];
    for(uint8_t i = 0; i < length; i++){
        newPalette_arr[i] = colorUtilsPS::randColor();
    }                                 
    newPalette = {newPalette_arr, length};
    return newPalette;
}
 