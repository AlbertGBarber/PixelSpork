#ifndef ShimmerSL_h
#define ShimmerSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/*
A simple effect
Colors all the pixels in a SegmentSetPS either using a single color, choosing colors from a palette, or at random
each pixel is dimmed a random amount each cycle, which produces the effect
The dim range is controlled by ShimmerSLMin and ShimmerSLMax.

Each cycle each pixel's brightness and color are picked at random from the palette.
If you do use your own palette it's probably best to pick colors that are similar to each other.

If using a single color, it will be stored in the effect's paletteTemp palette
You can change the color by using <your effect name>->paletteTemp.paletteArr[0] = <your color>
or by calling paletteUtils::setColor(<your effect name>->paletteTemp, 0, <your color>)

The effect is adapted to work on segment lines for 2D use, but you can keep it 1D by
passing in a SegmentSetPS with only one segment containing the whole strip.
Or you can set lineMode to false, which means each pixel always gets it's own shimmer value.

Example calls: 
    ShimmerSL shimmer(mainSegments, 0, 180, 80);
    Each pixel will be set to a random color, refreshing at 80ms
    The dim range will be 0 to 180.
    (note this sets randMode = 1)

    ShimmerSL shimmer(mainSegments, CRGB::Red, 0, 230, 100);
    The dim range will be 0 to 230.
    Each pixel will be set to red, and dimmed randomly, refreshing at 100ms

    ShimmerSL shimmer(mainSegments, cybPnkPal_PS, 0, 180, 80);
    The dim range will be 0 to 180.
    Each pixel will be set to a color from the palette, dimmed randomly, refreshing at 80ms

Constructor Inputs:
    palette(optional, see constructors) -- the palette from which colors will be chosen randomly
    color(optional, see constructors) -- the color that the randomly chosen pixels will be set to
    shimmerSLMin (min 0, max 255) -- The minimum amount of dim that will be applied to a pixel 
    shimmerSLMax (min 0, max 255) -- The maximum amount of dim that will be applied to a pixel
    rate -- The update rate (ms)

Other Settings:
    colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    lineMode (default true) -- If false, each pixel will have it's own shimmer amount (rather than it being the same along segment lines)
                               Only really useful if you want multi-segment color modes and don't want the shimmers in lines
    randMode (default 0) -- sets how colors will be picked
                            0: Picks colors from the palette
                            1: Picks colors at random

Functions:
    setSingleColor(Color) -- Sets the effect to use a single color for the pixels
    update() -- updates the effect
*/
class ShimmerSL : public EffectBasePS {
    public:

        //Constructor using a random shimmer color
        ShimmerSL(SegmentSetPS &SegSet, uint8_t ShimmerMin, uint8_t ShimmerMax, uint16_t Rate);

        //Constructor using a set shimmer color
        ShimmerSL(SegmentSetPS &SegSet, CRGB ShimmerColor, uint8_t ShimmerMin, uint8_t ShimmerMax, uint16_t Rate);

        //Constructor for colors randomly chosen from palette
        ShimmerSL(SegmentSetPS &SegSet, palettePS &Palette, uint8_t ShimmerMin, uint8_t ShimmerMax, uint16_t Rate);

        ~ShimmerSL();

        uint8_t
            randMode = 0,
            colorMode = 0,
            shimmerMin,
            shimmerMax;

        bool
            lineMode = true;

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        void
            setSingleColor(CRGB Color),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        uint8_t
            paletteLength,
            shimmerVal;

        uint16_t
            numSegs,
            numLines,
            pixelNum;

        CRGB
            pickColor(),
            colorOut,
            color;

        void
            init(SegmentSetPS &SegSet, uint16_t Rate);
};

#endif