#ifndef Fire2012SL_h
#define Fire2012SL_h

//TODO: -- Add alternating direction mode?

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "Effects/Fire2012Seg/Utils/Fire2012SegUtilsPS.h"

/* 
A traditional fire loop, most useful for strips with a diffuser
Code modified from https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#fire
Similar to Fire2012Seg, but draws a a random fire along each segment LINE rather than along each segment
Each segment LINE has it's own fire, but the settings are shared accross the set to keep the fires consistent
Note that segment lines are perpendicular to the segments

Fires can be configured to start either at the first or last segment

The fire colors are based on a palette and a background color
The background color will be the coldest color, and is usually blank
It is input seperately from the palette because most palettes don't include a blank color
The palette should be ordered from the coldest to the hottest color
ie, the first color in the palette will be used for the coldest parts of the fire
while the last color will be used for the hottest

Palettes can be of any length, although 3 - 4 colors seems to work best for normal fires
try { CRGB::Red, CRGB{255, 143, 0}, CRGB{255, 255, 100} } for a tranditional fire
{ CRGB{225, 0, 127}, CRGB{123, 7, 197}, CRGB{238, 130, 238} } for a pink/purple fire
or { CRGB{16, 124, 126 }, CRGB{ 43, 208, 17 }, CRGB{120, 212, 96} } for a green/blue fire

To produce a smoother fire, the palette colors can be blended between based on temperature
This does take more processing power, and can be turned off using the "blend" flag

The other variables determine how the fire is drawn:
Cooling: indicates how fast a flame cools down. More cooling means shorter flames,
recommended values are between 20 and 100. 50 seems the nicest.

Sparking: indicates the chance (out of 255) that a spark will ignite. A higher value makes the fire more active.
Suggested values lay between 50 and 200. 90 & 120 work well.

recommended update rate for the effect is 30-80ms

This effect is not compatible with color modes, but the bgColor is a pointer, so you can bind it

Note that the effect stores a uint8_t value for heat for each heat point
The number of heat points is numLines * numberOfSegments (see segment.h for more info)
so watch your memory usage

Example calls: 
    CRGB firePalette_arr[] = { CRGB::Red, CRGB{255, 143, 0}, CRGB{255, 255, 100} };
    palettePS firePalette = {firePalette_arr, SIZE(firePalette_arr)};

    Fire2012SL(mainSegments, firePalette, 0, 50, 90, true, 70, true);
    Does a blended fire using the firePalette with a blank background
    cooling is set to 50 and sparking is set to 90
    The fire updates at 70ms
    Fire will start at the first segment and end at the last segment
 
Constructor Inputs:
    Palette-- The palette used for the fire, should be arranged from coldest to hottest colors
    bgColor -- The color used for the coldest parts of the fire ( usually blank (0) )
    cooling -- Indicates how fast a flame cools down. More cooling means shorter flames,
              Recommended values are between 20 and 100. 50 seems the nicest.
    sparking -- indicates the chance (out of 255) that a spark will ignite. A higher value makes the fire more active.
               Suggested values lay between 50 and 200. 90 & 120 work well.
    blend -- Determines if the fire colors will be blended together according to temperature
            Blended fires are smoother, but need more processing power
    direct -- The direction of the fire (true means it will start at the first segment, false, at the last)
    rate -- The update rate (ms) (recommended between 30-80ms)

Functions:
    reset() -- Resets the effect, use this if you change any segment lengths
    update() -- updates the effect 
*/
class Fire2012SL : public EffectBasePS {
    public:
        Fire2012SL(SegmentSet &SegmentSet, palettePS &Palette, CRGB BgColor, uint8_t Cooling, uint8_t Sparking, bool Blend, bool Direct, uint16_t Rate);  
        
        ~Fire2012SL();

        SegmentSet 
            &segmentSet; 
        
        uint8_t
            cooling,
            sparking;
        
        bool
            direct,
            blend;
        
        CRGB 
            bgColorOrig,
            *bgColor = nullptr; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)
        
        palettePS
            *palette = nullptr;

        void 
            reset(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;

        uint8_t
            *heat = nullptr,
            cooldown,
            paletteLength,
            paletteSecLen,
            sparkPoint;
        
        uint16_t
            numLines,
            numSegs,
            segNum,
            heatSecStart,
            heatIndex,
            ledLoc;
        
        CRGB 
            colorOut;
};  

#endif