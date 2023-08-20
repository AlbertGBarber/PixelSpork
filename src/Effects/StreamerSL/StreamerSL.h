#ifndef StreamerSL_h
#define StreamerSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/* 
Repeats a set of lines down the strip according to the passed in palette, shifting them forward over time
Like a theater chase, but a pattern is used instead of just dots
Each streamer blends as it moves forwards, for a smoother effect
Since the blending takes a bit of processing power, you can disable it with fadeOn
For an even faster version of this effect, see StreamerFastPS

Patterns work the same as with other effects, they are a pattern of palette array indexes
ie a pattern of {0, 1, 2} would be the first three colors of a palette
to indicate a background pixel (ie set to the BgColor) we use 255 in the pattern
This does mean if your palette has 255 colors, you'll lose the final color, but you shouldn't have palettes that large

For example, lets say we wanted to do the first two colors of our palette,
each as length 4 streamers, with 3 background leds in between each
we would make a pattern as : {0, 0, 0, 0, 255, 255, 255, 1, 1, 1, 1, 255, 255, 255}

For simple patterns like the previous example, I have written a few constructors for the effect
that automate the pattern creation, so you don't have to do it yourself (see constructor notes below)

Note that while each entry in the pattern is a uint8_t,
if you have a lot of colors, with long streamers, your patterns may be quite large
so watch your memory usage. Likewise, if you re-size the waves, the pattern may also be dynamically re-sized.
(see alwaysResizeObjPS in Include_Lists -> GlobalVars, and the Effects Advanced Wiki Page -> Managing Memory Fragmentation)

This effect has been adapted to use segment lines to allow 2D effects. 
Note that this requires an array for storing some colors, so if you change the number of segments
in your segment set make sure to call reset or the effect will crash!!!!
The array's length is the number of segment in the segment set.

This effect is fully compatible with color modes, and the bgColor is a pointer, so you can bind it
to an external color variable.

Notes:
    You can change the palette, and patterns on the fly, but there's no way to smoothly transition
    between patterns, so there will possibly be a jump.

    If the constructor made your pattern, it will be stored in patternTemp
    same goes for the palette.

    Turning off fading using fadeOn should improve performance. The effect will not be as smooth, but should still look
    pretty good. Note that you'll need to set your new update rate to (fadeSteps * your current rate) to keep the same
    overall update rate.

Example calls: 
    uint8_t pattern_arr = {0, 255, 255, 255, 1, 1, 255, 255};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
    StreamerSL streamer(mainSegments, pattern, cybPnkPal, 0, 30, 20);
    Will do a set of streamers using the first two colors in the palette
    The streamer will begin with 1 pixel of color 0, with three spaces after, followed by 2 pixels of color 1, followed by 2 spaces
    The bgColor is zero (off)
    The streamers will blend forward, taking 30 steps, with 20ms between each step

    uint8_t pattern_arr = {1, 2, 3};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
    StreamerSL streamer(mainSegments, pattern, cybPnkPal, 3, 4, 0, 0, 120);
    Will do a streamer using the first three colors of the palette (taken from the pattern)
    Each streamer will be length 3, followed by 4 spaces, bgColor is 0 (off)
    The fade steps are set to zero, so there is no blending, 
    The effect updates at a rate of 120ms

    StreamerSL streamer(mainSegments, cybPnkPal, 3, 4, CRGB::Red, 10, 40);
    Will do a streamer using all the colors in cybPnkPal, each streamer will be length 3, with 4 spaces in between
    The bgColor is red
    The streamer will blend forward, taking 10 steps, with 40ms between each step

    StreamerSL streamer(mainSegments, CRGB::Blue, 2, 2, CRGB::Red, 0, 140);
    Will do a blue streamers with length 2 and 2 spaces in between
    The bgColor is red
    The fade steps are set to zero, so there is no blending
    The effect updates at a rate of 140ms
 
