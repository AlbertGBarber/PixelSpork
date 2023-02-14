#ifndef paletteSetPS_h
#define paletteSetPS_h

#include "Palette_Stuff/palettePS.h"

/*
A struct for storing an array of palettes (see palettePS.h)
Contains a pointer to the array and the length of the array (number of palettes)

Example declaration:
    (assuming you've already declared palette 1 and 2, etc)
    palettePS *paletteArr[] = { &palette1, &palette2, etc};
    paletteSetPS paletteSet = {paletteArr, SIZE(paletteArr)};
*/
struct paletteSetPS {
    palettePS **paletteArr = nullptr;
    uint8_t length;

    //Returns the pointer to a palette at the index in the palette set (wraps so you always get a palette)
    //Most palette functions take a palette pointer.
    palettePS *getPalette(uint8_t index){
        return paletteArr[ mod8(index, length) ];
    };
};

#endif