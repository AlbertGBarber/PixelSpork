#ifndef GradientCycleFastSegPS_h
#define GradientCycleFastSegPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

//Gradient cycle like GradientCycleSL can be done using color modes and the segment gradient pallet
//This effect is the seg version of GradientCycleFastSL

/* 
Moves a set of color gradients along the segment set, segment by segment
the gradients can be set to follow a pattern, use a palette, or set randomly
The gradients have a set length, and smoothly transition from one color to the next, wrapping at the end
If the total length of the gradients is longer than the segment set, they will still all transition on
whatever fits onto the segement set will be drawn at one time

The effect is adapted to work on whole segments, so each segment will be set to a single gradient color
which is shifted over time

Note that this effect should not be run alongside other effects on the same segmentSet
since it needs to use the existing colors of the leds

(Specifically the effect checks the color of the first led in each segment)

There are a few restrictions to the effect
1: Changing the palette on the fly will have a delayed effect on the colors
  The exisiting colors will shift off the strip before new ones shift on
  This prevents this effect from playing well with paletteBlend functions
2: The same restrictions as (1) apply to changing the pattern or the gradLength
3: Changing the direction of the segments or segment set mid-effect may break it temporarily

Basically the effect works by setting the color of the first segment, 
then for each subsequent segment, it copies the color of the next segment. (using the first pixel of each segment to get the color)
So any changes you make to colors will only show up at the first segment, and will be shifted along the segment set

However, as a bonus, this effect supports random colored gradients
where the colors for the gradients are choosen at random as the enter the strip
This is controlled by the randMode setting

Example calls: 
    uint8_t pattern_arr = {0, 1, 4};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
    GradientCycleFastSegPS(mainSegments, &pattern, &palette, 10, true, 100);
    Will do a gradient cycle from color 0, to color 1, to color 4, of the palette
    with 10 steps to each gradient, and a 100ms update rate
    The gradients will move from the last to the first segment

    GradientCycleFastSegPS(mainSegments, &palette, 10, false, 100);
    Will do a gradient cycle using the colors in the palette, with 10 steps to each gradient,and a 100ms update rate
    The gradients will move from the first to the last segment

    GradientCycleFastSegPS(mainSegments, 3, 15,, true, 80);
    Will do a gradient cycle using 3 randomly choosen colors, with 15 steps to each gradient,and an 80ms update rate
    note this is not the same as setting randMode, it just makes a random palette
    The gradients will move from the last to the first segment

Constructor Inputs:
    pattern(optional, see constructors) -- A pattern is struct made from a 1-d array of palette indexes ie {0, 1, 3, 6, 7} 
                                          and the length of the array 
                                          (see patternPS.h)   
    palette(optional, see constructors) -- The repository of colors used in the pattern, or can be used as the pattern itself
    numColors (optional, see contructors) -- The number of randomly choosen colors for the gradients
    gradLength -- How many steps for each gradient
    direct --  The effect direction, either from last to first segment (true), or visa versa
    Rate -- The update rate (ms)

Functions:
    setPaletteAsPattern() -- Sets the effect pattern to match the current palette
    setDirection() -- Sets the effect direction, either from last to first segment (true), or visa versa
    reset() -- Restarts the effect
    update() -- updates the effect

Other Settings:
    randMode (default 0) -- Sets the type of how colors are choosen:
                         -- 0: Colors will be choosen in order from the pattern (not random)
                         -- 1: Colors will be choosen completely at random
                         -- 2: Colors will be choosen randomly from the pattern (will not repeat the same color in a row)
                         --                                                     (unless your pattern has the same color in a row, like { 2, 2, 3})

Flags:
    initFillDone -- Flag for doing the initial fill of the gradients on the strip
                   Set true once the fill is done
                
Reference Vars:
    cycleNum -- Tracks how many cycles we've done, resets every gradLength cycles

Notes: 
*/
class GradientCycleFastSegPS : public EffectBasePS {
    public:
        //Constructor for using pattern
        GradientCycleFastSegPS(SegmentSetPS &SegmentSetPS, patternPS *Pattern, palettePS *Palette, uint8_t GradLength, bool Direct, uint16_t Rate); 

        //Constructor for using the palette as the pattern
        GradientCycleFastSegPS(SegmentSetPS &SegmentSetPS, palettePS *Palette, uint8_t GradLength, bool Direct, uint16_t Rate);

        //Constructor for using a random palette as the pattern
        GradientCycleFastSegPS(SegmentSetPS &SegmentSetPS, uint8_t NumColors, uint8_t GradLength, bool Direct, uint16_t Rate);

        ~GradientCycleFastSegPS();

        uint8_t
            randMode = 0,
            gradLength;
        
        uint16_t 
            cycleNum = 0; // tracks what how many patterns we've gone through

        bool
            direct, //reference only, set using setDirection()
            initFillDone = false;

        patternPS
            patternTemp,
            *pattern;

        palettePS
            paletteTemp,
            *palette;

        SegmentSetPS 
            &segmentSet; 
        
        void 
            setPaletteAsPattern(),
            setDirection(bool newDirect),
            reset(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        int8_t
            loopStep;
        
        int32_t
            endLimit,
            startLimit, 
            coloredSeg;

        uint8_t
            currentPattern,
            nextPattern;
        
        uint16_t
            numSegs,
            numSegsLim,
            pixelNum,
            patternCount = 0;

        CRGB 
            currentColor,
            nextColor,
            colorOut;
        
        void 
            init(uint16_t Rate),
            pickNextColor(),
            initalFill();    
};

#endif