#ifndef Fire2012SL_h
#define Fire2012SL_h

//TODO: -- Add alternating direction mode?

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "Effects/Fire2012Seg/Utils/fire2012UtilsPS.h"

/* 
A classic, traditional fire loop, most useful for strips with a diffuser
Code modified from https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#fire
Similar to Fire2012Seg, but draws a a random fire along each Segment LINE rather than along each segment
Each Segment LINE has its own fire, but the settings are shared across the set to keep the fires consistent
Note that segment lines are perpendicular to the segments

Fires can be configured to start either at the first or last segment for the segment lines.

The fire colors are based on a palette and background color. 
The palette should be ordered from the coldest to the hottest color, ie, the first color in the palette will be used 
for the coldest parts of the fire while the last color will be used for the hottest. 
Palettes can be of any length, although 3 - 4 colors seems to work best for normal fires. 
There are several built-in palettes that work well: 
try `firePal_PS` for a traditional fire, 
`firePalPink_PS` for a pink/purple fire, and 
`firePalBlue_PS` for a green/blue fire.

The background color is separate from the palette colors and will be treated as the coldest color, filling most of the strip. 
Usually it is left as blank. 
Keeping the background color outside of the palette allows you to use multiple pre-built palettes 
while keeping the background constant. 

To produce a smoother fire, the palette colors can be blended together based on temperature
This does take more processing power, and can be turned off using the "blend" setting.

The `cooling` and `sparking` settings largely shape your fire, they will probably be the main things you adjust.
Cooling: indicates how fast a flame cools down. More cooling means shorter flames,
recommended values are between 20 and 100. 50 seems the nicest.

Sparking: indicates the chance (out of 255) that a spark will ignite. A higher value makes the fire more active.
Suggested values lay between 50 and 200. 90 & 120 work well.

Recommended update rate for the effect is 30-80ms

This effect is not compatible with color modes, but the bgColor is a pointer, so you can bind it

Note that the effect stores a uint8_t value for heat for each heat point
The number of heat points is numLines * numberOfSegments (see segment.h for more info)
so watch your memory usage

Example calls: 

    Fire2012SL fire2012SL(mainSegments, firePal_PS, 0, 50, 90, true, true, 70);
    Does a fire using the built-in firePal_PS palette (see paletteList.h) 
    with a blank background
    cooling is set to 50 and sparking is set to 90
    The fire colors will be blended
    Fire will start at the first segment and end at the last segment
    The fire updates at 70ms
 
Constructor Inputs:
    Palette-- The palette used for the fire, should be arranged from coldest to hottest colors
    bgColor -- The color used for the coldest parts of the fire ( usually blank (0) )
    cooling -- Indicates how fast a flame cools down. More cooling means shorter flames,
              Recommended values are between 20 and 100. 50 seems the nicest.
    sparking -- indicates the chance (out of 255) that a spark will ignite. A higher value makes the fire more active.
               Suggested values lay between 50 and 200. 90 & 120 work well.
    blend -- Determines if the fire colors will be blended together according to temperature
            Blended fires are smoother, but need more processing power
    direct -- The direction of the fire along segment lines,
              true starts the fire at the first segment, false, at the last.
    rate -- The update rate (ms) (recommended between 30-80ms)

Functions:
    reset() -- Resets the effect, use this if you change any segment lengths
    update() -- updates the effect 
*/
class Fire2012SL : public EffectBasePS {
    public:
        Fire2012SL(SegmentSetPS &SegSet, palettePS &Palette, CRGB BgColor, uint8_t Cooling, uint8_t Sparking,
                   bool Blend, bool Direct, uint16_t Rate);

        ~Fire2012SL();

        uint8_t
            *heat = nullptr,
            cooling,
            sparking;

        bool
            direct,
            blend;

        CRGB
            bgColorOrig,
            *bgColor = nullptr;  //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)

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
            maxNumPoints = 0,  //used for tracking the memory size of the heat array
            ledLoc;

        CRGB
            colorOut;
};

#endif