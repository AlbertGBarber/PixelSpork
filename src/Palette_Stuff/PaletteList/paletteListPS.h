#ifndef paletteListPS_h
#define paletteListPS_h

#include "Palette_Stuff/palettePS.h"
#include "MathUtils/mathUtilsPS.h"

/*
A repository for pre-defined palettes
See .cpp file for colors.
*/

//Default purple/green palette for Segment Set Color Mode custom gradients
extern CRGB segDefaultPal_PS_arr[];
extern palettePS segDefaultPal_PS;

//Red/yellow fire palette for use with Fire2012 effects
extern CRGB firePal_PS_arr[];
extern palettePS firePal_PS;

//Pink/purple fire palette for use with Fire2012 effects
extern CRGB firePalPink_PS_arr[];
extern palettePS firePalPink_PS;

//Blue/green fire palette for use with Fire2012 effects
extern CRGB firePalBlue_PS_arr[];
extern palettePS firePalBlue_PS;

//Lava palette used in the Lava effect, basically a blend from dark red to yellow
extern CRGB lavaPal_PS_arr[];
extern palettePS lavaPal_PS;

//Classic neon magenta-orange-teal cyberpunk/vaporwave palette
extern CRGB cybPnkPal_PS_arr[];
extern palettePS cybPnkPal_PS;

//Alternate cyber punk palette, uses bright magenta, medium blue, and bright teal
extern CRGB cybPnkPalAlt_PS_arr[];
extern palettePS cybPnkPalAlt_PS;

#endif