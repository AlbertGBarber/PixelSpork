#ifndef palletPS_h
#define palletPS_h

#include "FastLED.h"

//A struct for holding an array of colors and its length in one place
//this makes passing it to effects and functions simplier
//(and allows you to do shinanigans by changing the length artificially)
//See palletUtils.h for how to interact with pallets

//note that the palletArr is a pointer to an array of CRGB colors
//this is b/c structs don't allow you to have variable length arrays

//example pallet:
//CRGB pallet_arr[] = { CRGB::Red, CRGB::Blue, CRGB::Green, CRGB::Purple, CRGB::Yellow };
//palletPS pallet = {pallet_arr, SIZE(pallet_arr)};
struct palletPS {
  CRGB *palletArr;
  uint8_t length;
} ;

#endif