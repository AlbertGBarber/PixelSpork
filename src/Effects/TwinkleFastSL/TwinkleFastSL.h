#ifndef TwinkleFastSL_h
#define TwinkleFastSL_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/* 
Fades sets of randomly chosen pixels in and out (like FastLED TwinkleFox). 
The effect is mostly the same as TwinkleSL.h, but uses much less ram, while having a few extra restrictions.

To make the effect "fast", it doesn't track where the twinkles are, 
instead, the background is filled, or the entire segment set is faded at the start of each update cycle. 
This wipes out of fades any existing twinkles, and then the a new twinkle is drawn. 
To make fading fast, we use FastLED's `fadeToBlackBy()` functions. 
Note, however, that this means that you cannot have a colored background and fading twinkles. 
The two are mutually exclusive. 

When fading, the twinkles are faded by a value out of 255 each cycle (`fadeOutRate`), 
eventually hitting 0 (black) due to integer math. 
 
Other than the above restriction, the effect is the same as Twinkle (Seg Line):
    * The color of the pixels can be set to a single color, chosen randomly, or picked from a palette. 

    * The effect supports Color Modes for both the main and background colors.

    * The effect is adapted to work on segment lines for 2D use. Each line will be a solid color.

    * The effect supports `randMode`'s for picking colors (see below).

randMode (default 0) (uint8_t):
    Sets how the twinkle colors will be picked.
    * 0 -- Picks colors randomly from the palette.
    * 1 -- Picks colors totally at random.

Example call: 
    TwinkleFastSL twinkleFast(mainSegments, CRGB::Red, 0, 5, true, 50, 70);
    Will spawn 5 pixels to fade from red each cycle.
    The background is black.
    "sparkle" is true, so the pixels will be faded to black (ignoring any background color)
    The fade rate is 50 (out of 255)
    The effect updates at 70ms.

    TwinkleFastSL twinkleFast(mainSegments, cybPnkPal_PS, CRGB::Green, 8, false, 50, 100);
    Will choose 8 pixels each cycle to set to colors from the cybPnkPal_PS palette.
    The background is green.
    "sparkle" is false, so the pixels will not be faded (the fade rate of 50 is ignored).
    The effect updates at 100ms.

Inputs:
    palette(optional, see constructors) -- The palette from which colors will be chosen randomly.
    color(optional, see constructors) -- A single color for the pixels. 
                                         The color will be stored in the effect's local palette, "paletteTemp".
    bgColor -- The color of the background. Is ignored if "sparkle" is true.
    numTwinkles -- The number of random pixels chosen each cycle.
    sparkle -- If true, the pixels will fade out over time, while forcing the background to be black.
    fadeOutRate -- The value (out of 255) that the pixels will fade by each cycle. Only used if `sparkle` is true.
                   Higher -> faster fading.
    rate -- The update rate

Other Settings:
    colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the background (see segDrawUtils::setPixelColor)
    fillBg (default true) -- If true, and `sparkle` is false, the background will be redrawn every cycle. 
                             Turning this off prevents "old" twinkles from being wiped out, producing a unique effect. 
                             Try it with a shifting Color Mode.
    randMode (default 0) -- (See randMode notes in intro)

Functions:
    setSingleColor(Color) -- Sets the effect to use a single color for the twinkles. The color will be stored in the 
                             effect's local palette, "paletteTemp".
    update() -- updates the effect
*/
class TwinkleFastSL : public EffectBasePS {
    public:
        //Constructor for a full palette effect
        TwinkleFastSL(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint16_t NumTwinkles, 
                      bool Sparkle, uint8_t FadeOutRate, uint16_t Rate);

        //Constructor for a using a single color
        TwinkleFastSL(SegmentSetPS &SegSet, CRGB Color, CRGB BgColor, uint16_t NumTwinkles, bool Sparkle,
                      uint8_t FadeOutRate, uint16_t Rate);

        //destructor
        ~TwinkleFastSL();

        uint8_t
            randMode = 0,
            colorMode = 0,
            bgColorMode = 0,
            fadeOutRate;

        uint16_t
            numTwinkles;

        bool
            fillBg = true,
            sparkle;

        CRGB
            bgColorOrig,
            *bgColor = nullptr;

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0};  //Must init structs w/ pointers set to null for safety

        void
            setSingleColor(CRGB Color),
            reset(),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        uint8_t
            paletteLength;

        uint16_t
            numLines,
            randLine;

        CRGB
            color;

        void
            init(CRGB BgColor, SegmentSetPS &SegSet, uint16_t Rate);
};

#endif