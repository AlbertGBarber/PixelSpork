#ifndef SegWaves_h
#define SegWaves_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
//#include "MathUtils/mathUtilsPS.h"

/* 
Colors each segment in a palette color, fading them to each other over time
Like a set of waves moving in a direction
It is a segment based version of the streamer effect
Each wave blends as it moves forwards, for a smoother effect
Since the blending takes a bit of processing power, you can disable it with fadeOn
For an even faster version of this effect, see SegWaveFast

The direction of the waves is controlled using the direct var. 
True will make the waves move from the last to first segment, false, the reverse.

Patterns work the same as with other effects, they are a pattern of palette array indexes
ie a pattern of {0, 1, 2} would be the first three colors of a palette
to indicate a background pixel (ie set to the BgColor) we use 255 in the pattern
This does mean if your palette has 255 colors, you'll lose the final color, but you shouldn't have palettes that large

For example, lets say we wanted to do the first two colors of our palette,
each as length 4 waves, with 3 background leds inbetween each
we would make a pattern as : {0, 0, 0, 0, 255, 255, 255, 1, 1, 1, 1, 255, 255, 255}

For simple patterns like the previous example, I have writtern a few constructors for the effect
that automate the pattern creation, so you don't have to do it yourself (see constructor notes below)

I have also included a shortcut for making a single wave (a wave of length 1, with only one wave active on the segment set a once)
For any of the constructors with a waveThickness input, just pass in 0, and a single wave pattern will be build automatically.

Likewise I've added a rainbow constructor, which will create a smooth rainbow transition wave.

The effect also has a random mode (set using randMode, not in the constructors),
where the pattern color are choosen at random as they enter the segments
The modes are: 
    0 (dfault): Colors will be choosen in order from the pattern (not random)
    1: Colors will be choosen completely at random
    2: Colors will be choosen randomly from the palette (allowing repeats)
    3: Colors will be choosen at random from the palette,
       but the same color won't be repeated in a row

Note that switching from random modes to the fixed pattern mode (randMode 0) will cause a jump in colors
Also if you change the number of segments in your segment set, you'll need to reset the effect when using a random mode
(The random mode needs to use an array of lenght numSegs + 1)

Note that while each entry in the pattern is a uint8_t,
if you have a lot of colors, with long waves, your patterns may be quite large
so watch your memory usage

Likewise, the effect needs to store a CRGB array of size numSegs + 1 for random modes.

This effect is fully compatible with color modes, but to help make the effect faster it's only
compatible with modes 2, 4, 5, 7, 9, and 10. In random modes, the colors do not change once the enter the segments
but the color modes will work as need colors are added.

The bgColor is a pointer, so you can bind it to an external color variable.

Example calls: 
    uint8_t pattern_arr = {0, 255, 255, 255, 1, 1, 255, 255};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
    SegWaves(mainSegments, &pattern, &palette3, 0, 30, true, 20);
    Will do a set of waves using the first two colors in the palette
    The wave will begin with 1 pixel of color 0, with three spaces after, followed by 2 pixels of color 1, followed by 2 spaces
    The bgColor is zero (off)
    The waves will blend, taking 30 steps, with 20ms between each step
    The waves will move from the last segment to the first

    uint8_t pattern_arr = {1, 2, 3};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
    SegWaves(mainSegments, &pattern, &palette3, 3, 4, 0, 0, false, 120);
    Will do a wave using the first three colors of the palette (taken from the pattern)
    Each wave will be length 3, followed by 4 spaces, bgColor is 0 (off)
    The fade steps are set to zero, so there is no blending.
    The waves will move from the first to last segment.
    The effect updates at a rate of 120ms

    SegWaves(mainSegments, &palette3, 0, 0, CRGB::Red, 10, true, 40);
    Will do a wave using all the colors in palette3,
    because the passed in waveThickness is 0, the effect will be configured as to create a pattern of single waves
    (wave thickness of 1, with spacing such that there's only one wave on the segment sets at one time)
    The bgColor is red
    The waves will blend forward, taking 10 steps, with 40ms between each step
    The waves will move from the last segment to the first

    SegWaves(mainSegments, CRGB::Blue, 2, 2, CRGB::Red, 0, true, 140);
    Will do a blue waves with length 2 and 2 spaces inbetween
    The bgColor is red
    The fade steps are set to zero, so there is no blending
    The effect updates at a rate of 140ms
    The waves will move from the last segment to the first

    SegWaves(mainSegments, 10, false, 80);
    Will do a rainbow wave set, with 10 blend steps for the rainbow
    The waves will move from the first to last segment.
    The effect updates at a rate of 80ms
 
Constructor Inputs:
    Pattern(optional, see constructors) -- The pattern used for the waves, made up of palette indexes 
    Palette(optional, see constructors) -- The repository of colors used in the pattern
    Color(optional, see constructors) -- Used for making a single color wave
    WaveThickness (optional, see constructors, max 255) -- The number pixels a wave color is. Used for automated pattern creation.
                                                           Passing 0 will create a pattern of single length waves (see intro above)
    Spacing (optional, see constructors, max 255) -- The number of pixels between each wave color (will be set to bgColor).  Used for automated pattern creation.
    bgColor -- The color of the spacing pixels. It is a pointer, so it can be tied to an external variable
    fadeSteps -- The number of steps to transition from one color to the next as the waves move down the strip
                 Passing 0 or 1 will set fadeOn to false (no fading)
    direct --  The direction the wave move
               True will make the waves move from the last to first segment, false, the reverse.
    Rate -- The update rate (ms)

Functions:
    reset() -- Restarts the wave pattern (also calls resetSegColors())
    resetSegColors() -- Only neeeded for random modes when changing the number of segments in your set. 
                        Re-creates the segColors array, as used by random modes
    setPatternAsPattern(*inputPattern, colorLength, spacing) -- Takes an input pattern and creates a wave pattern from it using the current palette
                                                                Ex: uint8_t pattern_arr = {1, 2, 3};
                                                                   patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
                                                                   setPatternAsPattern(&pattern, 3, 4) 
                                                                   Will do a wave using the first three colors of the palette (taken from the pattern)
                                                                   Each wave will be length 3, followed by 4 spaces
    setPaletteAsPattern(uint8_t colorLength, uint8_t spacing) -- Like the previous function, but all of the current palette will be used for the pattern                                                       
    makeSingleWave() -- Creates a wave pattern so that there's only a single wave of thickness 1 active on the segment set at one time
    update() -- updates the effect

Other Settings:
    colorMode (default 0) -- sets the color mode for the wave pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the spacing pixels (see segDrawUtils::setPixelColor)
    fadeOn (default true) -- If false, the wave will jump directly to the next color instead of fading
                             Note that if 1 or 0 are passed in as the FadeSteps in the constructor, 
                             fadeOn will be set to false automatically
    randMode (default 0) -- Sets the type of how colors are choosen:
                         -- 0: Colors will be choosen in order from the pattern (not random)
                         -- 1: Colors will be choosen completely at random
                         -- 2: Colors will be choosen randomly from the palette (allowing repeats)
                         -- 3: Colors will be choosen at random from the palette,
                               but the same color won't be repeated in a row

Notes:
    You can change the palette, and patterns on the fly, but there's no way to smoothly transition
    between patterns, so there will possibly be a jump

    Swapping from random modes to the fixed pattern (randMode 0) may create a jump in colors

    If the constructor made your pattern, it will be stored in patternTemp
    same goes for the palette

    Turning off fading using fadeOn should improve performance. The effect will not be as smooth, but should still look
    pretty good. Note that you'll need to set your new update rate to (fadeSteps * <your current rate>) to keep the same
    overall update rate

    Needs to store a CRGB array of size numSegs + 1.
*/
class SegWaves : public EffectBasePS {
    public:
        //constructor for using the passed in pattern and palette for the wave
        SegWaves(SegmentSet &SegmentSet, patternPS *Pattern, palettePS *Palette, CRGB BgColor, uint8_t FadeSteps, bool Direct, uint16_t Rate);  

