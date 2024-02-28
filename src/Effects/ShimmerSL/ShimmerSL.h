#ifndef ShimmerSL_h
#define ShimmerSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/*

A simple effect. 
Colors all the pixels in a segment set, dimming each pixel by a random amount each cycle. 
The pixel colors can be chosen from a single color, a palette, or totally at random. 
The dimming range is controlled by `shimmerMin` and `shimmerMax` (0 to 255). 
If using a palette, the effect looks best when the palette colors are similar. 
Try it with a NoisePalette.

Note that when using a single color, it will be stored in the effect's local palette, `paletteTemp`. 

Supports Color Modes for the shimmering pixels.

The effect is adapted to work on segment lines for 2D use, with each line being a single color. 
You can also force the effect to be 1D using `lineMode`:

    `lineMode` (bool) (default true):
        * true -- The effect will "shimmer" segment lines (2D).
        * false -- The effect will "shimmer" individual pixels (1D). 
                This is useful if you want to use a 2D color mode with individual pixels. 

    `randMode` (uint8_t) (default 0):
        Determines how the shimmer colors will be chosen.
        * 0 -- Picks colors from the palette.
        * 1 -- Picks colors at random.

Example calls: 
    ShimmerSL shimmer(mainSegments, 0, 180, 80);
    The effect will do shimmers in random colors.
    The dim range is 0 to 180.
    The effect updates at a rate of 80ms
    (note this sets randMode = 1)

    ShimmerSL shimmer(mainSegments, CRGB::Red, 0, 230, 100);
    The effect will do shimmers in red
    The dim range is 0 to 230.
    The effect updates at a rate of 100ms.

    ShimmerSL shimmer(mainSegments, cybPnkPal_PS, 0, 180, 80);
    The effect will do shimmers using colors from the cybPnkPal_PS palette
    The dim range is 0 to 180.
    The effect updates at a rate of 80ms.

Constructor Inputs:
    palette(optional, see constructors) -- the palette from which colors will be chosen randomly
    color(optional, see constructors) -- the color that the randomly chosen pixels will be set to
    shimmerMin (min 0, max 255) -- The minimum amount of dim that will be applied to a pixel (0 is no dimming).
    shimmerMax (min 0, max 255) -- The maximum amount of dim that will be applied to a pixel (255 is max dimming, ie full black).
    rate -- The update rate (ms)

Other Settings:
    colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    lineMode (default true) -- Sets the effect to do shimmers along whole segment lines or individual pixels 
                               (see lineMode notes in intro)
    randMode (default 0) -- Sets how the shimmer colors will be chosen. (See randMode notes in intro)

Functions:
    setSingleColor(color) -- Sets the effect to use a single color for the pixels
    update() -- updates the effect
*/
class ShimmerSL : public EffectBasePS {
    public:

        //Constructor using random shimmer colors
        ShimmerSL(SegmentSetPS &SegSet, uint8_t ShimmerMin, uint8_t ShimmerMax, uint16_t Rate);

        //Constructor using a set shimmer color
        ShimmerSL(SegmentSetPS &SegSet, CRGB Color, uint8_t ShimmerMin, uint8_t ShimmerMax, uint16_t Rate);

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
            setSingleColor(CRGB color),
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