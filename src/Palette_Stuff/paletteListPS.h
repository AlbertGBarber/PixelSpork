#ifndef paletteListPS_h
#define paletteListPS_h

#include "palettePS.h"

/*A repository for pre-defined palettes*/

//Red/yellow fire palette for use with Fire2012 effects
static CRGB firePal_arr[] = { CRGB::Red, CRGB{255, 143, 0}, CRGB{255, 255, 100} };
static palettePS firePal = {firePal_arr, SIZE(firePal_arr)};

//Pink/purple fire palette for use with Fire2012 effects
static CRGB firePalPink_arr[] = { CRGB{225, 0, 127}, CRGB{123, 7, 197}, CRGB{238, 130, 238} };
static palettePS firePalPink = {firePalPink_arr, SIZE(firePalPink_arr)};

//Blue/green fire palette for use with Fire2012 effects
static CRGB firePalBlue_arr[] = { CRGB{16, 124, 126 }, CRGB{ 43, 208, 17 }, CRGB{120, 212, 96} };
static palettePS firePalBlue = {firePalBlue_arr, SIZE(firePalBlue_arr)};

//Classic neon magenta-teal cyber punk palette, uses bright magenta, medium blue, and bright teal
static CRGB cybPnkPal_arr[] = { CRGB{240, 0, 219}, CRGB{19, 61, 201}, CRGB{11, 196, 207}};
static palettePS cybPnkPal = {cybPnkPal_arr, SIZE(cybPnkPal_arr)};

#endif