#ifndef ShimmerSLPS_h
#define ShimmerSLPS_h

//TODO
//-- add version with backgound color?, would need to use blend instead of scale8

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/*
A simple effect
Colors all the pixels in a segmentSet either using a single color, choosing colors from a pallet, or at random
each pixel is dimmed a random amount each cycle, which produces the effect
The dim range is controlled by ShimmerSLMin and ShimmerSLMax

If using a single color, it will be stored in the effect's palletTemp pallet
You can change the color by using <your effect name>->palletTemp.palletArr[0] = <your color>
or by calling palletUtils::setColor(<your effect name>->palletTemp, 0, <your color>)

The effect is adapted to work on segment lines for 2D use, but you can keep it 1D by
passing in a segmentSet with only one segment containing the whole strip.

Example calls: 
    ShimmerSLPS(SegmentSet &SegmentSet, 0, 180, 80);
    Each pixel will be set to a random color, refreshing at 80ms
    The dim range will be 0 to 180.
    (note this sets randMode = 1)

    ShimmerSLPS(mainSegments, CRGB::Red, 0, 230, 100);
    The dim range will be 0 to 230.
    Each pixel will be set to red, and dimmed randomly, refreshing at 100ms

    ShimmerSLPS(mainSegments, pallet, 0, 180, 80);
    The dim range will be 0 to 180.
    Each pixel will be set to a color from the pallet, dimmed randomly, refreshing at 80ms

Constructor Inputs:
    Pallet(optional, see constructors) -- the pallet from which colors will be choosen randomly
    Color(optional, see constructors) -- the color that the randomly choosen pixels will be set to
    ShimmerSLMin (min 0, max 255) -- The minimum amount of dim that will be applied to a pixel 
    ShimmerSLMax (min 0, max 255) -- The maximum amount of dim that will be applied to a pixel
    Rate -- The update rate (ms)

Functions:
    setSingleColor(Color) -- Sets the effect to use a single color for the pixels
    setPallet(*newPallet) -- Sets the pallet used
    update() -- updates the effect

Other Settings:
    colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    randMode (default 0) -- sets how colors will be picked
                           0: Picks colors from the pallet
                           1: Picks colors at random
*/
class ShimmerSLPS : public EffectBasePS {
    public:

        //Constructor using a random shimmer color
        ShimmerSLPS(SegmentSet &SegmentSet, uint8_t ShimmerMin, uint8_t ShimmerMax, uint16_t Rate);

        //Constructor using a set shimmer color
        ShimmerSLPS(SegmentSet &SegmentSet, CRGB ShimmerColor, uint8_t ShimmerMin, uint8_t ShimmerMax, uint16_t Rate);  

        //Constuctor for colors randomly choosen from pallet
        ShimmerSLPS(SegmentSet &SegmentSet, palletPS *Pallet, uint8_t ShimmerMin, uint8_t ShimmerMax, uint16_t Rate);

        ~ShimmerSLPS();

        SegmentSet 
            &segmentSet; 
        
        uint8_t
            randMode = 0,
            colorMode = 0,
            shimmerMin,
            shimmerMax;
        
        palletPS
            palletTemp,
            *pallet;

        void
            setSingleColor(CRGB Color),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint8_t
            numSegs,
            palletLength,
            shimmerVal;
        
        uint16_t
            numLines,
            pixelNum;
        
        CRGB
            pickColor(),
            colorOut,
            color;
            
        void 
            init(uint16_t Rate);
};

#endif