#ifndef EmptyEffectPS_h
#define EmptyEffectPS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
//#include "MathUtils/mathUtilsPS.h"

/*
Utility Description:

This utility just calls segDrawUtils::show() at a set rate. 
May be useful if you're trying to manage frame rates 
or balance updating mutiple simultaneous effects with different update rates.

Note that the utility does require a segment set, but when it is updated it will update all LEDs
(it calls segDrawUtils::show(), which calls FastLED.show()).

Otherwise this utility behaves like a effect, it has an *rate update rate pointer, a showNow var, etc.

Example calls: 

    JustShowPS justShow(mainSegments, 30);
    Will call FastLED.show() every 30ms

Constructor Inputs:
    segSet -- A segment set, required for the utility, but the exact segment set doesn't matter, since FastLED.show() updates all LEDs
    Rate -- update rate (ms). Like with other effects, this a pointer.

Functions:
    update() -- updates the utility
   
*/
class JustShowPS : public EffectBasePS {
    public:
        JustShowPS(SegmentSet &SegSet, uint16_t Rate);  
        
        void 
            update(void);
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
};

#endif