#ifndef EdgeBurstSLPS_h
#define EdgeBurstSLPS_h

/*
This code was adapted from code by Andrew Tuline in adapting PixelBlaze patterns:
https://github.com/atuline/PixelBlaze/blob/master/edge_burst/edge_burst.ino
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
An effect based on the EdgeBurst pattern in PixelBlaze that produces a rainbow burst
half-way along the strip, that then expands and reflects back.
The effect is mostly the same as the orignal, but I've made it so that the start point 
of the waves is picked randomly from the number of lines for each wave. 
This adds some nice variation to the effect.
I've also added the ability to use a pallet for the wave colors. You can have them be choosen randomly for each wave.
I've also allowed you to adjust the pause time factor between waves.
I have this set to 2 (the original was 4), which makes the pause time very short.

The effect is adapted to work on segment lines for 2D use, but you can keep it 1D by
passing in a segmentSet with only one segment containing the whole strip.

Rainbow colors are controlled by rainbowMode, true will do rainbows.

Randomize Notes:

    The randomize pallet option is set by randomizePal. Note that the effect stores a *pallet
    and a palletTemp. If you pass in your own pallet, *pallet is bound to your pallet, otherwise
    I bind *pallet to palletTemp (which is set to a random set of colors)
    Randomizing a pallet changes the colors in a pallet directly, so you probably don't want it happening to 
    your passed in pallet. So I've coded the randomize to only work on the palletTemp.
    If you always want a random pallet, this is no problem, just call the random pallet constructor, but if
    you wanted to switch from your pallet to random pallets you need to bind *pallet to palletTemp and give 
    palletTemp a set length.
    Do this with: 
    <your effect instance name>->palletTemp.length = <a length value>
    <your effect instance name>->pallet = <your effect instance name>->&palletTemp;

Example calls: 

    EdgeBurstSLPS(ringSegments, 15, 80);
    Will do a a ranibow edge burst with a burst freq of 15
    The effect updates at 80ms

    EdgeBurstSLPS(ringSegments, &pallet1, 10, 80);
    Will do a an edge burst using colors from pallet1 with a burst freq of 10
    The effect updates at 80ms

    EdgeBurstSLPS(ringSegments, 3, true, 10, 80);
    Will do a an edge burst using a pallet of 3 random colors with a burst freq of 10
    The pallet colors will be randomly picked for each wave
    The effect updates at 80ms

Constructor inputs: 
    pallet (optional, see constructors) -- A custom pallet passed to the effect for colors
    numColors (optional, see constructors) -- How many colors will be in the randomly created pallet
    randomizePal (optional, see constructors) -- Only for the random color constructor. If true, random pallet colors
                                                 will be choosen for each wave
                                                 (see randomize note on this setting above)
    burstFreq (min value 1) -- How fast the bursts happen and move, recommend value of 5 - 30
                               Higher -> faster
    Rate -- The update rate (ms) note that this is synced with all the particles.

Functions:
    update() -- updates the effect 

Other Settings:
    burstPause (default 2, orginal code value was 4) -- Adjusts how long there is between bursts
                                                        Factors of 2 work best.
                                                        2 is almost no time, while 4 is roughly equal time between on and off
                                                        Note that higher values also increase wave speed, 
                                                        so you'll want to reduce your burstFreq
    rainbowMode (default false) -- If true, colors from the rainbow will be used. This is set automatically
                                   to true for the rainbow mode constructor.
*/
class EdgeBurstSLPS : public EffectBasePS {
    public:
        //Constructor for rainbow mode
        EdgeBurstSLPS(SegmentSet &SegmentSet, uint8_t BurstFreq, uint16_t Rate);  

        //Constructor for colors from pallet
        EdgeBurstSLPS(SegmentSet &SegmentSet, palletPS *Pallet, uint8_t BurstFreq, uint16_t Rate);

        //Constructor for a randomly created pallet
        //RandomizePal = true will randomize the pallet for each wave
        EdgeBurstSLPS(SegmentSet &SegmentSet, uint8_t numColors, bool RandomizePal, uint8_t BurstFreq, uint16_t Rate);

        ~EdgeBurstSLPS();

        SegmentSet 
            &segmentSet; 
        
        uint8_t
            burstPause = 2,
            burstFreq = 15;
        
        bool
            randomizePal = true,
            rainbowMode = false;
        
        palletPS
            palletTemp,
            *pallet;

        void 
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        int16_t
            edge;
        
        uint8_t
            numSegs,
            beatVal,
            t1,
            f,
            v,
            h;
        
        uint16_t
            offset = 0,
            blendLength,
            pixelNum,
            numLines;
        
        bool    
            offsetFlipFlop = true;
    
        CRGB 
            colorOut;
        
        void
            init(uint16_t Rate),
            pickRandStart();
};

#endif