#ifndef RainbowCycleSLPS_h
#define RainbowCycleSLPS_h
    
#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
#include "MathUtils/mathUtilsPS.h"

/*
Draws a repeated rainbow of a set length repeatedly along segment lines
shifts the rainbows in along the segmentSet liens in the passed in direction at the passed in rate
maximum length of the rainbow is 255 (b/c rgb are byte representations)
suggested min length is 5 (ie red, yellow, green, blue, indigo/violet)

This is the 2D version of rainbowCycle (although if you pass in a 1D segment the results will be the same)
For most effects I would combine the two effects for one that just works on segment lines
But since this is a very basic effect I wanted to keep it more user friendly, so I kept the 1D version of the effect

You also don't really need this effect at all, since the segment rainbow effects are built into
the library's colorModes. So you could just make a BGRateFill effect with color mode 4
Then change the segment's gradLineVal to set the rainbow length, set the offsetRate to set the speed
set runOffset to true, and change the offsetDirect if needed. 
ie: //mainSegments.runOffset = true;
      mainSegments.gradLineVal = 30;
      mainSegments.offsetRateOrig = 80;
      mainSegments.offsetDirect = true;
This would also let you change to different color modes on the fly
But this is maybe too complicated for learners.

Example call: 
    RainbowCycleSLPS(mainSegments, 30, true, 80); 
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
class RainbowCycleSLPS : public EffectBasePS {
    public:
        RainbowCycleSLPS(SegmentSet &SegmentSet, uint8_t Length, bool Direction, uint16_t Rate); 
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
            numLines,
            maxCycleLength;
        
        uint32_t
            stepVal;
        
        CRGB
            color;

        void 
            init();

};

#endif