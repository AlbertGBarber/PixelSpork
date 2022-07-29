#ifndef GradientCyclePS_h
#define GradientCyclePS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/* 
Moves a set of color gradients along the segment set
the gradients can be set to follow a pattern, use a palette, or set randomly
The gradients have a set length, and smoothly transition from one color to the next, wrapping at the end
If the total length of the gradients is longer than the segment set, they will still all transition on
whatever fits onto the segement set will be drawn at one time

There is a version of this effect that takes less CPU power (GradientCycleLowPS)
It has a few restrictions, but should run faster than this effect

Example calls: 
    uint8_t pattern_arr = {0, 1, 4};
    patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
    GradientCyclePS(mainSegments, &pattern, &palette, 10, 100);
    Will do a gradient cycle from color 0, to color 1, to color 4, of the palette
    with 10 steps to each gradient, and a 100ms update rate

    GradientCyclePS(mainSegments, &palette, 10, 100);
    Will do a gradient cycle using the colors in the palette, with 10 steps to each gradient,and a 100ms update rate

    GradientCyclePS(mainSegments, 3, 15, 80);
    Will do a gradient cycle using 3 randomly choosen colors, with 15 steps to each gradient,and an 80ms update rate
 
Constructor Inputs:
    Pattern(optional, see constructors) -- A pattern is struct made from a 1-d array of palette indexes ie {0, 1, 3, 6, 7} 
                                          and the length of the array 
                                          (see patternPS.h)   
    Palette(optional, see constructors) -- The repository of colors used in the pattern, or can be used as the pattern itself
    NumColors (optional, see contructors) -- The number of randomly choosen colors for the gradients
    GradLength -- How many steps for each gradient
    Rate -- The update rate (ms)

Functions:
    setPalette(*newPalette) -- Sets the palette used for the gradient
    setTotalEffectLength() -- Calculates the total length of all the gradients combined, you shouldn't need to call this
    setPattern(*newPattern) -- Sets the passed in pattern to be the effect pattern
                              Will force setTotalEffectLength() call, so may cause effect to jump
    setPaletteAsPattern() -- Sets the effect pattern to match the current palette (calls setTotalEffectLength())
    setGradLength(newGradLength) -- Changes the Gradlength to the specified value, adjusting the length of the gradients (calls setTotalEffectLength())
    update() -- updates the effect

Notes:
    For the randomly generated gradient constructor, the random palette can be accessed via paletteTemp

    If using the palette as the pattern, if you change the palette, you'll need to change the pattern as well
    (unless the palettes are the same length) 
*/
class GradientCyclePS : public EffectBasePS {
    public:
        //Constructor for using pattern
        GradientCyclePS(SegmentSet &SegmentSet, patternPS *Pattern, palettePS *Palette, uint8_t GradLength, uint16_t Rate); 

        //Constructor for using the palette as the pattern
        GradientCyclePS(SegmentSet &SegmentSet, palettePS *Palette, uint8_t GradLength, uint16_t Rate);

        //Constructor for using a random palette as the pattern
        GradientCyclePS(SegmentSet &SegmentSet, uint8_t NumColors, uint8_t GradLength, uint16_t Rate);

        ~GradientCyclePS();

        uint8_t
            gradLength;
        
        uint16_t 
            totalCycleLength, //total length of all the gradients combined, for reference
            cycleNum = 0; // tracks what how many patterns we've gone through

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
            setPalette(palettePS* newPalette),
            setPattern(patternPS *newPattern),
            setPaletteAsPattern(),
            setTotalEffectLength(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint8_t
            nextPattern, 
            currentPattern,
            currentColorIndex,
            blendStep;
        
        uint16_t
            step,
            numPixels;

        CRGB 
            currentColor,
            nextColor,
            colorOut;
        
        void 
            init(uint16_t Rate),
            setnextColors(uint16_t pixelNum);
        
};

#endif