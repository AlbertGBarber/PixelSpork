#ifndef FixedLengthRainbowCyclePS_h
#define FixedLengthRainbowCyclePS_h

#include "SegmentFiles.h"
#include "palletFiles.h"

#include "Effects/EffectBasePS.h"

//draws a repeated rainbow of a set length repeatedly along a segmentSet
//shifts the rainbows in along the segmentSet in the passed in direction at the passed in rate
//maximum length of the rainbow is 255 (b/c rgb are byte representations)
//suggested min length is 5 (ie red, yellow, green, blue, indigo/violet)
class FixedLengthRainbowCyclePS : public EffectBasePS {
    public:
        FixedLengthRainbowCyclePS(SegmentSet &SegmentSet, uint8_t Length, bool Direction, uint16_t Rate); 
        //length fixed at 255 max, b/c that's the maximum length of a rainbow
    
        uint8_t
            satur = 255,
            value = 255,
            length;

        bool
            direct(void);
        
        SegmentSet 
            &segmentSet;
        
        void 
            setDirect(bool newDirect),
            update(void);

    private:
        int8_t
            stepDirect;
        
        uint16_t
            cycleCount,
            ledCount = 0,
            numSegs,
            numLeds,
            maxCycleLength,
            ledLoc;
        
        unsigned long
            currentTime,
            prevTime = 0;

        bool 
            _direct;

        void 
            init();

};

#endif