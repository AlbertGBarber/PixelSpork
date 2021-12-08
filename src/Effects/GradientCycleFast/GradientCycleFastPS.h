#ifndef GradientCycleFastPS_h
#define GradientCycleFastPS_h

//TODO -- add constructor for random options?

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
//   This will move along the strip before disappering

//Basically the effect works by setting the color of the first pixel, then for each subsequent pixel,
//it copies the color of the next pixel in line
//So any changes you make to colors will only show up at the first pixel, and will be shifted along the strip

//However, as a bonus, this effect supports random colored gradients
//where the colors for the gradients are choosen at random as the enter the strip
//This is controlled by the randColors flag and the randMode setting

//Example calls: 
    //uint8_t pattern_arr = {0, 1, 4};
    //patternPS pattern = {pattern_arr, SIZE(pattern_arr)};
    //GradientCycleFastPS(mainSegments, &pattern, &pallet, 10, 100);
    //Will do a gradient cycle from color 0, to color 1, to color 4, of the pallet
    //with 10 steps to each gradient, and a 100ms update rate

    //GradientCycleFastPS(mainSegments, &pallet, 10, 100);
    //Will do a gradient cycle using the colors in the pallet, with 10 steps to each gradient,and a 100ms update rate

    //GradientCycleFastPS(mainSegments, 3, 15, 80);
    //Will do a gradient cycle using 3 randomly choosen colors, with 15 steps to each gradient,and an 80ms update rate
    //note this is not the same as setting randMode, it just makes a random pallet
 
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

//Other Settings:
    //randColors (default false) -- If true, the colors for each gradient will be choosen randomly according to the randMode
    //randMode (default 0) -- Sets the type of random colors choosen:
    //                     -- 0: Colors will be choosen completely at random
    //                     -- 1: Colors will be choosen randomly from the pattern (will not repeat the same color in a row)
    //                     --                                                     (unless your pattern has the same color in a row, like { 2, 2, 3})

//Flags:
    //initFillDone -- Flag for doing the initial fill of the gradients on the strip
    //                Set true once the fill is done

//Notes:

class GradientCycleFastPS : public EffectBasePS {
    public:
        GradientCycleFastPS(SegmentSet &SegmentSet, patternPS *Pattern, palletPS *Pallet, uint8_t GradLength, uint16_t Rate); 

        GradientCycleFastPS(SegmentSet &SegmentSet, palletPS *Pallet, uint8_t GradLength, uint16_t Rate);

        GradientCycleFastPS(SegmentSet &SegmentSet, uint8_t NumColors, uint8_t GradLength, uint16_t Rate);

        ~GradientCycleFastPS();

        uint8_t
            randMode = 0,
            gradLength;
        
        uint16_t 
            cycleNum = 0; // tracks what how many patterns we've gone through

        bool
            randColors = false,
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
            nextPattern,
            shuffleIndex();
        
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
            pickNextColor(),
            initalFill();    
};

#endif