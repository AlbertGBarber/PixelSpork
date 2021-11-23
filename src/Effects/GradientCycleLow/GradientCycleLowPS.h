#ifndef GradientCycleLowPS_h
#define GradientCycleLowPS_h

#include "Effects/EffectBasePS.h"
#include "Effects/EffectUtils/EffectUtilsPS.h"

//Moves a set of color gradients along the segment set
//the gradients can be set to follow a pattern, use a pallet, or set randomly
//The gradients have a set length, and smoothly transition from one color to the next, wrapping at the end
//If the total length of the gradients is longer than the segment set, they will still all transition on
//whatever fits onto the segement set will be drawn at one time

//Note that this effect should not be run alongside other effects on the same segmentSet
//since it needs to use the existing colors of the leds

//This effect is the same as GradientCyclePS, but should need fewer caculations per cycle
//however it does have a few extra restrictions
//1: Changing the pallet on the fly will have a delayed effect on the colors
//   The exisiting colors will shift off the strip before new ones shift on
//   This prevents this effect from playing well with palletBlend functions
//2: The same restrictions as (1) apply to changing the pattern of the gradLength
//3: Changing the direction of the segments or segment set mid-effect may break it temporarily
//4: Depending on the length, you may have a temporary blend "jump" at the beginning of the strip
//   This will along the strip before disappering

//Example calls: 
    //uint8_t pattern_arr = {0, 1, 4};
    //patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
    //GradientCycleLowPS(mainSegments, &pattern, &pallet, 10, 100);
    //Will do a gradient cycle from color 0, to color 1, to color 4, of the pallet
    //with 10 steps to each gradient, and a 100ms update rate

    //GradientCycleLowPS(mainSegments, &pallet, 10, 100);
    //Will do a gradient cycle using the colors in the pallet, with 10 steps to each gradient,and a 100ms update rate

    //GradientCycleLowPS(mainSegments, 3, 15, 80);
    //Will do a gradient cycle using 3 randomly choosen colors, with 15 steps to each gradient,and an 80ms update rate
 
//Constructor Inputs:
    //Pattern(optional, see constructors) -- A pattern is struct made from a 1-d array of pallet indexes ie {0, 1, 3, 6, 7} 
    //                                       and the length of the array 
    //                                       (see patternPS.h)   
    //Pallet(optional, see constructors) -- The repository of colors used in the pattern, or can be used as the pattern itself
    //NumColors (optional, see contructors) -- The number of randomly choosen colors for the gradients
    //GradLength -- How many steps for each gradient
    //Rate -- The update rate (ms)

//Functions:
    //setPallet(*newPallet) -- Sets the pallet used for the gradient
    //setPattern(*newPattern) -- Sets the passed in pattern to be the effect patten
    //setPalletAsPattern() -- Sets the effect pattern to match the current pallet
    //reset() -- Restarts the effect
    //update() -- updates the effect

//Flags:
    //initFillDone -- Flag for doing the initial fill of the gradients on the strip
    //                Set true once the fill is done

//Notes:

class GradientCycleLowPS : public EffectBasePS {
    public:
        GradientCycleLowPS(SegmentSet &SegmentSet, patternPS *Pattern, palletPS *Pallet, uint8_t GradLength, uint16_t Rate); 

        GradientCycleLowPS(SegmentSet &SegmentSet, palletPS *Pallet, uint8_t GradLength, uint16_t Rate);

        GradientCycleLowPS(SegmentSet &SegmentSet, uint8_t NumColors, uint8_t GradLength, uint16_t Rate);

        ~GradientCycleLowPS();

        uint8_t
            gradLength;
        
        uint16_t 
            cycleNum = 0; // tracks what how many patterns we've gone through

        bool
            initFillDone = false;

        patternPS
            patternTemp,
            *pattern;

        palletPS
            palletTemp,
            *pallet;

        SegmentSet 
            &segmentSet; 
        
        void 
            setPallet(palletPS* newPallet),
            setPattern(patternPS *newPattern),
            setPalletAsPattern(),
            reset(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;

        uint8_t
            currentPattern,
            nextPattern;
        
        uint16_t
            pixelNumber,
            nextPixelNumber,
            patternCount = 0,
            numPixels;

        CRGB 
            currentColor,
            nextColor,
            colorOut;
        
        void 
            init(uint16_t Rate),
            initalFill();    
};

#endif