#ifndef EmptyEffectPS_h
#define EmptyEffectPS_h

//#include "SegmentFiles.h"
//#include "palletFiles.h"

#include "Effects/EffectBasePS.h"
#include "Effects/EffectUtils/EffectUtilsPS.h"
///#include "Utils/MathUtilsPS.h"

//an empty effect, copy it to use as a base to bulid effects
class EmptyEffectPS : public EffectBasePS {
    public:
        EmptyEffectPS(SegmentSet &SegmentSet, uint16_t Rate);  

        SegmentSet 
            &segmentSet; 
        
        //use for background color (if needed)
        //use BgColor in constructor
        //CRGB 
            //bgColorOrig,
           //*bgColor; //bgColor is a pointer so it can be tied to an external variable if needed (such as a pallet color)
        
        void 
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
};

#endif