#ifndef GradientCycleSL_h
#define GradientCycleSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/* 
Moves a set of color gradients along the segment set
the gradients can be set to follow a pattern, use a palette, or set randomly
The gradients have a set length, and smoothly transition from one color to the next, wrapping back to the first color at the end.
If the total length of the gradients is longer than the segment set, all colors will still be shown, 
they will just cycle on and off the segment set.

There is a version of this effect that takes less CPU power (GradientCycleSLFastPS)
It has a few restrictions, but should run faster than this effect

The effect is adapted to work on segment lines for 2D use, but you can keep it 1D by
passing in a SegmentSetPS with only one segment containing the whole strip.

Example calls: 
    uint8_t pattern_arr = {0, 2, 1};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
    GradientCycleSL gradientCycle(mainSegments, pattern, cybPnkPal_PS, 10, 100);
    Will do a gradient cycle of the cybPnkPal_PS palette, 
    following the pattern (color 0 to color 2 to color 1)
    with 10 steps to each gradient, and a 100ms update rate

    GradientCycleSL gradientCycle(mainSegments, cybPnkPal_PS, 10, 100);
    Will do a gradient cycle using the colors in the cybPnkPal_PS palette, 
    with 10 steps to each gradient,and a 100ms update rate

    GradientCycleSL gradientCycle(mainSegments, 3, 15, 80);
    Will do a gradient cycle using 3 randomly chosen colors, 
    with 15 steps to each gradient, and an 80ms update rate
 
Constructor Inputs:
    pattern(optional, see constructors) -- A pattern is struct made from a 1-d array of palette indexes ie {0, 1, 3, 6, 7} 
                                          and the length of the array 
                                          (see patternPS.h)   
    palette(optional, see constructors) -- The repository of colors used in the pattern, or can be used as the pattern itself
    numColors (optional, see constructors) -- The number of randomly chosen colors for the gradients
    gradLength -- How many steps for each gradient
    rate -- The update rate (ms)

Functions:
    setPaletteAsPattern() -- Sets the effect pattern to match the current palette (calls setTotalEffectLength())
    update() -- updates the effect

Reference Vars:
    totalCycleLength -- Total length of all the gradients combined, ie pattern->length * gradLength.totalCycleLength -- Total length of all the gradients combined, ie pattern->length * gradLength.
    cycleNum -- Tracks how many patterns we've gone through, resets every totalCycleLength cycles, set during update()

Notes:
    For the randomly generated gradient constructor, the random palette can be accessed via paletteTemp

    If using the palette as the pattern, if you change the palette, you'll need to change the pattern as well
    (unless the palettes are the same length) 
*/
class GradientCycleSL : public EffectBasePS {
    public:
        //Constructor for using pattern
        GradientCycleSL(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, uint8_t GradLength,
                        uint16_t Rate);

        //Constructor for using the palette as the pattern
        GradientCycleSL(SegmentSetPS &SegSet, palettePS &Palette, uint8_t GradLength, uint16_t Rate);

        //Constructor for using a random palette as the pattern
        GradientCycleSL(SegmentSetPS &SegSet, uint8_t NumColors, uint8_t GradLength, uint16_t Rate);

        ~GradientCycleSL();

        uint8_t
            gradLength;

        uint16_t
            totalCycleLength,  //total length of all the gradients combined, for reference
            cycleNum = 0;      // tracks what how many patterns we've gone through, resets every totalCycleLength cycles

        patternPS
            *pattern = nullptr,
            patternTemp = {nullptr, 0, 0};  //Must init structs w/ pointers set to null for safety

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        void
            setPaletteAsPattern(),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        uint8_t
            nextPattern,
            currentPattern,
            currentColorIndex,
            blendStep;

        uint16_t
            step,
            numLines;

        CRGB
            currentColor,
            nextColor,
            colorOut;

        void
            init(SegmentSetPS &SegSet, uint16_t Rate),
            setNextColors(uint16_t pixelNum);
};

#endif