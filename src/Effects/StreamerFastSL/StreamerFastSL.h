#ifndef StreamerFastPS_h
#define StreamerFastPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/* 
Similar to StreamerPS, but runs faster with some restrictions
Repeats a set of lines down the strip according to the passed in palette, shifting them forward over time
Like a theater chase, but a pattern is used instead of just dots.

This effect has been adapted to use segment lines to allow 2D effects. 

The restrictions when compared to StreamerPS are listed below:
1: There is no fading to shift the streamers along the strip
2: Changing the palette on the fly will have a delayed effect on the colors
  The existing colors will shift off the strip before new ones shift on
  This prevents this effect from playing well with paletteBlend functions
3: The same restrictions as (2) apply to changing the pattern
4: Changing the direction of the segments or segment set mid-effect may break it temporarily
5: This effect is not compatible with colorModes for either the streamers or the background

Basically the effect works by setting the color of the first pixel, 
then for each subsequent pixel, it copies the color of the next pixel in line
So any changes you make to colors will only show up at the first pixel, and will be shifted along the strip

However, as a bonus, this effect supports random colored streamer
where the colors for the streamers are choosen at random as the enter the strip
This is controlled by the randMode setting

Otherwise, all the settings are/functions are the same as StreamerPS

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
so watch your memory usage

Example calls: 
    uint8_t pattern_arr = {0, 255, 255, 255, 1, 1, 255, 255};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
    StreamerPS(mainSegments, pattern, palette3, 0, 120);
    Will do a set of streamers using the first two colors in the palette
    The streamer will begin with 1 pixel of color 0, with three spaces after, followed by 2 pixels of color 1, followed by 2 spaces
    The bgColor is zero (off)
    The streamers will update at a 120ms rate

    uint8_t pattern_arr = {1, 2, 3};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
    StreamerPS(mainSegments, pattern, palette3, 3, 4, 0, 120);
    Will do a streamer using the first three colors of the palette (taken from the pattern)
    Each streamer will be length 3, followed by 4 spaces, bgColor is 0 (off)
    The effect updates at a rate of 120ms

    StreamerPS(mainSegments, palette3, 3, 4, CRGB::Red, 120);
    Will do a streamer using all the colors in palette3, each streamer will be length 3, with 4 spaces in between
    The bgColor is red
    The streamers will update at a 120ms rate

    StreamerPS(mainSegments, CRGB::Blue, 2, 2, CRGB::Red, 0, 140);
    Will do a blue streamers with length 2 and 2 spaces in between
    The bgColor is red
    The effect updates at a rate of 140ms
 
Constructor Inputs:
    pattern(optional, see constructors) -- The pattern used for the streamers, made up of palette indexes 
    palette(optional, see constructors) -- The repository of colors used in the pattern
    color(optional, see constructors) -- Used for making a single color streamer
    colorLength (optional, see constructors, max 255) -- The number pixels a streamer color is. Used for automated pattern creation.
    spacing (optional, see constructors, max 255) -- The number of pixels between each streamer color (will be set to bgColor).  Used for automated pattern creation.
    bgColor -- The color of the spacing pixels. It is a pointer, so it can be tied to an external variable
    rate -- The update rate (ms)

Functions:
    reset() -- Restarts the streamer pattern
    setPatternAsPattern(&inputPattern, colorLength, spacing) -- Takes an input pattern and creates a streamer pattern from it using the current palette
                                                                Ex: uint8_t pattern_arr = {1, 2, 3};
                                                                   patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
                                                                   setPatternAsPattern(pattern, 3, 4) 
                                                                   Will do a streamer using the first three colors of the palette (taken from the pattern)
                                                                   Each streamer will be length 3, followed by 4 spaces
    setPaletteAsPattern(colorLength, spacing) -- Like the previous function, but all of the current palette will be used for the pattern                                                       
    update() -- updates the effect

Other Settings:
    randMode (default 0) -- Sets the type of how colors are choosen:
                         -- 0: Colors will be choosen in order from the pattern (not random)
                         -- 1: Colors will be choosen completely at random
                         -- 2: Colors will be choosen at random from the !!palette!!, 
                               but the same color won't be repeated in a row
                         -- 3: Colors will be choosen randomly from the pattern
                         -- 4: Colors will be choosen randomly from the !!palette!!
                               (option included b/c the pattern may have a lot of spaces, 
                                so choosing from it may be very biased)

Flags:
    initFillDone -- Indicates if the strip has been pre-filled with the effect's color outputs 
                    This needs to happen before running the first update cycle
                    If false, preFill() will be called when first updating
                    Set true once the first update cycle has been finished

Reference Vars:
    cycleNum -- How many update cycles we've done, resets every pattern length cycles (the effect has gone through the pattern once)

Notes:
    If the constructor made your pattern, it will be stored in patternTemp
    same goes for the palette 
*/
class StreamerFastSL : public EffectBasePS {
    public:
        //constructor for using the passed in pattern and palette for the streamer
        StreamerFastSL(SegmentSet &SegSet, patternPS &Pattern, palettePS &Palette, CRGB BgColor, uint16_t Rate);

        //constructor for building the streamer pattern from the passed in pattern and the palette, using the passed in colorLength and spacing
        StreamerFastSL(SegmentSet &SegSet, patternPS &Pattern, palettePS &Palette, uint8_t ColorLength, uint8_t Spacing, CRGB BgColor, uint16_t Rate);
        
        //constructor for building a streamer using all the colors in the passed in palette, using the colorLength and spacing for each color
        StreamerFastSL(SegmentSet &SegSet, palettePS &Palette, uint8_t ColorLength, uint8_t Spacing, CRGB BgColor, uint16_t Rate);

        //constructor for doing a single colored streamer, using colorLength and spacing
        StreamerFastSL(SegmentSet &SegSet, CRGB Color, uint8_t ColorLength, uint8_t Spacing, CRGB BgColor, uint16_t Rate);  
    
        ~StreamerFastSL();

        uint8_t
            randMode = 0;
        
        uint16_t
            cycleNum = 0;
            
        bool
            initFillDone = false;

        CRGB 
            bgColorOrig,
            *bgColor = nullptr; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        SegmentSet 
            &SegSet; 
        
        patternPS
            *pattern = nullptr,
            patternTemp = {nullptr, 0}; //Must init structs w/ pointers set to null for safety
        
        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0}; //Must init structs w/ pointers set to null for safety
        
        void 
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
            nextPatternRand,
            prevPattern;
        
        uint16_t
            numLines,
            numLinesLim,
            longestSeg,
            pixelNum;

        CRGB 
            nextColor,
            randColor = 0,
            pickStreamerColor(uint8_t nextPatternTemp);
        
        void
            initalFill(),
            init(CRGB BgColor, uint16_t Rate);
};  

#endif