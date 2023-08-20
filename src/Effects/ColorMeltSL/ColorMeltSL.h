#ifndef ColorMeltSL_h
#define ColorMeltSL_h

/*
This code was adapted from code by Andrew Tuline in adapting PixelBlaze patterns:
https://github.com/atuline/PixelBlaze/blob/master/rainbow_melt/rainbow_melt.ino
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

/*
An effect based on the RainbowMelt pattern in PixelBlaze which produces rainbow bands tha shift in an out. 
Basically a whole mess of rainbows.
You can also supply your own colors with a palette.
The effect is mostly the same as the original, but I've added the ability to change the
melting frequency to speed up or slow down the melts, and also to slowly shift the melts across the 
strip.

The melt shifting can cause some weirdness so you can disable it completely with phaseEnable = false
or by setting the phaseFreq = 0 in the constructor.

As part of the effect, colors are dimmed in accordance with a wave, leaving large dark areas.
I have given you to option to invert the brightness, which swaps the brightness, making the dark areas bright and bright areas dim.
(In my opinion, the effect looks better with them inverted)

The effect is adapted to work on segment lines for 2D use, but you can keep it 1D by
passing in a SegmentSet with only one segment containing the whole strip.

Example calls: 

    ColorMeltSL colorMelt(mainSegments, 15, 3, true, 80);
    Will do a melt with a meltFreq of 15 and a phaseFreq of 3
    Rainbow colors will be used
    The brightness waves will be inverted for more color
    The effect updates at 80ms

    ColorMeltSL colorMelt(mainSegments, cybPnkPal, 9, 0, true, 80);
    Will do a melt with a meltFreq of 9 and a phaseFreq of 0
    Colors from cybPnkPal will be used
    Because the phaseFreq is 0, phaseEnable will be set false as part of the constructor
    The brightness waves will be inverted for more color
    The effect updates at 80ms

    ColorMeltSL colorMelt(mainSegments, 3, 9, 1, false 80);
    Will do a melt with a meltFreq of 9 and a phaseFreq of 1
    A palette of 3 random colors will be used
    The brightness waves will NOT be inverted
    The effect updates at 80ms

Constructor inputs: 
    palette (optional, see constructors) -- A custom palette passed to the effect for colors
    numColors (optional, see constructors) -- How many colors will be in the randomly created palette
    meltFreq -- How fast the melts happen, recommend value of 8 - 20, (min 1) (lower is faster)
    phaseFreq -- How quickly the melts are shifted around, can cause weirdness
                 Recommend values between 1 - 5
                 Passing in 0 to constructor will set phaseEnable to false
    briInvert -- If true, then the brightness wave will be inverted, producing larger areas of color (see intro above)
    rate -- The update rate (ms) note that this is synced with all the particles.

Functions:
    update() -- updates the effect 

Other Settings:
    phaseEnable (default true) -- Allows the phase to shift over time at the phaseFreq
    phase -- The phase offset. This is changed over time when phaseEnable is true, but you can 
             set it to a specific value if you like.
    rainbowMode (default false) -- If true, colors from the rainbow will be used. This is set automatically
                                   to true for the rainbow mode constructor.
    hlDiv (default 2) -- Sets the melt divisions, the default of 2 is taken from the original effect
                         It's hard to describe what this does, but increasing it sort of shortens the melt waves.
*/
class ColorMeltSL : public EffectBasePS {
    public:
        //Constructor for rainbow mode
        ColorMeltSL(SegmentSet &SegSet, uint8_t MeltFreq, uint8_t PhaseFreq, bool BriInvert, uint16_t Rate);  

        //Constructor for colors from palette
        ColorMeltSL(SegmentSet &SegSet, palettePS &Palette, uint8_t MeltFreq, uint8_t PhaseFreq, bool BriInvert, uint16_t Rate);

        //Constructor for a randomly created palette
        ColorMeltSL(SegmentSet &SegSet, uint8_t numColors, uint8_t MeltFreq, uint8_t PhaseFreq, bool BriInvert, uint16_t Rate);

        ~ColorMeltSL();
        
        uint8_t
            meltFreq,
            phaseFreq,
            phase = 0,
            hlDiv = 2;
        
        bool
            briInvert = true,
            rainbowMode = false,
            phaseEnable = true;

        palettePS
            *palette = nullptr,
            paletteTemp = {nullptr, 0}; //Must init structs w/ pointers set to null for safety
        
        void 
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint8_t
            t1,
            t2,
            v,
            c1, 
            c2,
            c3;
        
        uint16_t
            hl,
            pixelNum,
            numLines,
            numSegs,
            blendLength;
    
        CRGB 
            colorOut;
        
        void 
            init(SegmentSet &SegSet, uint16_t Rate);
};

#endif