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
suggested min length is 5 (ie red, yellow, green, blue, indigo/violet)

This is the 1D version of the effect. See RainbowCycleSL.h for a segment line 2D version.
Comparing the two may help you learn how drawing on segment lines works.

Example call: 
    RainbowCyclePS(mainSegments, 30, true, 80); 
    Will draw rainbows of length 30, moving towards the end of the segmentSet, at 80ms

    RainbowCyclePS(mainSegments, true, 80); 
    Will draw rainbows of length 255 (the length is set to 255 by default b/c it is omitted from the constructor),
    moving towards the end of the segmentSet, at 80ms

Constructor Inputs:
    length (optional) -- The length of each rainbow, if omitted, the rainbow will be set to the default of 255
    direction -- The direction the rainbows will move in (true is forward)
    Rate -- update rate (ms)

Functions:
    setLength(newLength) -- sets a new rainbow length

Other Settings:
    satur (default 255) -- rainbow saturation value
    value (default 255) -- rainbow value value

Reference Vars (do not set these directly):
    length -- The length of the rainbow. Set this using setLength();

Notes: 
*/
class RainbowCyclePS : public EffectBasePS {
    public:
        RainbowCyclePS(SegmentSet &SegmentSet, uint16_t Length, bool Direction, uint16_t Rate); 
        
        //Does a rainbow cycle of length 255
        RainbowCyclePS(SegmentSet &SegmentSet, bool Direction, uint16_t Rate); 

        SegmentSet 
            &segmentSet;
    
        uint8_t
            satur = 255,
            value = 255;
        
        uint16_t
            length;
        
        bool 
            direct; //reference only, call setDirect() to change the rainbow direction
        
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
            cycleCount,
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
            init(uint16_t Rate);

};

#endif