#ifndef TheaterChaseSL_h
#define TheaterChaseSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/*
Does a classic theater chase (one color running on top of another with spaces in between)
You can customize the spacing between the running color spots, and the lengths of each of the sections
ie you can have say red spots of length 2, running on a blue background, with 3 pixels in between each red spot
so you'd have rrbbbrrbbb, etc, were r = red, b = blue

note that both the spot color and the background color are fully compatible with colorMode settings (see segDrawUtils::setPixelColor)

Both color and bgColor are pointers, so you can tie them to external variables
ie color = &YourColorVar
colorOrig is the default binding

The effect is adapted to work on segment lines for 2D use, but you can keep it 1D by
passing in a SegmentSet with only one segment containing the whole strip.

Example call: 
    TheaterChaseSL theaterChase(mainSegments, CRGB::Red, CRGB::Green, 1, 2, 100)
    Red spots of length 1, running on a green background, with 2 pixels in between each spot, updating at 100ms
    ie you'd get rggrgg, etc where r = red, g = green pixels

Constructor Inputs:
    color-- The color of the running spots
    bgColor -- The color of the background, the color in between the spots
    litLength (max 255, min 1) -- The size of the spots
    spacing (max 255, min 1) -- The size of the space between the spots
    rate -- The update rate (ms)

Functions:
    setLitLength(newLitLength) -- Changes the spot size, you can set litLength directly, but make sure its >1
    setSpacing(newSpacing) -- Changes the size between spots, you can set spacing directly, but make sure its >1
    update() -- updates the effect

Other Settings:
    colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the background (see segDrawUtils::setPixelColor)

Reference Vars:
    cycleNum --  This tracks how many update cycles have happened, resets every totalDrawLength (spacing + litLength)
    spacing -- The distance between lit pixels, can be set manually as long as it's >1, otherwise set with setLitLength()
    litLength -- The length of lit pixel sections, can be set manually as long as it's >1, otherwise set with setSpacing()
*/
class TheaterChaseSL : public EffectBasePS {
    public:
        TheaterChaseSL(SegmentSet &SegSet, CRGB Color, CRGB BgColor, uint8_t LitLength, uint8_t Spacing, uint16_t Rate);  

        SegmentSet 
            &SegSet; 
        
        uint8_t
            litLength = 1, //min of 1
            spacing = 1, //min of 1
            colorMode = 0,
            bgColorMode = 0;
        
        uint16_t
            cycleNum = 0; //How many update cycles have happened, for reference only
        
        CRGB 
            colorOrig,
            *color = nullptr, //color is a pointer so it can be tied to an external variable if needed (such as a palette color)
            bgColorOrig,
            *bgColor = nullptr; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)
    
        void
            setLitLength(uint8_t newLitLength),
            setSpacing(uint8_t newSpacing),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;

        uint16_t
            totalDrawLength,
            numLines,
            lineNum;
};

#endif