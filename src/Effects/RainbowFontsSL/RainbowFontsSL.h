#ifndef RainbowFontsSLPS_h
#define RainbowFontsSLPS_h

/*
This code was adapted from code by Andrew Tuline in adapting PixelBlaze patterns:
https://github.com/atuline/PixelBlaze/blob/master/rainbow_fonts/rainbow_fonts.ino
For more information about creating patterns with Pixelblaze and the shared pattern library, see:
PixelBlaze as found at https://electromage.com/
PixelBlaze expressions: https://github.com/simap/pixelblaze/blob/master/README.expressions.md
Getting started guide: https://www.bhencke.com/pixelblazegettingstarted
Patterns: https://electromage.com/patterns

I've replaced the general PixelBlaze functions described here:
https://github.com/atuline/PixelBlaze/blob/master/pixelblaze.h
with FastLed versions. PixelBlaze uses a base value range of  0 to 1
while FastLed uses 0 to 255. So I've converted the code to use 0 to 255 in place of 0 to 1.
This allows me to avoid using floating point numbers, and lets me use the pre-build FastLed wave and math functions.

The PixelBlaze time() function can be replaced with beat8(freq) 
where the frequency is converted via: 60 / (*Time val in PixelBlaze code* / 0.015)
(0.015 is ~1 beat per second in PixelBlaze)
ex: for 0.1 in PixelBlaze we do 60 / (0.1/0.015) = ~9;
so we'd use beat8(9) in place of time(0.1);
*/

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/*
An effect based on the RainbowFonts pattern in PixelBlaze that rainbow waves that grow and shrink
The effect is largly the same as the orignal, but I've added the ability to change the frequency of the waves

The effect is adapted to work on segment lines for 2D use, but you can keep it 1D by
passing in a segmentSet with only one segment containing the whole strip.

Example calls: 

    RainbowFontsSL(ringSegments, 15, 80);
    Will do the effect with a wave freq of 15
    The effect updates at 80ms

Constructor inputs: 
    waveFreq (min value 1) -- How fast the waves happen and move, recommend value of 5 - 30
                              Higher -> faster
    Rate -- The update rate (ms) note that this is synced with all the particles.

Functions:
    update() -- updates the effect 
*/
class RainbowFontsSL : public EffectBasePS {
    public:
        RainbowFontsSL(SegmentSet &SegmentSet, uint8_t WaveFreq, uint16_t Rate);  

        SegmentSet 
            &segmentSet; 
        
        uint8_t
            waveFreq;

        void 
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        
        uint8_t
            t1,
            c;
        
        uint16_t
            hl,
            pixelNum,
            lineNum,
            numSegs,
            numLines;
    
        CRGB 
            colorOut;
};

#endif