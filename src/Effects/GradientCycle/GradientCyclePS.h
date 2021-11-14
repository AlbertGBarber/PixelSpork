#ifndef GradientCyclePS_h
#define GradientCyclePS_h

#include "Effects/EffectBasePS.h"
#include "Effects/EffectUtils/EffectUtilsPS.h"

//Moves a set of color gradients along the segment set
//the gradients can be set to follow a pattern, use a pallet, or set randomly
//The gradients have a set length, and smoothly transition from one color to the next, wrapping at the end
//If the total length of the gradients is longer than the segment set, they will still all transition on
//whatever fits onto the segement set will be drawn at one time

//There is a version of this effect that takes less CPU power (GradientCycleLowPS)
//It has a few restrictions, but should run faster than this effect

//Example calls: 
    //uint8_t pattern = {0, 1, 4};
    //GradientCyclePS(mainSegments, pattern, SIZE(pattern), &pallet, 10, 100);
    //Will do a gradient cycle from color 0, to color 1, to color 4, of the pallet
    //with 10 steps to each gradient, and a 100ms update rate

    //GradientCyclePS(mainSegments, &pallet, 10, 100);
    //Will do a gradient cycle using the colors in the pallet, with 10 steps to each gradient,and a 100ms update rate

    //GradientCyclePS(mainSegments, 3, 15, 80);
    //Will do a gradient cycle using 3 randomly choosen colors, with 15 steps to each gradient,and an 80ms update rate
 
//Constructor Inputs:
    //Pattern(optional, see constructors) -- A pattern is a 1-d array of pallet indexes ie {0, 1, 3, 6, 7} 
    //                                       the pattern is faded through in order, wrapping at the end
    //                                       so the above example would fade from the first(0) color in the pallet to the second, to the fourth, etc
    //PatternLength -- the length of the pattern above
    //Pallet(optional, see constructors) -- The repository of colors used in the pattern, or can be used as the pattern itself
    //NumColors (optional, see contructors) -- The number of randomly choosen colors for the gradients
    //GradLength -- How many steps for each gradient
    //Rate -- The update rate (ms)

//Functions:
    //setPallet(*newPallet) -- Sets the pallet used for the gradient
    //setTotalEffectLength() -- Calculates the total length of all the gradients combined, you shouldn't need to call this
    //setPattern(*newPattern, newPatternLength) -- Sets the passed in pattern to be the effect pattern
    //                                             Will force setTotalEffectLength() call, so may cause effect to jump
    //setPalletAsPattern(*newPallet) -- Sets the passed in pallet as the effect pallet, and also the effect pattern (calls setTotalEffectLength())
    //setGradLength(newGradLength) -- Changes the Gradlength to the specified value, adjusting the length of the gradients (calls setTotalEffectLength())
    //update() -- updates the effect

//Notes:
    //For the randomly generated gradient constructor, the random pallet can be accessed via palletTemp

    //If using the pallet as the pattern, if you change the pallet, you'll ned to change the pattern as well
    //(unless the pallets are the same length)
class GradientCyclePS : public EffectBasePS {
    public:
        GradientCyclePS(SegmentSet &SegmentSet, uint8_t *Pattern, uint8_t PatternLength, palletPS *Pallet, uint8_t GradLength, uint16_t Rate); 

        GradientCyclePS(SegmentSet &SegmentSet, palletPS *Pallet, uint8_t GradLength, uint16_t Rate);

        GradientCyclePS(SegmentSet &SegmentSet, uint8_t NumColors, uint8_t GradLength, uint16_t Rate);

        ~GradientCyclePS();

        uint8_t
            gradLength, //total steps per fade
            patternLength, //length of the pattern array, use SIZE(pattern)
            *pattern;
        
        uint16_t 
            totalCycleLength, //total length of all the gradients combined, for reference
            cycleNum = 0; // tracks what how many patterns we've gone through

        palletPS
            palletTemp,
            *pallet;

        SegmentSet 
            &segmentSet; 
        
        void 
            setGradLength(uint8_t newGradLength),
            setPallet(palletPS* newPallet),
            setPattern(uint8_t *newPattern, uint8_t newPatternLength),
            setPalletAsPattern(palletPS *newPallet),
            setTotalEffectLength(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint8_t
            currentColorIndex,
            blendStep;
        
        uint16_t
            step,
            numPixels;
        
        bool
            tempPatternSet;

        CRGB 
            currentColor,
            nextColor,
            colorOut;
        
        void 
            init(uint16_t Rate);
        
};

#endif