#ifndef palettePS_h
#define palettePS_h

#include "FastLED.h"

//A struct for holding an array of colors and its length in one place
//this makes passing it to effects and functions easier
//(and allows you to do shenanigans by changing the length artificially)
//See paletteUtils.h for how to interact with palettes

//note that the paletteArr is a pointer to an array of CRGB colors
//this is b/c structs don't allow you to have variable length arrays

//Example palette:
//CRGB palette_arr[] = { CRGB::Red, CRGB::Blue, CRGB::Green, CRGB::Purple, CRGB::Yellow };
//palettePS palette = {palette_arr, SIZE(palette_arr)};
struct palettePS {
    CRGB *paletteArr;
    uint8_t length;
};

/*
A struct for storing an array of palettes
Contains a pointer to the array and the length of the array (number of palettes)

Example declaration:
    (assuming you've already declared palette 1 and 2, etc)
    palettePS *paletteArr[] = { &cybPnkPal_PS, &palette2, etc};
    paletteSetPS paletteSet = {paletteArr, SIZE(paletteArr), SIZE(paletteArr)};

    The second SIZE() is used to record the maximum size of the palette array for memory management.
    It should always be the same as the actual size of the array. 
*/
struct paletteSetPS {
    palettePS **paletteArr;
    uint8_t length;

    uint8_t maxLength;  //the maximum length of the palette array (used for memory management)

    //Returns the pointer to a palette at the index in the palette set (wraps so you always get a palette)
    //Most palette functions take a palette pointer.
    palettePS *getPalette(uint8_t index) {
        return paletteArr[mod8(index, length)];
    };

    //Replaces the palette at the `index` by changing the array's palette pointer 
    //(wraps so you always add the palette somewhere). 
    //In other words, it replaces the pointer in the set's palette array with the new palette pointer.
    void setPalette(palettePS &palette, uint8_t index) {
        paletteArr[mod8(index, length)] = &palette;
    };

    //Replaces the palette at the `index` by copying from the input palette 
    //(wraps so you always add the palette somewhere). 
    //Unlike the `setPalette()` function, this replaces the `index` palette's palette color array and length 
    //with that of the input palette's. This is handy if you have a temporary palette you want to place in the set.
    void setPaletteByCopy(palettePS &palette, uint8_t index) {
        paletteArr[mod8(index, length)]->paletteArr = palette.paletteArr;
        paletteArr[mod8(index, length)]->length = palette.length;
    };
};


#endif