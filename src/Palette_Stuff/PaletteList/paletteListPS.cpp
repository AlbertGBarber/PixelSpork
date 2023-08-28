#include "paletteListPS.h"

//Red/yellow fire palette for use with Fire2012 effects
CRGB firePal_PS_arr[] = {CRGB::Red, CRGB{255, 143, 0}, CRGB{255, 255, 100}};
palettePS firePal_PS = {firePal_PS_arr, SIZE(firePal_PS_arr)};

//Pink/purple fire palette for use with Fire2012 effects
CRGB firePalPink_PS_arr[] = {CRGB{225, 0, 127}, CRGB{123, 7, 197}, CRGB{238, 130, 238}};
palettePS firePalPink_PS = {firePalPink_PS_arr, SIZE(firePalPink_PS_arr)};

//Blue/green fire palette for use with Fire2012 effects
CRGB firePalBlue_PS_arr[] = {CRGB{16, 124, 126}, CRGB{43, 208, 17}, CRGB{120, 212, 96}};
palettePS firePalBlue_PS = {firePalBlue_PS_arr, SIZE(firePalBlue_PS_arr)};

//Lava palette used in the Lava effect, basically a blend from dark red to yellow
CRGB lavalPal_arr[] = {CRGB::DarkRed, CRGB::Maroon, CRGB::Red, CRGB::Orange, CRGB(245, 202, 10)};
palettePS lavaPal_PS = {lavalPal_arr, SIZE(lavalPal_arr)};

//Classic neon magenta-teal cyber punk palette, uses bright magenta, medium blue, and bright teal
CRGB cybPnkPal_PS_arr[] = {CRGB{240, 0, 219}, CRGB{19, 61, 201}, CRGB{11, 196, 207}};
palettePS cybPnkPal_PS = {cybPnkPal_PS_arr, SIZE(cybPnkPal_PS_arr)};