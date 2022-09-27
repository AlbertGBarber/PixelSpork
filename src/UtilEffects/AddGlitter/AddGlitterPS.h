#ifndef AddGlitterPS_h
#define AddGlitterPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

/*
A util effect to add random sparkles to an existing effect by lighting up random pixels in a glitter color
This util is intended to be run with another effect, and is not an effect by itself.

The effect has two rates: the glitterRate and the rate. 
The glitterRate sets how often the glitter particle locations change
while the rate sets how often the glitter particles are re-drawn (should be the same as your other effect rate)
Ie, if we wanted glitter to be set every 1000ms, but the effect we're applying glitter to 
updates every 100ms the glitter would need to be re-drawn every 100ms so that it wasn't over-written by the effect
but new glitter locations would only be set every 1000ms.
So the rate would be 100, and the glitterRate would be 1000.

There are two  glitter modes:
    0: All the glitter particles get new locations each update
    1: The glitter particles are replaced one by one when new locations at each update

Note that the effect never removes any glitter, it assumes that it will be run along-side another effect,
which will remove the glitter for it.
It also assumes that the glitter rate is slower or equal to the other effect rates
if glitter is the fastest, then it will keep filling the segment up with glitter.

The glitterColor, glitterRate, and rate are all pointers, so you can bind them externally.
(The passed in glitterColor, glitterRate, and Rate, are stored in glitterColorOrig, glitterRateOrig, and rateOrig)

Example call: 
    AddGlitterPS(mainSegments, CRGB::White, 10, 1, 1000, 100);
    Will drawn 10 white glitter particles on the mainSegment set every 100ms.
    Glitter mode 1 is used.
    New particle locations will be set every 1000ms.

Constructor Inputs:
    glitterColor -- The color of the glitter particles
    glitterNum -- How many glitter particles there are
    glitterMode -- What glitter mode will be used (see above)
    glitterRate -- How often new particle locations are set
    Rate -- The update rate (ms) (should be the same as the rate in the effect you want to add glitter to)

Functions:
    setGlitterNum(uint16_t newNum) -- Set the total number of glitter particles
    update() -- updates the effect

Reference Vars:
    glitterNum -- The number of glitter particles, use setGlitterNum() to set
*/

class AddGlitterPS : public EffectBasePS {
    public:
        AddGlitterPS(SegmentSet &SegmentSet, CRGB GlitterColor, uint16_t GlitterNum, uint8_t GlitterMode, uint16_t GlitterRate, uint16_t Rate);  

        ~AddGlitterPS();

        SegmentSet 
            &segmentSet; 
        
        uint16_t 
            glitterNum,
            *glitterRate,
            glitterRateOrig;
        
        uint8_t    
            glitterMode;

        CRGB 
            glitterColorOrig,
            *glitterColor;

        void 
            setGlitterNum(uint16_t newNum),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevGlitterTime = 0,
            prevTime = 0;
        
        uint16_t
            pixelNum,
            numLeds,
            *glitterLocs;
        
        void
            advanceGlitterArr(),
            fillGlitterArr();
};

#endif