#ifndef EmptyEffectPS_h
#define EmptyEffectPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
//#include "MathUtils/mathUtilsPS.h"

/*
An empty effect, copy it to use as a base to build effects

<<Effect Comment Sections>>
Effect Description:
Inputs Guide / Notes:
Example calls: 
Constructor Inputs:
Functions:
Other Settings:
Reference Vars:
Flags:

*/
class EmptyEffectPS : public EffectBasePS {
    public:
        EmptyEffectPS(SegmentSet &SegmentSet, uint16_t Rate);  

        SegmentSet 
            &segmentSet; 
        
        //use for background color (if needed)
        //use BgColor in constructor
        //CRGB 
            //bgColorOrig,
            //*bgColor = nullptr; //bgColor is a pointer so it can be tied to an external variable if needed (such as a palette color)
        
        void 
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
};

#endif