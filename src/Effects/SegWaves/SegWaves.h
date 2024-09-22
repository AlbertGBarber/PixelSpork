#ifndef SegWaves_h
#define SegWaves_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
//#include "MathUtils/mathUtilsPS.h"

/* 

Moves a set of colors across whole segments over time. The colors transition smoothly by blending, like a set of waves. 
The effect uses a pattern for colors. You can either input a custom pattern, or the effect can build one 
for you with a set color wave lengths and spaces between each wave. 
There is also a "rainbow mode" constructor that creates a rainbow wave, 
and a constructor for using a single wave color only.

This effect is adapted for 2D use. The wave colors will be drawn and move along whole segments. 
For a version of this effect for segment lines, see StreamerSL.

Because blending can take quite a bit of processing power, especially for longer segment sets,
you can disable it using the `fadeOn` setting. 
If `fadeOn` is false, the waves will transition in one step, greatly simplifying the effect at the cost of some smoothness. 
There is also a "fast" version of this effect, SegWavesFast, 
which requires less processing power to run, but has a few extra restrictions. 

This effect supports color modes for both the wave and background colors, 
but to help make the effect faster you are restricted to modes 2, 4, 5, 7, 9, and 10. 
Also, in random modes (see `randMode`'s below), the wave colors are copied and shifted across the segments, 
so color modes will not function correctly.

Inputs Guide & Notes:

    Patterns:
        Patterns work the same as with other effects; they are a pattern of palette array indexes. 
        ie a pattern of {0, 2, 1} would be the first three colors of a palette. 
        However, in this effect, to indicate a background pixel (set it to the bgColor) 
        we use 255 in the pattern. Make sure your palette doesn't have 255 colors (that would be waaaay to big!).

        For example, lets say we wanted to do the first two colors of our palette, each as length 4 waves, 
        with 3 background pixels in between each. 
        We would make a pattern as: `{0, 0, 0, 0, 255, 255, 255, 1, 1, 1, 1, 255, 255, 255}`.

        For simple patterns like the previous example, I have written a few constructors that automate the 
        pattern creation for you (see constructor notes below). I have also included a shortcut for 
        making a single wave (a wave of length 1, with only one wave active on the segment set a once) 
        that sets the spacing automatically. All the pattern colors will still be used.
        You can trigger this with any of the constructors with a `waveThickness` input, 
        just pass in 0, and a single wave pattern will be built for you.

        Note that while each entry in the pattern is a uint8_t, if you have a lot of colors, 
        with long waves and spaces, your patterns may be quite large, so watch your memory usage. 

        Any automatically generated patterns will be allocated dynamically. 
        To avoid memory fragmentation, when you create the effect, you should set your color length and spacing 
        to the maximum values you expect to use, and then call `setPatternAsPattern()` or `setPaletteAsPattern()` 
        to resize your waves and spacing. 
        See https://github.com/AlbertGBarber/PixelSpork/wiki/Effects-Advanced#managing-dynamic-memory-and-fragmentation 
        for more details. 

        Also, remember that the pattern length is limited to 65,025 (uint16_t max), 
        so make sure your `(waveLength + spacing) * <num palette colors>` is less than the limit. 

        Finally, the effect needs to store a CRGB array of size `numSegs + 1` for random modes. 
        This is also allocated dynamically, however, this will only needs to be adjusted if you 
        change the number of segments in your segment set. It is re-sized when calling `reset()`.
    
    Rainbow Waves:
        The effect has a special constructor which will automatically create a rainbow palette and pattern
        that is spread evenly across the segment set.
        Note that if the number of segments in the set is greater than 255, the rainbow will cap at 255, and repeat.
        There is no spacing for this mode.
        Should you need to, you can re-create the rainbow pattern/palette by calling makeRainbowWaves().

    Random Modes: 
        `randMode` controls how pattern colors are chosen. For `randMode`'s other than 0, 
        colors are chosen at random as they enter the first segment, and are then shifted across the segments. 

        `randMode` (default 0) (uint8_t): 
        * 0 -- Colors will be chosen in order from the pattern (not random).
        * 1 -- Colors will be chosen completely at random.
        * 2 -- Colors will be chosen at random from the pattern, but the same color won't be repeated in a row. (won't pick spaces).
        * 3 -- Colors will be chosen randomly from the pattern (allowing repeats). (won't pick spaces).

        Note that switching from random modes to the fixed pattern mode (`randMode` 0) will cause a jump in colors.

The bgColor is a pointer, so you can bind it to an external color variable.

Example calls: 
    uint8_t pattern_arr = {0, 255, 255, 255, 1, 1, 255, 255};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
    SegWaves segWaves(mainSegments, pattern, cybPnkPal_PS, 0, 30, true, 20);
    Will do a set of waves using pattern above, with colors from the cybPnkPal_PS palette.
    The background is blank (0).
    The waves will begin with 1 pixel of color 0, with three spaces after, 
    followed by 2 pixels of color 1, followed by 2 spaces
    The waves will blend, taking 30 steps, with 20ms between each step
    The waves will move from the last segment to the first

    uint8_t pattern_arr = {0, 2, 1};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
    SegWaves segWaves(mainSegments, pattern, cybPnkPal_PS, 0, 3, 4, 0, false, 120);
    Will do a wave using the first three colors of the cybPnkPal_PS palette 
    (following the pattern)
    The background is blank (0).
    Each wave will be length 3, followed by 4 spaces.
    The fade steps are set to zero, so there is no blending. (fadeOn will be set false).
    The waves will move from the first to last segment.
    The effect updates at a rate of 120ms

    SegWaves segWaves(mainSegments, cybPnkPal_PS, CRGB::Red, 0, 0, 10, true, 40);
    Will do a waves using all the colors in the cybPnkPal_PS palette,
    because the passed in waveThickness is 0, the effect will be configured as to create a pattern of single waves
    (wave thickness of 1, with spacing such that there's only one wave on the segment sets at one time)
    (the 0 spacing input will be ignored)
    The background is red.
    The waves will blend, taking 10 steps, with 40ms between each step
    The waves will move from the last segment to the first

    SegWaves segWaves(mainSegments, CRGB::Blue, CRGB::Red, 2, 3, 0, true, 140);
    Will do a blue waves with length 2 and 3 spaces in between
    The background is red.
    The fade steps are set to zero, so there is no blending (fadeOn will be set false).
    The effect updates at a rate of 140ms
    The waves will move from the last segment to the first

    SegWaves segWaves(mainSegments, 10, false, 80);
    Will do a rainbow wave set, with 10 blend steps for the rainbow
    The waves will move from the first to last segment.
    The effect updates at a rate of 80ms
 
Constructor Inputs:
    pattern(optional, see constructors) -- The pattern used for the waves, made up of palette indexes 
    palette(optional, see constructors) -- The repository of colors used in the pattern
    color(optional, see constructors) -- Used for making a single color wave
    bgColor -- The color of the spacing pixels. It is a pointer, so it can be tied to an external variable
    waveThickness (optional, see constructors) -- The length of a wave. Used for automated pattern creation. 
                                                  Passing 0 will create a pattern of single length waves (see Inputs Guide above).
    spacing (optional, see constructors) -- The number of pixels between each wave color (will be set to bgColor).  
                                            Used for automated pattern creation.
    fadeSteps -- The number of steps to blend from one wave color to the next.
                 Passing 0 or 1 will set fadeOn to false (no fading) (see Other Settings below).
    direct --  The direction the waves move
               If true, the waves will move from the last to first segment, if false, the reverse.
    rate -- The update rate (ms)

Other Settings:
    colorMode (default 0) -- sets the color mode for the wave pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the spacing pixels (see segDrawUtils::setPixelColor)
    fadeOn (default true) -- If false, waves will use a single blend step, jumping directly from one color to the next.
                             This requires less processing power, but isn't as smooth.
                             Note that if 1 or 0 are passed as fadeSteps in a constructor, 
                             fadeOn will be set to false automatically
                             If you switch during run time, adjust your update rate to 
                             (fadeSteps * <your current rate>) for smoothness.
    randMode (default 0) -- (See randMode notes in intro)

Functions:
    reset() -- Restarts the wave pattern (also calls resetSegColors())
    resetSegColors() -- Only needed for random modes when changing the number of segments in your set. 
                        Re-creates the segColors array, which used for the random modes.
    setPatternAsPattern(&inputPattern, colorLength, spacing) -- Takes an input pattern and creates a wave pattern from it using the current palette
                                                                Ex: uint8_t pattern_arr = {1, 2, 3};
                                                                   patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
                                                                   setPatternAsPattern(pattern, 3, 4) 
                                                                   Will do a wave using the first three colors of the palette (taken from the pattern)
                                                                   Each wave will be length 3, followed by 4 spaces
    setPaletteAsPattern(colorLength, spacing) -- Like the previous function, but all of the current palette will be used for the pattern                                                       
    makeSingleWave() -- Creates a wave pattern such that there's a single wave of length 1 
                        active on the segment set at one time. All the pattern colors will still be used.
    makeRainbowWaves() -- Uses the effect's local pattern and palette (patternTemp and paletteTemp)
                          To create an evenly distributed a rainbow across the segment set
                          Note that if the number of segments in the set is greater than 255, 
                          the rainbow will cap at 255, and repeat.
    update() -- updates the effect

Reference vars:
    cycleNum -- Tracks how many patterns we've gone through, 
                resets every pattern length number of cycles (ie once we've gone through the whole pattern)
*/
class SegWaves : public EffectBasePS {
    public:
        //Constructor for using the passed in pattern and palette for the waves
        SegWaves(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BgColor, uint8_t FadeSteps,
                 bool Direct, uint16_t Rate);

