#ifndef palettePS_h
#define palettePS_h

#include "FastLED.h"

//A struct for holding an array of colors and its length in one place
//this makes passing it to effects and functions simplier
//(and allows you to do shinanigans by changing the length artificially)
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

#endif