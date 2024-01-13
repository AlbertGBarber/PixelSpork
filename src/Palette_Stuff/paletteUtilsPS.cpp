#include "paletteUtilsPS.h"

//TODO: add blendColorFromPalette where you just pass in a step (total steps assumed to be 255) and it works out how far in the palette you should be
using namespace paletteUtilsPS;

//sets the palette color at the specified index
//the index wraps, so running off the end of the palette, will put you back at the start
void paletteUtilsPS::setColor(palettePS &palette, CRGB color, uint8_t index) {
    palette.paletteArr[mod8(index, palette.length)] = color;
}

//returns the color at a specified index
//the index wraps, so running off the end of the palette, will put you back at the start
CRGB paletteUtilsPS::getPaletteColor(palettePS &palette, uint8_t index) {
    return palette.paletteArr[mod8(index, palette.length)];
}

//returns a pointer to the color in the palette array at the specified index (wrapping if needed)
//useful for background color syncing
//ie yourEffect.bgColor = paletteUtilsPS::getColorPtr(yourPalette, colorIndex);
CRGB *paletteUtilsPS::getColorPtr(palettePS &palette, uint8_t index) {
    return &(palette.paletteArr[mod8(index, palette.length)]);
}

//randomizes all the colors in a palette
//These can either be totally random,
//or use a random base hue, and use it to find complementary colors (colors evenly spaced across the HSV spectrum)
//In the complimentary case, the colors will share a limited random saturation value
void paletteUtilsPS::randomize(palettePS &palette, bool comp) {
    if( !comp ) {
        //make a fully random palette
        for( uint8_t i = 0; i < palette.length; i++ ) {
            randomizeCol(palette, i);
        }
    } else {
        //make a complementary palette, with a random base hue, and limited random saturation
        uint8One = random8();
        uint8Two = random8(100, 255);
        for( uint8_t i = 0; i < palette.length; i++ ) {
            colorOne = colorUtilsPS::getCompColor(uint8One, palette.length, i, uint8Two, 255);
            setColor(palette, colorOne, i);
        }
    }
}

//randomize the color in palette at the specified index
void paletteUtilsPS::randomizeCol(palettePS &palette, uint8_t index) {
    setColor(palette, colorUtilsPS::randColor(), index);
}

//Reverses the order of the palette colors
void paletteUtilsPS::reverse(palettePS &palette) {
    uint8One = 0;                   //start index
    uint8Two = palette.length - 1;  //end index

    //To reverse the palette we use a common algo
    //We basically swap pairs of colors, starting with the end and start of the array, and then moving inwards
    //Once we reach the center of the array, we stop
    while( uint8One < uint8Two ) {
        colorOne = getPaletteColor(palette, uint8One);  //get the start index color
        colorTwo = getPaletteColor(palette, uint8Two);  //get the end index color
        setColor(palette, colorTwo, uint8One);          //set the end color into the start index
        setColor(palette, colorOne, uint8Two);          //set the start color into the end index
        //move to the next pair of indexes
        uint8One++;
        uint8Two--;
    }
}

//Randomizes the order of colors in a palette.
//Note that it does not check against the current order, so it's possible to get the same palette order back. 
//The likely-hood of this is 1/(palette.length!), ie 1/3! => 1/6 for a palette length of 3. 
//Also includes an optional indexOrder array input, which will be set to match the shuffled index order,
//allowing you to track the palette changes over time without checking colors. 
//ie for a palette of 3 colors, if the shuffled palette only swaps the last to colors, 
//the indexOrder would be [0, 2, 1].
//NOTE that indexOrder's length should be equal to palette.length, and it should be pre-filled
//with the palette indexes (ie for a palette of length 3, it would be [0, 1, 2])
void paletteUtilsPS::shuffle(palettePS &palette, uint8_t *indexOrder) {
    uint8One = palette.length - 1; //loopStart
    //Start from the last element and swap one by one at random 
    //i > 0 is the loop limit b/c we don't need to swap the first element.
    for( uint8_t i = uint8One; i > 0; i-- ) {
        //Pick a random index from 0 to i
        uint8Two = random8(i + 1);

        //Swap arr[i] with the color at random index
        colorOne = getPaletteColor(palette, i);
        colorTwo = getPaletteColor(palette, uint8Two);  //random color
        setColor(palette, colorOne, uint8Two);
        setColor(palette, colorTwo, i);

        //Get the new palette index order (if an indexOrder array has been supplied)
        //by swapping the index values at the random index and the current loop index.
        if(indexOrder){
            uint8Three = indexOrder[uint8Two];
            indexOrder[uint8Two] = indexOrder[i];
            indexOrder[i] = uint8Three;
        }
    }
}

