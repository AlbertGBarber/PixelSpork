#ifndef patternPS_h
#define patternPS_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WConstants.h"
#include "WProgram.h"
#include "pins_arduino.h"
#endif

//a struct for holding a uint8_t array and it's length in one place
//this makes passing it to effects and functions simplier
//(and allows you to do shinanigans by changing the length artificially)
//See patternUtils.h for interacting with patterns

//generally a pattern will be paired with a palette in an effect, where the 
//pattern entrys will be the indecies of the colors in the palette
//ie pattern of {0, 1, 4} would corrospond to the first, second, and fifth colors in the palette

//note that the patternArr is a pointer to an array of CRGB colors
//this is b/c structs don't allow you to have variable length arrays

//example pattern:
//uint8_t pattern_arr[] = {0, 2, 1};
//patternPS pattern = { pattern_arr, SIZE(pattern_arr) };
struct patternPS {
  uint8_t *patternArr = nullptr;
  uint16_t length;
} ;

#endif