#ifndef RainbowCyclePS_h
#define RainbowCyclePS_h

//TODO
    //Add spacing setting? -> use streamer effect + colorMode + segment rainbow shifting instead?
    
#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/*
A classic effect
Moves repeating rainbows of a set length along a segmentSet
shifts the rainbows in along the segmentSet in the passed in direction at the passed in rate
maximum length of the rainbow is 255 (b/c rgb are byte representations)
suggested min length is 5 (ie red, yellow, green, blue, indigo/violet)

This is the 1D version of the effect. See RainbowCycleSL.h for a segment line 2D version.
Comparing the two may help you learn how drawing on segment lines works.

Example call: 
    RainbowCyclePS(mainSegments, 30, true, 80); 
    Will draw rainbows of length 30, moving towards the end of the segmentSet, at 80ms

Constructor Inputs:
    length (max 255) -- The length of each rainbow
    direction -- The direction the rainbows will move in (true is forward)
    Rate -- update rate (ms)

Functions:
    setDirect(newDirect) -- sets a new direction for the effect
    setLength(newLength) -- sets a new rainbow length

Other Settings:
    satur (default 255) -- rainbow saturation value
    value (default 255) -- rainbow value value

Reference Vars (do not set these directly):
    length -- The length of the rainbow. Set this using setLength();
    direct --  The rainbow's direction. Set this using setDirect();

Notes: 
*/
class RainbowCyclePS : public EffectBasePS {
    public:
        RainbowCyclePS(SegmentSet &SegmentSet, uint8_t Length, bool Direction, uint16_t Rate); 
        //length fixed at 255 max, b/c that's the maximum length of a rainbow

        SegmentSet 
            &segmentSet;
    
        uint8_t
            satur = 255,
            value = 255,
            length; //reference only, call setLength() to change the rainbow length
        
        bool 
            direct; //reference only, call setDirect() to change the rainbow direction
        
        void 
            setDirect(bool newDirect),
            setLength(uint8_t newLength),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        int8_t
            stepDirect;
        
        uint16_t
            cycleCount,
            ledCount = 0,
            numSegs,
            numLeds,
            totSegLen,
            maxCycleLength,
            pixelNum;
        
        uint32_t
            stepVal;
        
        CRGB
            color;

        void 
            init();

};

#endif