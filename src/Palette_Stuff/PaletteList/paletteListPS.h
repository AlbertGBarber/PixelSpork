#ifndef paletteListPS_h
#define paletteListPS_h

#include "Palette_Stuff/palettePS.h"
#include "MathUtils/mathUtilsPS.h"

/*
A repository for pre-defined palettes
See .cpp file for colors.
*/

//Red/yellow fire palette for use with Fire2012 effects
extern CRGB firePal_arr[];
extern palettePS firePal;

//Pink/purple fire palette for use with Fire2012 effects
extern CRGB firePalPink_arr[];
extern palettePS firePalPink;

//Blue/green fire palette for use with Fire2012 effects
extern CRGB firePalBlue_arr[];
extern palettePS firePalBlue;

//Lava palette used in the Lava effect, basically a blend from dark red to yellow
extern CRGB lavaPal_arr[];
extern palettePS lavaPal;

//Classic neon magenta-teal cyber punk palette, uses bright magenta, medium blue, and bright teal
extern CRGB cybPnkPal_arr[];
extern palettePS cybPnkPal;

#endif