Constructor Inputs:
    pattern(optional, see constructors) -- The pattern used for the streamers, made up of palette indexes 
    palette(optional, see constructors) -- The repository of colors used in the pattern
    color(optional, see constructors) -- Used for making a single color streamer
    colorLength (optional, see constructors, max 255) -- The number pixels a streamer color is. Used for automated pattern creation.
    spacing (optional, see constructors, max 255) -- The number of pixels between each streamer color (will be set to bgColor).  Used for automated pattern creation.
    bgColor -- The color of the spacing pixels. It is a pointer, so it can be tied to an external variable
    fadeSteps -- The number of steps to transition from one color to the next as the streamers move down the strip
    rate -- The update rate (ms)

Functions:
    reset() -- Restarts the streamer pattern, you should call this if you change segment sets as well.
    setPatternAsPattern(&inputPattern, colorLength, spacing) -- Takes an input pattern and creates a streamer pattern from it using the current palette
                                                                Ex: uint8_t pattern_arr = {1, 2, 3};
                                                                    patternPS pattern = {pattern_arr, SIZE(pattern_arr), SIZE(pattern_arr)};
                                                                    setPatternAsPattern(pattern, 3, 4) 
                                                                    Will do a streamer using the first three colors of the palette (taken from the pattern)
                                                                    Each streamer will be length 3, followed by 4 spaces
    setPaletteAsPattern(uint8_t colorLength, uint8_t spacing) -- Like the previous function, but all of the current palette will be used for the pattern                                                       
    update() -- updates the effect

Other Settings:
    colorMode (default 0) -- sets the color mode for the streamer pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the spacing pixels (see segDrawUtils::setPixelColor)
    fadeOn (default true) -- If false, the streamer will jump directly to the next color instead of fading
                             Note that if 1 or 0 are passed in as the FadeSteps in the constructor, 
                             fadeOn will be set to false automatically
    
Reference Vars:
    cycleNum -- How many update cycles we've done, resets every pattern length number of cycles (the pattern has been streamed once)

*/
class StreamerSL : public EffectBasePS {
    public:
        //Constructor for using the passed in pattern and palette for the streamer
        StreamerSL(SegmentSet &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BgColor, uint8_t FadeSteps, uint16_t Rate);  

        //Constructor for building the streamer pattern from the passed in pattern and the palette, using the passed in colorLength and spacing
        StreamerSL(SegmentSet &SegSet, patternPS &Pattern, palettePS &Palette, uint8_t ColorLength, uint8_t Spacing, CRGB BgColor, uint8_t FadeSteps, uint16_t Rate);
        
        //Constructor for building a streamer using all the colors in the passed in palette, using the colorLength and spacing for each color
        StreamerSL(SegmentSet &SegSet, palettePS &Palette, uint8_t ColorLength, uint8_t Spacing, CRGB BgColor, uint8_t FadeSteps, uint16_t Rate);

        //Constructor for doing a single colored streamer, using colorLength and spacing
        StreamerSL(SegmentSet &SegSet, CRGB Color, uint8_t ColorLength, uint8_t Spacing, CRGB BgColor, uint8_t FadeSteps, uint16_t Rate);

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
            *prevLineColors = nullptr,
            bgColorOrig,
            *bgColor = nullptr; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color) 
        
        patternPS
            *pattern = nullptr,
            patternTemp = {nullptr, 0, 0}; //Must init structs w/ pointers set to null for safety
        
        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0}; //Must init structs w/ pointers set to null for safety
        
        void 
            init(CRGB BgColor, SegmentSet &SegSet, uint16_t Rate),
            setPatternAsPattern(patternPS &inputPattern, uint8_t colorLength, uint8_t spacing),
            setPaletteAsPattern(uint8_t colorLength, uint8_t spacing),
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
            nextColor,
            colorOut,
            getNextColor(uint16_t lineNum, uint16_t segNum),
            getBlendedColor(CRGB nextColor, uint16_t segNum);
        
        void
            updateFade(),
            updateNoFade();
};

#endif