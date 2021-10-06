#ifndef ShimmerPS_h
#define ShimmerPS_h

//TODO
//-- add version with backgound color?, would need to use blend instead of scale8

#include "Effects/EffectBasePS.h"
#include "Effects/EffectUtils/EffectUtilsPS.h"

//A simple effect
//Colors all the pixels in a segmentSet either using a single color, choosing colors from a pallet, or at random
//each pixel is dimmed a random amount each cycle, which produces the effect
//The dim range is controlled by shimmerMin and shimmerMax

//Example calls: 
    //ShimmerPS(SegmentSet &SegmentSet, 80);
    //Each pixel will be set to a random color, refreshing at 80ms

    //FairyLightsPS(mainSegments, CRGB::Red, 100);
    //Each pixel will be set to red, and dimmed randomly, refreshing at 100ms

    //FairyLightsPS(mainSegments, pallet, 80);
    //Each pixel will be set to a color from the pallet, dimmed randomly, refreshing at 80ms

//Constructor Inputs:
    //Pallet(optional, see constructors) -- the pallet from which colors will be choosen randomly
    //Color(optional, see constructors) -- the color that the randomly choosen pixels will be set to
    //Rate -- The update rate (ms)

//Functions:
    //setSingleColor(Color) -- Sets the effect to use a single color for the pixels
    //setPallet(*newPallet) -- Sets the pallet used
    //update() -- updates the effect

//Other Settings:
    //colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    //shimmerMin (min 0, max 255, default 0) -- The minimum amount of dim that will be applied to a pixel 
    //shimmerMax (min 0, max 255, default 180) -- The maximum amount of dim that will be applied to a pixel 
class ShimmerPS : public EffectBasePS {
    public:

        ShimmerPS(SegmentSet &SegmentSet, uint16_t Rate);

        ShimmerPS(SegmentSet &SegmentSet, CRGB ShimmerColor, uint16_t Rate);  

        ShimmerPS(SegmentSet &SegmentSet, palletPS *Pallet, uint16_t Rate);

        ~ShimmerPS();

        SegmentSet 
            &segmentSet; 
        
        uint8_t
            colorMode = 0,
            shimmerMin = 0,
            shimmerMax = 180;
        
        palletPS
            *pallet;

        void
            setSingleColor(CRGB Color),
            setPallet(palletPS *newPallet),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint8_t
            shimmerVal;
        
        uint16_t
            numActiveLeds;
        
        CRGB
            color;
        
        pixelInfoPS
            pixelInfo{0, 0, 0, 0};

        palletPS
            palletTemp;
            
        void 
            pickColor(uint16_t pixelNum),
            init(uint16_t Rate);
};

#endif