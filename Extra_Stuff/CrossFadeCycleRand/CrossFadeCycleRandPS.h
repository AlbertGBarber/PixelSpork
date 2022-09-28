#ifndef CrossFadeCycleRandPS_h
#define CrossFadeCycleRandPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"

//like CrossFadeCycle, but all the colors are choosen at random
//(this class is redundant since CrossFadeCycle can do randoms, but is here anyway)
class CrossFadeCycleRandPS : public EffectBasePS {
    public:
        CrossFadeCycleRandPS(SegmentSet &SegmentSet, uint16_t NumCycles, uint8_t Steps, uint16_t Rate);
       
        uint8_t
            steps;
        
        uint16_t 
            numCycles;
        
        bool
            infinite = false,
            done = false;

        SegmentSet 
            &segmentSet; 

        void 
            reset(void),
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        CRGB 
            startColor,
            newColor, 
            nextColor;
        
        uint8_t
            currentStep = 0;
        
        uint16_t 
            cycleCount = 0;
};

#endif