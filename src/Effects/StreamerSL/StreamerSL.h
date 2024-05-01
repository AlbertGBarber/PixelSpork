#ifndef StreamerSL_h
#define StreamerSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/* 
Moves a set of colored strips along a segment set, similar to Theater Chase, but with more options. 
The colors transition smoothly by blending, like a set of waves. The effect uses a pattern for colors. 
You can either input a custom pattern, or the effect can build one for you 
with a set color wave lengths and spaces between each wave. 

This effect is adapted for 2D use. The wave colors will be repeated down segment lines. 
For a version of this effect for whole segments, see SegWaves.h.

Because blending can take quite a bit of processing power, especially for longer segment sets, 
you can disable it using the `fadeOn` setting. If `fadeOn` is false, 
the waves will transition in one step, greatly simplifying the effect at the cost of some smoothness.
There is also a "fast" version of this effect, StreamerFastSL, which requires less processing power to run, \
but has a few extra restrictions. 

This effect supports Color Modes for both the streamer and background colors.

    Patterns:
        Patterns work the same as with other effects; they are a pattern of palette array indexes. 
        ie a pattern of {0, 2, 1} would be the first three colors of a palette. 
        However, in this effect, to indicate a background pixel (set it to the `bgColor`) we use 255 in the pattern. 
        Make sure your palette doesn't have 255 colors (that would be waaaay to big!).

        For example, lets say we wanted to do the first two colors of our palette, each as length 4 streamers, 
        with 3 background pixels in between each. We would make a pattern as: 
        `{0, 0, 0, 0, 255, 255, 255, 1, 1, 1, 1, 255, 255, 255}`.

        For simple patterns like the previous example, I have written a few constructors that automate the 
        pattern creation for you, taking the length of the colored streamers, and the length of the background spaces 
        as arguments (see constructor notes below). 

        Note that while each entry in the pattern is a uint8_t, if you have a lot of colors, 
        with long waves and spaces, your patterns may be quite large, so watch your memory usage. 

        Any automatically generated patterns will be allocated dynamically. 
        To avoid memory fragmentation, when you create the effect, you should set your color length and spacing 
        to the maximum values you expect to use, and then call `setPatternAsPattern()` or `setPaletteAsPattern()` 
        to resize your waves and spacing. See 
        https://github.com/AlbertGBarber/PixelSpork/wiki/Effects-Advanced#managing-dynamic-memory-and-fragmentation
        for more details. 

        Also, remember that the pattern length is limited to 65,025 (uint16_t max), 
        so make sure your `(colorLength + spacing) * <num palette colors>` is less than the limit. 

Example calls: 
    uint8_t pattern_arr = {0, 255, 255, 255, 1, 1, 255, 255};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};

    StreamerSL streamer(mainSegments, pattern, cybPnkPal_PS, 0, 30, 20);
    Will do a set of streamers using the first two colors from the cybPnkPal_PS palette, following the pattern above
    The streamer will begin with 1 pixel of color 0, with three spaces after, 
    followed by 2 pixels of color 1, followed by 2 spaces.
    The background is blank (0).
    The streamers will blend using 30 steps, with 20ms between each step.

    uint8_t pattern_arr = {0, 2, 1};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
    
    StreamerSL streamer(mainSegments, pattern, cybPnkPal_PS, 0, 3, 4, 0, 120);
    Will do streamers using the first three colors from the cybPnkPal_PS palette, following the pattern above.
    The background is blank (0).
    Each streamer will be length 3, followed by 4 spaces.
    The fade steps are set to zero, so there is no blending (fadeOn will be set false).
    The effect updates at a rate of 120ms

    StreamerSL streamer(mainSegments, cybPnkPal_PS, CRGB::Red, 3, 4, 10, 40);
    Will do streamers using all the colors in cybPnkPal_PS.
    The background is red.
    each streamer will be length 3, with 4 spaces in between.
    The streamers will blend using 10 steps, with 40ms between each step.

    StreamerSL streamer(mainSegments, CRGB::Blue, CRGB::Red, 2, 2, 0, 140);
    Will do blue streamers with length 2 and 2 spaces in between
    The background is red
    The fade steps are set to zero, so there is no blending (fadeOn will be set false).
    The effect updates at a rate of 140ms.
 