//retuns a random color from the palette
//the code checks to see if the random index matches the current color (passed in)
//if it does we'll just advance the index by one and return its color
//this stops the same color from being chosen again (assuming the palette doesn't repeat)
CRGB paletteUtilsPS::getShuffleColor(palettePS &palette, CRGB &currentPaletteVal) {
    uint8One = random8(palette.length);                             //guess an index
    colorOne = paletteUtilsPS::getPaletteColor(palette, uint8One);  //get the color at the guess
    if( colorOne == currentPaletteVal ) {
        return paletteUtilsPS::getPaletteColor(palette, uint8One + 1);
    } else {
        return colorOne;
    }
}

//returns the blended result of two palette colors
//startIndex and endIndex are the starting and ending Palette color indexes.
//step is the blend amount, out of totalSteps. 
//So a blend at step 5 out of 10 totalSteps would be 50% blended towards the endIndex color.
CRGB paletteUtilsPS::getBlendedPaletteColor(palettePS &palette, uint8_t startIndex, uint8_t endIndex, uint8_t step, uint8_t totalSteps) {
    //colorOne = getPaletteColor(palette, startIndex);
    //colorTwo = getPaletteColor(palette, endIndex);
    return colorUtilsPS::getCrossFadeColor(*getColorPtr(palette, startIndex), *getColorPtr(palette, endIndex), step, totalSteps);
}

/*
Returns a gradient color from a palette based on several inputs. Treats the whole palette as one large gradient.
    step: The value of what you want to apply the gradient to (generally a pixel or segment number) out of totalSteps
    offset: Any offset of step (used for shifting gradients over time)
    totalSteps: The length the entire palette gradient is spread across (usually the length of a SegmentSetPS or similar) */
CRGB paletteUtilsPS::getPaletteGradColor(palettePS &palette, uint16_t step, uint16_t offset, uint16_t totalSteps) {

    //gradient steps per color in the palette
    //this is a uint16_t to allow for gradients > 255
    uint16One = totalSteps / palette.length;  //gradLength

    //divide by 0 protection
    if( !uint16One ) {
        uint16One = 1;
    }

    return getPaletteGradColor(palette, step, offset, totalSteps, uint16One);
}

/* 
Returns a gradient color from a palette based on several inputs. Treats the whole palette as one large gradient.
This function is the same as the other "getPaletteGradColor()", but includes the gradLength as an input.
gradLength should always be totalSteps/palette.length. Using it as an argument allows you to pre-calculate it
rather than have the function do it every time for faster execution.*/
CRGB paletteUtilsPS::getPaletteGradColor(palettePS &palette, uint16_t step, uint16_t offset, uint16_t totalSteps, uint16_t gradLength) {
    //the actual gradient number we need based on the offset
    uint16One = addMod16PS(step, offset, totalSteps);  //(step + offset) % totalSteps;

    //the index of the palette color we're starting from (integer division always rounds down)
    uint8One = uint16One / gradLength;  // (step + offset)/gradLength

    // get the cross-fade step
    //uint8_t gradStep = locWOffset - (colorIndex * steps);

    //get the gradient step we're on between the two colors
    uint16One = mod16PS(uint16One, gradLength);  // (step + offset) % gradLength

    //get the blend ratio
    uint8Two = (uint16One * 255) / gradLength;

    //colorOne = getPaletteColor(palette, uint8One);
    //colorTwo = getPaletteColor(palette, uint8One + 1);
    return colorUtilsPS::getCrossFadeColor(*getColorPtr(palette, uint8One), *getColorPtr(palette, uint8One + 1), uint8Two);
}

