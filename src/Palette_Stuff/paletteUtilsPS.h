#ifndef paletteUtilsPS_h
#define paletteUtilsPS_h

#include "palettePS.h"
#include "ColorUtils/colorUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

//series of utility functions for interacting with palettes
//use these to change palettes
//!!DO NOT try to palettes without using these functions
namespace paletteUtilsPS {
    void  //Functions for changing aspects of palettes
        setColor(palettePS &palette, CRGB color, uint8_t index),
        randomize(palettePS &palette, bool comp = false),
        randomizeCol(palettePS &palette, uint8_t index),
        shuffle(palettePS &palette, uint8_t *indexOrder = nullptr),
        reverse(palettePS &palette);

    CRGB  //Functions for getting colors from palettes
        getPaletteColor(palettePS &palette, uint8_t index),
        getBlendedPaletteColor(palettePS &palette, uint8_t startIndex, uint8_t endIndex, uint8_t step, uint8_t totalSteps),
        getShuffleColor(palettePS &palette, CRGB &currentPaletteVal),
        *getColorPtr(palettePS &palette, uint8_t index),
        getPaletteGradColor(palettePS &palette, uint16_t step, uint16_t offset, uint16_t totalSteps),
        getPaletteGradColor(palettePS &palette, uint16_t step, uint16_t offset, uint16_t totalSteps, uint16_t gradLength);

    palettePS  //Functions for making quick new palettes
        makeSingleColorPalette(CRGB Color),
        makeRandomPalette(uint8_t length, bool comp = false),
        makeCompPalette(uint8_t length, uint8_t baseHue, uint8_t sat, uint8_t val);

    //Pre-allocated variables
    static uint8_t
        uint8One,
        uint8Two,
        uint8Three;

    static uint16_t
        uint16One,
        uint16Two;

    static CRGB
        colorOne,
        colorTwo;
};

#endif