        //Constructor for building the wave pattern from the passed in pattern and the palette, using the passed in waveThickness and spacing
        SegWaves(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BgColor, uint16_t WaveThickness,
                 uint16_t Spacing, uint8_t FadeSteps, bool Direct, uint16_t Rate);

        //Constructor for building a wave using all the colors in the passed in palette, using the waveThickness and spacing for each color
        SegWaves(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint16_t WaveThickness, 
                 uint16_t Spacing, uint8_t FadeSteps, bool Direct, uint16_t Rate);

        //Constructor for doing a single colored wave, using waveThickness and spacing
        SegWaves(SegmentSetPS &SegSet, CRGB Color, CRGB BgColor, uint16_t WaveThickness, uint16_t Spacing,
                 uint8_t FadeSteps, bool Direct, uint16_t Rate);

        //Constructor doing a rainbow based on the number of segments
        SegWaves(SegmentSetPS &SegSet, uint8_t FadeSteps, bool Direct, uint16_t Rate);

        ~SegWaves();

        uint8_t
            colorMode = 0,
            bgColorMode = 0,
            randMode = 0,
            fadeSteps;

        uint16_t
            cycleNum = 0;  // tracks what how many patterns we've gone through, for reference

        bool
            fadeOn = true,
            direct;

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
            makeSingleWave(),
            setPatternAsPattern(patternPS &inputPattern, uint16_t waveThickness, uint16_t spacing),
            setPaletteAsPattern(uint16_t waveThickness, uint16_t spacing),
            makeRainbowWaves(),
            reset(),
            resetSegColors(),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        uint8_t
            nextPattern,
            prevPattern,
            randPat = 255, //Used to tracking the current pattern value for randModes, starts as spacing, but will be set in the first update().
            blendStep = 0;

        uint16_t
            patternLength,
            nextPatternIndex,
            pixelNum,
            segNum,
            numSegs,
            numSegsLim,
            numSegsMax = 0,  //used for tracking the memory size of the segColors array
            handleDirect(uint16_t segNum);

        bool
            initFillDone = false;  //For filling in the segColors array in random modes

        CRGB
            *segColors = nullptr,
            nextColor,
            currentColor,
            colorOut,
            randColor,
            getNextColor(uint16_t segNum, uint16_t segNumRaw);

        void
            updateFade(),
            updateNoFade(),
            handleRandColors(),
            initFill(),
            init(CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate);
};

#endif