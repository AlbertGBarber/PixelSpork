#ifndef palettePS_h
#define palettePS_h

#include "FastLED.h"

//A struct for holding an array of colors and its length in one place
//this makes passing it to effects and functions easier
//(and allows you to do shenanigans by changing the length artificially)
//See paletteUtils.h for how to interact with palettes

//note that the paletteArr is a pointer to an array of CRGB colors
//this is b/c structs don't allow you to have variable length arrays

//example palette:
//CRGB palette_arr[] = { CRGB::Red, CRGB::Blue, CRGB::Green, CRGB::Purple, CRGB::Yellow };
//palettePS palette = {palette_arr, SIZE(palette_arr)};
struct palettePS {
  CRGB *paletteArr;
  uint8_t length;
} ;

/*
A struct for storing an array of palettes
Contains a pointer to the array and the length of the array (number of palettes)

Example declaration:
    (assuming you've already declared palette 1 and 2, etc)
    palettePS *paletteArr[] = { &cybPnkPal, &palette2, etc};
    paletteSetPS paletteSet = {paletteArr, SIZE(paletteArr)};
*/
struct paletteSetPS {
    palettePS **paletteArr;
    uint8_t length;

    //Returns the pointer to a palette at the index in the palette set (wraps so you always get a palette)
    //Most palette functions take a palette pointer.
    palettePS *getPalette(uint8_t index){
        return paletteArr[ mod8(index, length) ];
    };

    //Sets the palette at the passed in array index (wraps so you always add the palette somewhere)
    void setPalette(palettePS &palette, uint8_t index){
        paletteArr[ mod8(index, length) ] = &palette;
    };
};


#endif