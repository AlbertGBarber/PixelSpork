#ifndef TwinkleFastSLPS_h
#define TwinkleFastSLPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/* 
Fades sets of randomly chosen segment lines in and out (like FastLED TwinkleFox)
This is mostly the same as TwinkleSL, but uses much less ram, and has a few restrictions
Like, TwinkleSL, the color of the pixels can be set to a single color, chosen randomly, or picked from a palette 
However, the pixels can either be set to fade out, or switch to a background color, but not both
so if you choose to fade out the background will be set to 0
(this is because the whole segmentSet is faded each cycle)

The effect is adapted to work on segment lines for 2D use, but you can keep it 1D by
passing in a segmentSet with only one segment containing the whole strip.

This effect is fully compatible with color modes, and the bgColor is a pointer, so you can bind it
to an external color variable

Example call: 
    TwinkleFastSL(mainSegments, CRGB::Red, 5, 0, true, 50, 70);
    Will choose 5 segment lines to fade from red at 50 rate every cycle

Inputs:
    palette(optional, see constructors) -- the palette from which colors will be choosen randomly
    color(optional, see constructors) -- the color that the randomly choosen pixels will be set to
    numTwinkles -- The amount of random pixels choosen each cycle 
    bgColor -- The color of the background, this is what pixels will fade to and from
    sparkle -- Turns the fading on and off (on will produce a blank background)
    fadeOutRate -- The number of steps taken to fade pixels out, if turned on (min 1, max 255), higher = fasted fade
    rate -- The update rate

Functions:
    setSingleColor(Color) -- Sets the effect to use a single color for the pixels, will restart the effect
    update() -- updates the effect

Other Settings:
    colorMode (default 0) -- sets the color mode for the random pixels (see segDrawUtils::setPixelColor)
    bgColorMode (default 0) -- sets the color mode for the background (see segDrawUtils::setPixelColor)
    fillBG (default true) -- sets the background to be redrawn every cycle
    randMode (default 0) -- sets how colors will be picked
                           0: Picks colors from the palette
                           1: Picks colors at random 
*/
class TwinkleFastSL : public EffectBasePS {
    public:
        //Constructor for a full palette effect
        TwinkleFastSL(SegmentSet &SegmentSet, palettePS *Palette, uint16_t NumTwinkles, CRGB BgColor, bool Sparkle, uint8_t FadeOutRate, uint16_t Rate);  
        
        //Constructor for a using a single color
        TwinkleFastSL(SegmentSet &SegmentSet, CRGB Color, uint16_t NumTwinkles, CRGB BgColor, bool Sparkle, uint8_t FadeOutRate, uint16_t Rate);
        
        //Constructor for choosing all colors at random
        TwinkleFastSL(SegmentSet &SegmentSet, uint16_t NumTwinkles, CRGB BgColor, bool Sparkle, uint8_t FadeOutRate, uint16_t Rate);
        
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
            fillBG = true,
            sparkle;

        CRGB 
            bgColorOrig,
            *bgColor = nullptr;

        SegmentSet 
            &segmentSet; 
        
        palettePS
            paletteTemp,
            *palette = nullptr;
        
        void 
            setSingleColor(CRGB Color),
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
            init(CRGB BgColor, uint16_t Rate);
};

#endif