#ifndef RollingWavesSL2_h
#define RollingWavesSL2_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "Effects/ParticlesSL/Particle_Stuff/particleUtilsPS.h" //We use the particle color dimming function to dim the waves
#include "MathUtils/mathUtilsPS.h"

/*
This effect is almost identical to RollingWavesFastSL, but works slightly differently
which causes an "error" when working with segments sets with different length segments.
The error causes the segment lines not to sync up, leading to some interesting results
which is why this effect exists.
I must stress that for segment sets where all the segments are the same length, the effect will be
the same as the normal RollingWavesFastSL, and should not be used.

This effect is also very similar to gradientCycleFastSL, and could be adapted to work with gradients
I have not included a gradientCycle version because this effect is more of a quirk.
I didn't want to bloat up the code base with multiple versions.
(It's bad enough that this is basically identical to RollingWavesFastSL, and technically their
common functions should be split into a separate file)

The effect is adapted to work on segment lines for 2D use, but you can keep it 1D by
passing in a SegmentSetPS with only one segment containing the whole strip.

Note that this effect should not be run alongside other effects on the same SegmentSetPS
since it needs to use the existing colors of the leds

Note that for full waves, odd numbered gradLengths work the best

This effect is the same as RollingWavesPS, but should need fewer calculations per cycle
because it only works out the colors once. It may take up more programming space.
however it does have a few extra restrictions
1: Changing the palette on the fly will have a delayed effect on the colors
   The existing colors will shift off the strip before new ones shift on
   This prevents this effect from playing well with paletteBlend functions
2: The same restrictions as (1) apply to changing the pattern, the gradLength, the spacing, or trailModes
3: Changing the direction of the segments or segment set mid-effect may break it temporarily
4: Color modes are disabled

Basically the effect works by setting the color of the first pixel, then for each subsequent pixel,
it copies the color of the next pixel in line
So any changes you make to colors will only show up at the first pixel, and will be shifted along the strip

However, as a bonus, this effect supports random colored waves
where the colors for the waves are chosen at random as the enter the strip
This is controlled by the randMode setting

Example calls: 
    uint8_t pattern_arr = {0, 2, 1};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
    RollingWavesSL2 rollingWaves2(mainSegments, pattern, cybPnkPal_PS, 0, 7, 1, 0, 100);
    Will do a set of waves according to the pattern, with a blank background
    each wave will be 7 pixels long, using both types of trails
    there will be zero spacing between the waves
    The effect will update at a 100ms

    RollingWavesSL2 rollingWaves2(mainSegments, cybPnkPal_PS, 0, 9, 0, 2, 80);
    Will do a set of waves matching the input palette with an blank background
    Each wave will be 9 pixels long, the wave will consist of the trailing portion only
    There will be two spaces in between each wave,
    The effect will update at 80ms

    RollingWavesSL2 rollingWaves2(mainSegments, 1, CRGB::Red, 12, 1, 3, 80);
    Will do a set of waves of a single color chosen at random, with a red background
    Each wave will a length of 12, and will contain only the leading portion of the wave
    There will be 3 background spaces between each wave
    The effect will update at 80ms

Constructor Inputs:
    pattern(optional, see constructors) -- A pattern is struct made from a 1-d array of palette indexes ie {0, 1, 3, 6, 7} 
                                           and the length of the array 
                                           (see patternPS.h)   
    palette(optional, see constructors) -- The repository of colors used in the pattern, or can be used as the pattern itself
    numColors (optional, see constructors) -- The number of randomly chosen colors for the gradients
    BgColor -- The color of the spacing pixels. It is a pointer, so it can be tied to an external variable
    gradLength -- How many steps for each gradient
    trailMode -- They type of waves used (see trailMode section below)
    spacing -- The number of background spaces between each wave
    Rate -- The update rate (ms)

Trail Modes:
    The waves can be drawn with a lead or trailing tail or both
    The gradLength sets how long the waves will be
    The same gradLength will be used for each wave type, so for double waves the wave lengths will be half the gradLength
    The Modes:
    0: Only ending trail will be drawn
    1: Both ending and leading trails will be drawn (at half gradLength)
    2: Only the leading trail will be drawn
    For example, with a trail length of 7, the modes will produce:
    (The trail head is *, - are the trail)
    0: ------* 
    2: ---*---
    3: *------

Other Settings:
    randMode (default 0) -- Sets how colors are chosen from the palette
                         -- 0: Colors will be chosen from the palette in order (not random)
                         -- 1: Colors will be chosen completely at random
                         -- 2: Colors will be chosen randomly from the pattern (will not repeat the same color in a row)

    dimPow (default 120, max 255) -- Adjusts the rate of dimming for the wave trails
                                     255 will do typical linear dimming
                                     While anything below will dim faster
                                     120 sets a good balance of brightness, will not dimming most colors to 0 before the end of the wave

Functions:
    setPaletteAsPattern() -- Sets the effect pattern to match the current palette
    setGradLength(newGradLength) -- Changes the gradLength to the specified value, adjusting the length of the waves
    setTrailMode(newTrailMode) -- Changes the trail mode used for the waves
    buildLineArr() -- Creates the array for storing the line pixel locations
                      !! Only call this if you change your segment set
    update() -- updates the effect
    
Flags:
    initFillDone -- Flag for doing the initial fill of the gradients on the strip
                    Set true once the fill is done

Reference Vars:
    totalCycleLength -- Total length of all the gradients combined, re-calculated each update.
    gradLength -- The length of the waves, set using setGradLength().
    trailMode -- The type of trails used for the waves, set using setTrailMode().
    cycleNum -- Tracks what how many patterns we've gone through, resets every totalCycleLength cycles.

Notes:
*/
class RollingWavesSL2 : public EffectBasePS {
    public:
        //Constructor with pattern
        RollingWavesSL2(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BgColor,
                        uint8_t GradLength, uint8_t TrailMode, uint8_t Spacing, uint16_t Rate);

