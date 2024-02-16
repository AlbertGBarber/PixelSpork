#ifndef GradientCycleFastSL_h
#define GradientCycleFastSL_h

//GradientCycleFastSeg is in the extras folder of the library

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/* 
Moves a set of color gradients along the segment set
the gradients can be set to follow a pattern, use a palette, or set randomly
The gradients have a set length, and smoothly transition from one color to the next, wrapping back to the first color at the end.
If the total length of the gradients is longer than the segment set, all colors will still be shown, 
they will just cycle on and off the segment set.

The effect is adapted to work on segment lines for 2D use, but you can keep it 1D by
passing in a SegmentSetPS with only one segment containing the whole strip.

Note that this effect should not be run alongside other effects on the same SegmentSetPS
since it needs to use the existing colors of the leds

This effect is the same as Gradient Cycle (Seg Line), 
but instead of calculating the color of every pixel, it only does so for the lead pixel, 
while copying the color of each subsequent pixel down along the strip. 
This leads to a much more lightweight effect, however it does have a few extra restrictions:
    1: Changing the palette on the fly will have a delayed effect on the colors. 
       The existing colors will shift off the strip before new ones shift on. 
       This prevents this effect from playing well with most Palette Utility Classes.
    2: The same restrictions as (1) apply to changing the pattern or the gradient lengths, `gradLength`.
    3: Changing the direction of the segments or segment set mid-effect may break it temporarily.
    4: The effect should not be run alongside other effects on the same Segment Set due to it copying colors from LEDs.
    5: The effect will not work well with the Effect Set Fader utility.

However, as a bonus, this effect supports random colored gradients
where the colors for the gradients are chosen at random as the enter the strip
This is controlled by the randMode setting:

randModes (default 0): 
    0: Colors will be chosen from the palette in order (not random).
    1: Colors will be chosen completely at random.
    2: Colors will be chosen randomly from the pattern (will not repeat the same color in a row).

Example calls: 
    uint8_t pattern_arr = {0, 2, 1};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
    GradientCycleFastSL gradientCycleFast(mainSegments, pattern, cybPnkPal_PS, 10, 100);
    Will do a gradient cycle from color 0, to color 1, to color 4, of the palette
    with 10 steps to each gradient, and a 100ms update rate

    GradientCycleFastSL gradientCycleFast(mainSegments, cybPnkPal_PS, 10, 100);
    Will do a gradient cycle using the colors in the palette, with 10 steps to each gradient,and a 100ms update rate

    GradientCycleFastSL gradientCycleFast(mainSegments, 3, 15, 80);
    Will do a gradient cycle using 3 randomly chosen colors, with 15 steps to each gradient,and an 80ms update rate
    note this is not the same as setting randMode, it just makes a random palette
 
Constructor Inputs:
    pattern(optional, see constructors) -- A pattern is struct made from a 1-d array of palette indexes ie {0, 1, 3, 6, 7} 
                                          and the length of the array 
                                          (see patternPS.h)   
    palette(optional, see constructors) -- The repository of colors used in the pattern, or can be used as the pattern itself
    numColors (optional, see constructors) -- The number of randomly chosen colors for the gradients
    gradLength -- How many steps for each gradient
    rate -- The update rate (ms)

Other Settings:
    randMode (default 0) -- (See randMode notes in intro)

Functions:
    setPaletteAsPattern() -- Sets the effect pattern to match the current palette
    reset() -- Restarts the effect
    update() -- updates the effect

Reference Vars:
    cycleNum -- Tracks how many cycles we've done, resets every gradLength cycles

Flags:
    initFillDone -- Flag for doing the initial fill of the gradients on the strip
                    Set true once the fill is done

*/
class GradientCycleFastSL : public EffectBasePS {
    public:
        //Constructor for using pattern
        GradientCycleFastSL(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, uint8_t GradLength, uint16_t Rate);

        //Constructor for using the palette as the pattern
        GradientCycleFastSL(SegmentSetPS &SegSet, palettePS &Palette, uint8_t GradLength, uint16_t Rate);

        //Constructor for using a random palette as the pattern
        GradientCycleFastSL(SegmentSetPS &SegSet, uint8_t NumColors, uint8_t GradLength, uint16_t Rate);

        ~GradientCycleFastSL();

        uint8_t
            randMode = 0,
            gradLength,
            cycleNum = 0;  // tracks what how many patterns we've gone through;

        bool
            initFillDone = false;

        patternPS
            *pattern = nullptr,
            patternTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        void
            setPaletteAsPattern(),
            reset(),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        uint8_t
            longestSeg,
            currentPattern,
            nextPattern;

        uint16_t
            numLines,
            numLinesLim,
            pixelNum,
            patternCount = 0;

        CRGB
            currentColor,
            nextColor,
            colorOut;

        void
            init(SegmentSetPS &SegSet, uint16_t Rate),
            pickNextColor(),
            initalFill();
};

#endif