//returns a palette of length 1 containing the passed in color
//!!Make sure you delete the paletteArr when you're done with the palette since it is created with new
palettePS paletteUtilsPS::makeSingleColorPalette(CRGB Color) {
    palettePS newPalette;
    CRGB *newPalette_arr = (CRGB *)malloc(1 * sizeof(CRGB));
    //CRGB *newPalette_arr = new CRGB[1];
    newPalette_arr[0] = Color;
    newPalette = {newPalette_arr, 1};
    return newPalette;
}

//returns a palette of the specified length full of random colors
//Colors can either be fully random, or made to be complementary with a random base hue (colors evenly spaced across the HSV spectrum)
//The default is for fully random colors
//!!Make sure you delete the paletteArr when you're done with the palette since it is created with new
palettePS paletteUtilsPS::makeRandomPalette(uint8_t length, bool comp) {
    palettePS newPalette;
    CRGB *newPalette_arr = (CRGB *)malloc(length * sizeof(CRGB));
    newPalette = {newPalette_arr, length};
    randomize(newPalette, comp);
    return newPalette;
}

//Returns a palette with a set of complimentary colors, starting from a base hue value.
//Allows you to easily make split, tri, and tetrad palettes. 
//Note that the colors are HSV based. sat and val adjust the saturation and value of the resulting palette colors. 
//!!Make sure you delete the paletteArr when you're done with the palette since it is created with new
palettePS paletteUtilsPS::makeCompPalette(uint8_t length, uint8_t baseHue, uint8_t sat, uint8_t val) {
    palettePS newPalette;
    CRGB *newPalette_arr = (CRGB *)malloc(length * sizeof(CRGB));
    newPalette = {newPalette_arr, length};

    //Fill the palette with complimentary colors
    for( uint8_t i = 0; i < length; i++ ) {
        colorOne = colorUtilsPS::getCompColor(baseHue, length, i, sat, val);
        setColor(newPalette, colorOne, i);
    }
    return newPalette;
}

/* Returns a palette that is a subsection of the input palette
The subsection starts at the startIndex of the input palette, and is splitLength colors long.
So the ending color index is startIndex + (splitLength - 1) (because we include the start index color).
Note that the output palette re-uses the pointer to the input palette's color array, but offset by the startIndex.
This means that any changes to the original palette will be reflected in the split palette.
Which is great for blending palettes, but if you ever change the original palette's color array pointer,
you must update the split palette (most easily by calling the split function again)!
    Ex: I have an input palette with 5 colors, I want a sub-palette with the last 3 colors
        I would do splitPalette( inputPalette, 2, 3 ); to capture the input palette color indexes 2, 3, 4 (ie the last 3 colors)
    EX 2: I want to capture only the first color in the input palette
        I would do splitPalette( inputPalette, 0, 1 ); */
palettePS paletteUtilsPS::splitPalettePtr(palettePS &inputPalette, uint8_t startIndex, uint8_t splitLength){
    palettePS newPalette;

    //A quick guard to make sure that the split palette doesn't run off the end of the input
    if( (startIndex + splitLength) > inputPalette.length){
        splitLength = startIndex + splitLength;
    }

    //Create the new palette by offsetting the starting index of the input's color array pointer.
    //Basically telling the new palette to start at startIndex of the input's color array.
    //ie we're doing colorArray[0 + startIndex]
    //This works because the arrays in C just point to a block of memory starting with the array's address.
    newPalette = { inputPalette.paletteArr + startIndex, splitLength };

    return newPalette;
}