        //Constructor with palette as pattern
        RollingWavesSL2(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint8_t GradLength,
                        uint8_t TrailMode, uint8_t Spacing, uint16_t Rate);

        //Constructor with random colors
        RollingWavesSL2(SegmentSetPS &SegSet, uint8_t NumColors, CRGB BgColor, uint8_t GradLength,
                        uint8_t TrailMode, uint8_t Spacing, uint16_t Rate);

        ~RollingWavesSL2();

        uint8_t
            randMode = 0,
            dimPow = 120,  //120
            spacing,
            trailMode,
            blendStep,  //the step of the current blend, at 0, a new blend will start, for reference
            gradLength;

        uint16_t
            totalCycleLength,  //total length of all the gradients combined, for reference
            cycleNum = 0,      // tracks what how many patterns we've gone through, for reference
            *nextLine = nullptr;

        bool
            initFillDone = false;

        CRGB
            bgColorOrig,
            *bgColor = nullptr;  //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        patternPS
            *pattern = nullptr,
            patternTemp = {nullptr, 0, 0};  //Must init structs w/ pointers set to null for safety

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        void
            setGradLength(uint8_t newGradLength),
            setTrailMode(uint8_t newTrailMode),
            setPaletteAsPattern(),
            buildLineArr(),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        uint8_t
            stepTemp,
            currentPattern,
            currentColorIndex,
            halfGrad,
            midPoint,
            blendStepAdjust,
            firstHalfGrad,
            blendLimit;

        uint16_t
            numSegs,
            numSegsMax = 0,  //used for tracking the memory size of the next line array
            numLines,
            numLinesLim,
            pixelNum;

        bool
            setBg = false;

        CRGB
            getWaveColor(uint8_t step),
            currentColor,
            colorOut;

        void
            init(CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate),
            initalFill(),
            setTotalEffectLength(),
            setNextColors(uint16_t segPixelNum);
};

#endif