#ifndef EffectBasePS_h
#define EffectBasePS_h

#include "SegmentFiles.h"

//macros

//bind the segmentSet pointer since it is inherited from BaseEffectPS
#define bindSegPtrPS() ({        \
    segmentSetPtr = &segmentSet; \
})                               \

//bind the rate vars since they are inherited from BaseEffectPS
//the rate is used to control the update cycle
//rate is a pointer, by default it is bound to rateOrig, 
//which is set equal to the rate from the effect constructor
//but you can re-bind it to an external variable for external rate control
#define bindClassRatesPS() ({  \
    (rateOrig = Rate);         \
    (rate = &rateOrig);        \
})                             \

//Used for binding the background color used in some effects
//we want to use a pointer for the bgColor so it can be tied to an external variable easily
//so we use the bgColorOrig to store the BgColor from the constructor, 
//and point the bgColor to it
//in your .h class file include
//CRGB 
    //bgColorOrig,
    //*bgColor;
// and use BgColor in your constructors
#define bindBGColorPS() ({      \
    (bgColorOrig = BgColor);    \
    (bgColor = &bgColorOrig);   \
})                              \

//checks the effect is set to show, if so, do so
#define showCheckPS() ({    \
    if(showOn){             \
        (FastLED.show());   \
    }                       \
})                          \

//This is the base effect class from which effects are derived (and some other helper functions)
//contains:
    //update() interface method (all effects must have an update method)
    //a segmentSet pointer to access the effect's segmentSet from outside the effect
    //a few macros for common effect code pieces (see above)
class EffectBasePS {
    public:
        uint16_t 
            rateOrig,
            *rate;
        
        bool
            showOn = true;

        //pointer to the segmentSet the effect is using
        //defaults to null, since a couple of other classes use the effectBase but don't use a segmentSet
        //this sounds bad, but it lets you use them in an an EffectGroup, which is very useful
        SegmentSet 
            *segmentSetPtr = nullptr;

        virtual void update() = 0;

        virtual ~EffectBasePS() = 0;

};

#endif