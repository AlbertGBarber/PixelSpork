#ifndef TheaterChasePS_h
#define TheaterChasePS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

//Does a classic theater chase (one color running on top of another with spaces in between)
//You can customize the spacing between the running color spots, and the lengths of each of the sections
//ie you can have say red spots of length 2, running on a blue background, with 3 pixels in between each red spot
//so you'd have rrbbbrrbbb, etc, were r = red, b = blue

//note that both the spot color and the backgound color are fully compatible with colorMode settings (see segDrawUtils::setPixelColor)

//Both color and bgColor are pointers, so you can tie them to external variables
//ie color = &YourColorVar
//colorOrig is the defautl binding

//Example call: 
    //TheaterChasePS(mainSegments, CRGB::Red, CRGB::Green, 1, 2, 100)
    //Red spots of length 1, running on a green background, with 2 pixels inbetween each spot, updating at 100ms
    //ie you'd get rggrgg, etc where r = red, g = green pixels

//Constructor Inputs:
    //Color-- The color of the running spots
    //BgColor -- The color of the background, the color inbetween the spots
    //LitLength (max 255, min 1) -- The size of the spots
    //Spacing (max 255, min 1) -- The size of the space between the spots
    //Rate -- The update rate (ms)

//Functions:
    //setLitLength(uint8_t newLitlength) -- Changes the spot size
    //setSpacing(uint8_t newSpacing) -- Changes the size between spots
    //setDrawLength() -- Recaculates the sum of the spot and spacing (you shouldn't need to call this)
    //update() -- updates the effect

//Other Settings:
    //colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    //bgColorMode (default 0) -- sets the color mode for the background (see segDrawUtils::setPixelColor)
    //litLength -- The spot size, DO NOT set this directly, use setLitLength()
    //spacing -- The size of the space between spots, DO NOT set this directly, use setSpacing()
class TheaterChasePS : public EffectBasePS {
    public:
        TheaterChasePS(SegmentSet &SegmentSet, CRGB Color, CRGB BgColor, uint8_t LitLength, uint8_t Spacing, uint16_t Rate);  

        SegmentSet 
            &segmentSet; 
        
        uint8_t
            litLength = 1,
            colorMode = 0,
            bgColorMode = 0,
            spacing = 1;
        
        CRGB 
            colorOrig,
            *color, //color is a pointer so it can be tied to an external variable if needed (such as a pallet color)
            bgColorOrig,
            *bgColor; //bgColor is a pointer so it can be tied to an external variable if needed (such as a pallet color)
    
        void
            setLitLength(uint8_t newLitlength),
            setSpacing(uint8_t newSpacing),
            setDrawLength(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;

        uint16_t
            cycleCount= 0,
            totalDrawLength,
            numPixels,
            pixelLoc;
};

#endif