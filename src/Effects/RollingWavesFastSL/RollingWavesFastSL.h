#ifndef RollingWavesFastSL_h
#define RollingWavesFastSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "Effects/ParticlesSL/Particle_Stuff/particleUtilsPS.h" //We use the particle color dimming function to dim the waves
#include "MathUtils/mathUtilsPS.h"

/*
This effect is the "Fast" version of RollingWavesSL.

Repeats a set of waves along the segment set according to the input pattern and palette. 
Each wave is a gradient that shifts towards a set background color. 
The waves are configured to rise and fall across a set length, `gradLength`. 
They can be set to only have a rise, fall, or both using "trailMode" (see info below). 
For waves with both a rise and fall, odd numbered `gradLength`'s work best. 
A spacing length can be added between the waves.

The effect is adapted to work on segment lines for 2D use. The wave colors are copied along segment lines. 

This effect is the "Fast" version of Rolling Waves, but instead of calculating the color of every pixel, 
it only does so for the lead pixel, while copying the color of each subsequent pixel down along the strip. 
This leads to a much more lightweight effect, however it does have a few extra restrictions:
    1. Changing the palette on the fly will have a delayed effect on the colors. 
        The existing colors will shift off the strip before new ones shift on. 
        This prevents this effect from playing well with most Palette Utility Classes.
    2. The same restrictions as (1) apply to changing the pattern, `gradLength`, etc.
    3. Changing the direction of the segments or segment set mid-effect may break it temporarily.
    4. This effect is not compatible with colorModes for either the waves or the background.
    5. The effect should not be run alongside other effects on the same Segment Set due to it copying colors from LEDs.
    6. The effect will not work well with the EffectSetFaderPS utility.

However, as a bonus, this effect supports random colored waves
where the colors for the waves are chosen at random as the enter the strip
This is controlled by the "randMode" setting.

randModes (default 0): 
    0: Colors will be chosen from the palette in order (not random)
    1: Colors will be chosen completely at random
    2: Colors will be chosen randomly from the pattern (will not repeat the same color in a row)

Inputs Guide & Notes:

    Trail Modes:
        The waves can be drawn with a lead or trailing tail or both. 
        The `gradLength` sets how long the waves will be. 
        The same `gradLength` will be used for each wave type, so for double waves the
        wave lengths will be half the `gradLength`.

    trailMode (uint8_t):
        * 0 -- One trail facing away from the direction of motion (like a comet).
        * 1 -- Two trails, coming from both sides of the particle.
        * 2 -- One trail facing towards the direction of motion.

        For example, with a gradLength of 7, the modes will produce:
        (The wave front is *, - are the trail, particle is moving to the right ->)
        * 0: ------*
        * 1: ---*---
        * 2: *------

    Wave Fading:
        By default, the waves dim quickly in a non-linear fashion. 
        This makes the wave "head / center" brighter and standout more, which, in my opinion, 
        looks better then just using a linear fade. You can control the linearity of the wave 
        fades using the `dimPow` setting. A default of 120 is used in this effect. 
        dimPow is borrowed from the particle based effects; you can read the dimPow notes in ParticleUtils.h.

Example calls: 
    uint8_t pattern_arr = {0, 2, 1};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
    
    RollingWavesFastSL rollingWavesFast(mainSegments, pattern, cybPnkPal_PS, 0, 7, 1, 0, 100);
    Will do a set of waves according to the pattern, using cybPnkPal_PS for colors, 
    with a blank background
    Each wave will be 7 pixels long, using both leading and trailing trails
    there will be zero spacing between the waves
    The effect will update at a 100ms

    RollingWavesFastSL rollingWavesFast(mainSegments, cybPnkPal_PS, 0, 9, 0, 2, 80);
    Will do a set of waves matching the input palette with an blank background
    Each wave will be 9 pixels long, with only a trailing trail
    There will be two spaces in between each wave,
    The effect will update at 80ms

    RollingWavesFastSL rollingWavesFast(mainSegments, CRGB(CRGB::Blue), CRGB::Red, 12, 1, 3, 80);
    Will do a set of blue waves, with a red background
    Each wave will a length of 12, with only a leading trail
    There will be 3 background spaces between each wave
    The effect will update at 80ms
    !!If using a pre-built FastLED color for the waves,
    you need to pass it as CRGB( *color code* ) -> ex CRGB(CRGB::Blue)

Constructor Inputs:
    pattern(optional, see constructors) -- A pattern is struct made from a 1-d array of palette indexes ie {0, 1, 3, 6, 7} 
                                           and the length of the array 
                                           (see patternPS.h)   
    palette(optional, see constructors) -- The repository of colors used in the pattern, or can be used as the pattern itself
    numColors (optional, see constructors) -- The number of randomly chosen colors for the gradients
    color (optional, see constructors) -- A single color for the waves. 
                                          If using a pre-built FastLED color for the waves,
                                          you need to pass it as CRGB( *color code* ) -> ex CRGB(CRGB::Blue)
    bgColor -- The color of the spacing pixels. It is a pointer, so it can be tied to an external variable
    gradLength -- The length of each wave. Can be changed later using setGradLength().
    trailMode -- The type of waves used (see "Trail Modes" in intro). Can be changed later using setTrailMode().
    spacing -- The number of background spaces between each wave.
    rate -- The update rate (ms)

Other Settings:
    randMode (default 0) -- Sets how wave colors will be chosen. (See randMode notes in intro)
    dimPow (default 120, min -127, max 127) -- Adjusts fading rate for the wave trails (see Fading notes in intro)

Functions:
    setPaletteAsPattern() -- Sets the effect pattern to match the current palette
    setGradLength(newGradLength) -- Changes the gradLength, adjusting the length of the waves
    setTrailMode(newTrailMode) -- Changes the trail mode used for the waves. (see "Trail Modes" in intro).
    update() -- updates the effect

Reference Vars:
    gradLength -- The length of the waves, set using setGradLength().
    trailMode -- They type of trails drawn for the waves, set using setTrailMode().
    cycleNum -- Tracks what how many update cycles we've gone through, resets every totalCycleLength cycles.
    totalCycleLength -- Total length of all the waves of each color combined, re-calculated each update
    
Flags:
    initFillDone -- Flag for doing the initial fill of waves on the strip to pre-fill it for the first update cycle.
                    Set true once the fill is done.

*/
class RollingWavesFastSL : public EffectBasePS {
    public:
        //Constructor with pattern
        RollingWavesFastSL(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BgColor, 
                           uint8_t GradLength, uint8_t TrailMode, uint8_t Spacing, uint16_t Rate);

