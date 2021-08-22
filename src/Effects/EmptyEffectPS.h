#ifndef EmptyEffectPS_h
#define EmptyEffectPS_h

#include "SegmentFiles.h"
#include "palletFiles.h"

#include "Effects/EffectBasePS.h"

//an empty effect, copy it to use as a base to bulid effects
class EmptyEffectPS : public EffectBasePS {
    public:
        EmptyEffectPS(SegmentSet &SegmentSet, uint16_t Rate);  

        SegmentSet 
            &segmentSet; 
        
        void 
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
};

#endif