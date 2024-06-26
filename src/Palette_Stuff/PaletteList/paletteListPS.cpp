#include "paletteListPS.h"

//Default purple/green palette for Segment Set Color Mode custom gradients
CRGB segDefaultPal_PS_arr[] = {CRGB{30, 15, 230}, CRGB{0, 200, 0}};
palettePS segDefaultPal_PS = {segDefaultPal_PS_arr, SIZE(segDefaultPal_PS_arr)};

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

//Classic neon magenta-orange-teal cyberpunk/vaporwave palette
CRGB cybPnkPal_PS_arr[] = {CRGB{240, 0, 219}, CRGB{255, 153, 0}, CRGB{11, 196, 207}};
palettePS cybPnkPal_PS = {cybPnkPal_PS_arr, SIZE(cybPnkPal_PS_arr)};

//Alternate cyber punk palette, uses bright magenta, bright teal, and medium blue
CRGB cybPnkPalAlt_PS_arr[] = {CRGB{240, 0, 219}, CRGB{11, 196, 207}, CRGB{19, 61, 201} };
palettePS cybPnkPalAlt_PS = {cybPnkPalAlt_PS_arr, SIZE(cybPnkPalAlt_PS_arr)};