Constructor Inputs:
    pattern(optional, see constructors) -- The pattern used for the streamers, made up of palette indexes.
    palette(optional, see constructors) -- The repository of colors used in the pattern.
    color(optional, see constructors) -- Used for making single color streamers.
    colorLength (optional, see constructors) -- The length of a streamer. Used for automated pattern creation. 
    spacing (optional, see constructors) -- The number of pixels between each wave color (will be set to bgColor).  
                                            Used for automated pattern creation.
    bgColor -- The color of the spacing pixels. It is a pointer, so it can be tied to an external variable.
    fadeSteps -- The number of steps to blend from one streamer color to the next.
    rate -- The update rate (ms).

Other Settings:
    colorMode (default 0) -- sets the color mode for the streamer pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the spacing pixels (see segDrawUtils::setPixelColor)
    fadeOn (default true) -- If false, waves will use a single blend step, jumping directly from one color to the next.
                             This requires less processing power, but isn't as smooth.
                             Note that if 1 or 0 are passed as fadeSteps in a constructor, 
                             fadeOn will be set to false automatically
                             If you switch during run time, adjust your update rate to 
                             (fadeSteps * <your current rate>) for smoothness.

Functions:
    reset() -- Restarts the effect, you should call this if you change segment sets.
    setPatternAsPattern(&inputPattern, colorLength, spacing) -- Takes an input pattern and creates a streamer pattern from it using the current palette
                                                                Ex: uint8_t pattern_arr = {1, 2, 3};
                                                                    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
                                                                    setPatternAsPattern(pattern, 3, 4) 
                                                                    Will do a streamer using the first three colors of the palette (taken from the pattern)
                                                                    Each streamer will be length 3, followed by 4 spaces
    setPaletteAsPattern(colorLength, spacing) -- Like the previous function, but all of the current palette will be used for the pattern                                                       
    update() -- updates the effect
    
Reference Vars:
    cycleNum -- Tracks how many patterns we've gone through, 
                resets every pattern length number of cycles (ie once we've gone through the whole pattern)
*/
class StreamerSL : public EffectBasePS {
    public:
        //Constructor for using a passed in pattern and palette for the streamers
        StreamerSL(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BgColor, uint8_t FadeSteps, 
                   uint16_t Rate);  

        //Constructor for building the streamer pattern from the passed in pattern and the palette, using the passed in colorLength and spacing
        StreamerSL(SegmentSetPS &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BgColor, uint16_t ColorLength, 
                   uint16_t Spacing, uint8_t FadeSteps, uint16_t Rate);
        
        //Constructor for building a streamer using all the colors in the passed in palette, using the colorLength and spacing for each color
        StreamerSL(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint16_t ColorLength, uint16_t Spacing, 
                   uint8_t FadeSteps, uint16_t Rate);

        //Constructor for doing a single colored streamer, using colorLength and spacing
        StreamerSL(SegmentSetPS &SegSet, CRGB Color, CRGB BgColor, uint16_t ColorLength, uint16_t Spacing,
                   uint8_t FadeSteps, uint16_t Rate);

        ~StreamerSL();
            
        uint8_t
            colorMode = 0,
            bgColorMode = 0,
            fadeSteps;
        
        uint16_t 
            cycleNum = 0; //How many update cycles we've done, for reference
        
        bool 
            fadeOn = true;

        CRGB 
            bgColorOrig,
            *bgColor = nullptr; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color) 
        
        patternPS
            *pattern = nullptr,
            patternTemp = {nullptr, 0, 0}; //Must init structs w/ pointers set to null for safety
        
        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0}; //Must init structs w/ pointers set to null for safety
        
        void 
            init(CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate),
            setPatternAsPattern(patternPS &inputPattern, uint16_t colorLength, uint16_t spacing),
            setPaletteAsPattern(uint16_t colorLength, uint16_t spacing),
            reset(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint8_t
            nextPattern,
            blendStep = 0;
        
        uint16_t
            patternLength,
            nextPatternIndex,
            numSegs,
            numSegsMax = 0, ////used for tracking the memory size of the prevLineColors array
            numLines,
            numLinesLim,
            longestSeg,
            pixelNum;
        
        CRGB 
            *prevLineColors = nullptr,
            nextColor,
            colorOut,
            getNextColor(uint16_t lineNum, uint16_t segNum),
            getBlendedColor(CRGB nextColor, uint16_t segNum);
        
        void
            updateFade(),
            updateNoFade();
};

#endif