#ifndef TheaterChaseSL_h
#define TheaterChaseSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/*
A classic theater chase effect; one color running on top of another with spaces in between. 
You can customize the spacing between and the size of the running colors. For example, 
lets say you have red spots of length 2, running on a blue background, with 3 spaces in between each red spot.
The result would be ( r, r, b, b, b, r, r, etc) were r = red, b = blue.

Note: the minimum value for both the color spot size and spacing is 1.

The effect is adapted to work on segment lines for 2D use. Each segment line will be a single color.

Supports Color Modes for both the main and background colors.

For more advanced versions of this effect, see StreamerSL.h and SegWaves.h.

Example call: 
    TheaterChaseSL theaterChase(mainSegments, CRGB::Red, CRGB::Green, 1, 2, 100);
    Red spots of length 1, running on a green background, 
    with 2 spaces in between each spot, 
    updating at 100ms

Constructor Inputs:
    color-- The color of the running spots. Is a pointer. By default it's bound to colorOrig.
    bgColor -- The color of the background, the color in between the spots. 
               Is a pointer. By default it's bound to bgColorOrig.
    litLength (min 1) -- The size of the spots. 
    spacing (min 1) -- The size of the space between the spots. 
    rate -- The update rate (ms)

Other Settings:
    colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the background (see segDrawUtils::setPixelColor)

Functions:
    update() -- updates the effect

Reference Vars:
    cycleNum -- Tracks the number of update cycles, resets every (spacing + litLength) cycles.
*/
class TheaterChaseSL : public EffectBasePS {
    public:
        TheaterChaseSL(SegmentSetPS &SegSet, CRGB Color, CRGB BgColor, uint16_t LitLength, uint16_t Spacing,
                       uint16_t Rate);

        uint8_t
            colorMode = 0,
            bgColorMode = 0;

        uint16_t
            litLength = 1,  //min of 1
            spacing = 1,    //min of 1
            cycleNum = 0;  //How many update cycles have happened, for reference only

        CRGB
            colorOrig,
            *color = nullptr,  //color is a pointer so it can be tied to an external variable if needed (such as a palette color)
            bgColorOrig,
            *bgColor = nullptr;  //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

        void
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