        //Constructor with palette as pattern
        RollingWavesFastSL(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint8_t GradLength, 
                           uint8_t TrailMode, uint8_t Spacing, uint16_t Rate);

        //Constructor with random colors
        RollingWavesFastSL(SegmentSetPS &SegSet, uint8_t NumColors, CRGB BgColor, uint8_t GradLength, 
                           uint8_t TrailMode, uint8_t Spacing, uint16_t Rate);

        //constructor with a single color
        //!!If using a pre-built FastLED color for the waves you need to pass it as CRGB( *color code* ) -> ex CRGB(CRGB::Blue)
        RollingWavesFastSL(SegmentSetPS &SegSet, CRGB Color, CRGB BgColor, uint8_t GradLength, 
                           uint8_t TrailMode, uint8_t Spacing, uint16_t Rate);

        ~RollingWavesFastSL();

        uint8_t
            randMode = 0,
            dimPow = 120,  //120
            spacing,
            trailMode,
            blendStep,  //the step of the current blend, at 0, a new blend will start, for reference
            gradLength;

        uint16_t
            totalCycleLength,  //total length of all the gradients combined, for reference
            cycleNum = 0;      // tracks what how many update cycles we've gone through, for reference

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
            setTrailMode(uint8_t newTrailMode),
            setGradLength(uint8_t newGradLength),
            setPaletteAsPattern(),
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
            numLines,
            longestSeg,
            numLinesLim,
            pixelNum;

        bool
            setBg = false;

        CRGB
            currentColor,
            colorOut,
            getWaveColor(uint16_t step);

        void
            init(CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate),
            initalFill(),
            setTotalEffectLength(),
            setNextColors(uint16_t segPixelNum);
};

#endif