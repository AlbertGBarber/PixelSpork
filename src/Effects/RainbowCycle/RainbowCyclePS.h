#ifndef RainbowCyclePS_h
#define RainbowCyclePS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/*
A classic effect.
Moves repeating rainbows of a set length along a Segment Set in a set direction.
Suggested min length is 5 (ie red, yellow, green, blue, indigo/violet).

This is the 1D version of the effect. See RainbowCycleSL.h for a segment line 2D version.
Comparing the two may help you learn how drawing on segment lines works.

Example call: 
    RainbowCyclePS rainbowCycle(mainSegments, 30, true, 80); 
    Will draw rainbows of length 30, moving towards the end of the SegmentSetPS, at 80ms

    RainbowCyclePS rainbowCycle(mainSegments, true, 80); 
    Will draw rainbows of length 255 (the length is set to 255 by default b/c it is omitted from the constructor),
    moving towards the end of the SegmentSetPS, at 80ms

Constructor Inputs:
    length (optional) -- The length of each rainbow, if omitted, the rainbow will be set to the default of 255
    direct -- The direction the rainbows will move in (true is forward)
    rate -- update rate (ms)

Functions:
    setLength(newLength) -- sets a new rainbow length
    update() -- updates the effect 

Other Settings:
    satur (default 255) -- rainbow saturation value
    value (default 255) -- rainbow value value

Reference Vars:
    length -- The length of the rainbow. Set using setLength().

*/
class RainbowCyclePS : public EffectBasePS {
    public:
        //Constructor for a custom length rainbow cycle
        RainbowCyclePS(SegmentSetPS &SegSet, uint16_t Length, bool Direct, uint16_t Rate);

        //Constructor for a full length rainbow (255)
        RainbowCyclePS(SegmentSetPS &SegSet, bool Direct, uint16_t Rate);

        uint8_t
            satur = 255,
            value = 255;

        uint16_t
            length;

        bool
            direct;

        void
            setLength(uint16_t newLength),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        int8_t
            stepDirect;

        uint16_t
            cycleNum,
            ledCount = 0,
            numSegs,
            numLeds,
            totSegLen,
            maxCycleLength,
            pixelNum,
            stepVal;

        CRGB
            color;

        void
            init(SegmentSetPS &SegSet, uint16_t Rate);
};

#endif