        //constructor for building the wave pattern from the passed in pattern and the palette, using the passed in waveThickness and spacing
        SegWaves(SegmentSet &SegmentSet, patternPS *Pattern, palettePS *Palette, uint8_t WaveThickness, uint8_t Spacing, CRGB BgColor, uint8_t FadeSteps, bool Direct, uint16_t Rate);

        //constructor for building a wave using all the colors in the passed in palette, using the waveThickness and spacing for each color
        SegWaves(SegmentSet &SegmentSet, palettePS *Palette, uint8_t WaveThickness, uint8_t Spacing, CRGB BgColor, uint8_t FadeSteps, bool Direct, uint16_t Rate);
    
        //constructor for doing a single colored wave, using waveThickness and spacing
        SegWaves(SegmentSet &SegmentSet, CRGB Color, uint8_t WaveThickness, uint8_t Spacing, CRGB BgColor, uint8_t FadeSteps, bool Direct, uint16_t Rate);
    
        //constructor doing a rainbow based on the number of segments
        SegWaves(SegmentSet &SegmentSet, uint8_t FadeSteps, bool Direct, uint16_t Rate);

        ~SegWaves();

        uint8_t
            colorMode = 0,
            bgColorMode = 0,
            randMode = 0,
            fadeSteps;

        CRGB 
            bgColorOrig,
           *bgColor; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        SegmentSet 
            &segmentSet; 
        
        patternPS
            patternTemp,
            *pattern;
        
        palettePS
            paletteTemp,
            *palette;
        
        bool 
            fadeOn = true,
            direct;
        
        void 
            makeSingleWave(),
            setPatternAsPattern(patternPS *inputPattern, uint8_t waveThickness, uint8_t spacing),
            setPaletteAsPattern(uint8_t waveThickness, uint8_t spacing),
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
            blendStep = 0,
            cycleCount = 0;
        
        uint16_t
            patternLength,
            nextPatternIndex,
            pixelNum,
            segNum,
            numSegs;
        
        bool
            initFillDone = false; //For filling in the segColors array in random modes

        CRGB 
            nextColor,
            currentColor,
            colorOut,
            randColor,
            *segColors,
            getNextColor(uint16_t pixelNum);
        
        void
            updateFade(),
            updateNoFade(),
            handleRandColors(),
            initFill(),
            init(CRGB BgColor, uint16_t Rate);
};

#endif