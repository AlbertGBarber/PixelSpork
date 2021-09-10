#ifndef EffectBasePS_h
#define EffectBasePS_h

#include "SegmentFiles.h"

//macros

//bind the segmentSet pointer since it is inherited from BaseEffectPS
#define bindSegPtrPS() ({        \
    segmentSetPtr = &segmentSet; \
})                               \

//bind the rate vars since they are inherited from BaseEffectPS
#define bindClassRatesPS() ({  \
    (this->rate = Rate);       \
    (this->rateG = &rate);     \
})                             \

//used for setting the internal effect rate to the value of an external global variable
//(if th effect is using one)
#define globalRateCheckPS() ({   \
    if ((useGlobalRate)) {       \
        (rate) = (*rateG);       \
    }                            \
})

//checks the effect is set to show, if so, do so
#define showCheckPS() ({    \
    if(showOn){             \
        (FastLED.show());   \
    }                       \
})                          \

//This is the base effect class from which effects are derived (and some other helper functions)
//contains:
    //update() interface method (all effects must have an update method)
    //setGlobalRate and related vars, used for binding an effect's rate to an external variable
    //a segmentSet pointer to access the effect's segmentSet from outside the effect
    //a few macros for common effect code pieces (see above)
class EffectBasePS {
    public:
        uint16_t 
            rate;
        
        bool
            showOn = true,
            useGlobalRate;

        //pointer to the segmentSet the effect is using
        //defaults to null, since a couple of other classes use the effectBase but don't use a segmentSet
        //this sounds bad, but it lets you use them in an an EffectGroup, which is very useful
        SegmentSet 
            *segmentSetPtr = nullptr;

        virtual void update() = 0;

        virtual ~EffectBasePS() = 0;

        //binds the update rate to a global variable as passed in
        //useful for cordinating multiple effects
        void setGlobalRate(uint16_t &rate){
            rateG = &rate;
            useGlobalRate = true;
        }

    protected:

        uint16_t
            *rateG;

};

#endif