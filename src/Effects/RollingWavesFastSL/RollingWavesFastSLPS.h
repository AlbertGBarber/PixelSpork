#ifndef RollingWavesFastSLPS_h
#define RollingWavesFastSLPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/*
Repeats a set of waves along the segment set according to the input pattern and palette
Each wave is a gradient that shifts towards black (off)
The waves are configured to rise and fall across a set length (the gradLength var)
They can be set to only have a rise, fall, or both using the trailMode var
How quickly the wave dims can be controlled by the dimPow var
The default value of dimPow is 120, this gives a nice, water-like shimmer to the waves
A spacing can be added between the waves, which can be set to a background color

The effect is adapted to work on segment lines for 2D use, but you can keep it 1D by
passing in a segmentSet with only one segment containing the whole strip.

Note that this effect should not be run alongside other effects on the same segmentSet
since it needs to use the existing colors of the leds

Note that for full waves, odd numbered gradLengths work the best

This effect is the same as RollingWavesPS, but should need fewer caculations per cycle
because it only works out the colors once. It may take up more programing space.
however it does have a few extra restrictions
1: Changing the palette on the fly will have a delayed effect on the colors
   The exisiting colors will shift off the strip before new ones shift on
   This prevents this effect from playing well with paletteBlend functions
2: The same restrictions as (1) apply to changing the pattern, the gradLength, the spacing, or trailModes
3: Changing the direction of the segments or segment set mid-effect may break it temporarily
4: Color modes are disabled

Basically the effect works by setting the color of the first pixel, then for each subsequent pixel,
it copies the color of the next pixel in line
So any changes you make to colors will only show up at the first pixel, and will be shifted along the strip

However, as a bonus, this effect supports random colored waves
where the colors for the waves are choosen at random as the enter the strip
This is controlled by the randMode setting

Example calls: 
    uint8_t pattern_arr = {0, 1, 4};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
    RollingWavesFastSLPS(mainSegments, &pattern, &palette, 0, 7, 1, 0, 100);
    Will do a set of waves according to the pattern, with a blank background
    each wave will be 7 pixels long, using both types of trails
    there will be zero spacing between the waves
    The effect will update at a 100ms

    RollingWavesFastSLPS(mainSegments, &palette, 0, 9, 0, 2, 80);
    Will do a set of waves matching the input palette with an blank background
    Each wave will be 9 pixels long, the wave will consist of the trailing portion only
    There will be two spaces inbetween each wave,
    The effect will update at 80ms

    RollingWavesFastSLPS(mainSegments, 1, CRGB::Red, 12, 1, 3, 80);
    Will do a set of waves of a single color choosen at random, with a red background
    Each wave will a length of 12, and will contain only the leading portion of the wave
    There will be 3 background spaces between each wave
    The effect will update at 80ms

Constructor Inputs:
    pattern(optional, see constructors) -- A pattern is struct made from a 1-d array of palette indexes ie {0, 1, 3, 6, 7} 
                                           and the length of the array 
                                           (see patternPS.h)   
    palette(optional, see constructors) -- The repository of colors used in the pattern, or can be used as the pattern itself
    numColors (optional, see contructors) -- The number of randomly choosen colors for the gradients
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

Functions:
    setPalette(*newPalette) -- Sets the palette used for the waves
    setTotalEffectLength() -- Calculates the total length of all the waves in the pattern (incl spacing), you shouldn't need to call this
    setPattern(*newPattern) -- Sets the passed in pattern to be the effect pattern
                               Will force setTotalEffectLength() call, so may cause effect to jump
    setPaletteAsPattern() -- Sets the effect pattern to match the current palette (calls setTotalEffectLength())
    setGradLength(newGradLength) -- Changes the gradlength to the specified value, adjusting the length of the waves (calls setTotalEffectLength())
    setSpacing(newSpacing) -- Changes the spacing to the specified value, (calls setTotalEffectLength())
    setTrailMode(newTrailMode) -- Changes the trail mode used for the waves
    buildLineArr() -- Creates the array for storing the line pixel locations
                      !! Only call this if you change your segment set
    update() -- updates the effect

Other Settings:
    randMode (default 0) -- Sets how colors are choosen from the palette
                         -- 0: Colors will be choosen from the palette in order (not random)
                         -- 1: Colors will be choosen completely at random
                         -- 2: Colors will be choosen randomly from the pattern (will not repeat the same color in a row)

    dimPow (default 120, max 255) -- Adjusts the rate of dimming for the wave trails
                                     255 will do typical linear dimming
                                     While anything below will dim faster
                                     120 sets a good balance of brightness, will not dimming most colors to 0 before the end of the wave

Flags:
    initFillDone -- Flag for doing the initial fill of the gradients on the strip
                    Set true once the fill is done

Notes:
*/
class RollingWavesFastSLPS : public EffectBasePS {
    public:
        //Constructor with pattern
        RollingWavesFastSLPS(SegmentSet &SegmentSet, patternPS *Pattern, palettePS *Palette, CRGB BGColor, uint8_t GradLength, uint8_t TrailMode, uint8_t Spacing, uint16_t Rate); 

        //Constuctor with palette as pattern
        RollingWavesFastSLPS(SegmentSet &SegmentSet, palettePS *Palette, CRGB BGColor, uint8_t GradLength, uint8_t TrailMode, uint8_t Spacing, uint16_t Rate);

        //Constructor with random colors
        RollingWavesFastSLPS(SegmentSet &SegmentSet, uint8_t NumColors, CRGB BGColor, uint8_t GradLength, uint8_t TrailMode, uint8_t Spacing, uint16_t Rate);

        ~RollingWavesFastSLPS();

        uint8_t
            randMode = 0,
            dimPow = 120, //120
            spacing,
            trailMode,
            blendStep, //the step of the current blend, at 0, a new blend will start, for reference
            gradLength;
        
        uint16_t 
            totalCycleLength, //total length of all the gradients combined, for reference
            cycleNum = 0; // tracks what how many patterns we've gone through, for reference
        
        bool
            initFillDone = false;
        
        CRGB 
            bgColorOrig,
           *bgColor; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        patternPS
            patternTemp,
            *pattern;

        palettePS
            paletteTemp,
            *palette;

        SegmentSet 
            &segmentSet; 

        void 
            setGradLength(uint8_t newGradLength),
            setSpacing(uint8_t newSpacing),
            setPattern(patternPS *newPattern),
            setTrailMode(uint8_t newTrailMode),
            setPaletteAsPattern(),
            setTotalEffectLength(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint8_t
            dimRatio,
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
            getWaveColor(uint8_t step),
            desaturate(CRGB color, uint8_t step, uint8_t totalSteps),
            currentColor,
            colorOut;
        
        void
            init(CRGB BgColor, uint16_t Rate),
            initalFill(),
            setNextColors(uint16_t segPixelNum);       
};

#endif