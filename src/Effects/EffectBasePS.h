#ifndef EffectBasePS_h
#define EffectBasePS_h

//TODO: Add virtual reset() function (also add it to EffectSet), not all effects have reset()'s

#include "Include_Lists/SegmentFiles.h"

//macros

#define bindSegSetPtrPS() ({ \
    (segSet = &SegSet);      \
})

//bind the rate vars since they are inherited from BaseEffectPS
//the rate is used to control the update cycle
//rate is a pointer, by default it is bound to rateOrig,
//which is set equal to the rate from the effect constructor
//but you can re-bind it to an external variable for external rate control
#define bindClassRatesPS() ({ \
    (rateOrig = Rate);        \
    (rate = &rateOrig);       \
})

/* Used for binding the background color used in some effects
we want to use a pointer for the bgColor so it can be tied to an external variable easily
so we use the bgColorOrig to store the BgColor from the constructor,
and point the bgColor to it
in your .h class file include
CRGB
    bgColorOrig,
    *bgColor;
and use BgColor in your constructors */
#define bindBGColorPS() ({    \
    (bgColorOrig = BgColor);  \
    (bgColor = &bgColorOrig); \
})

//checks the effect is set to show, if so, do so
#define showCheckPS() ({                    \
    (segDrawUtils::show(*segSet, showNow)); \
})

/* This is the base effect class from which effects are derived (and some other helper functions)
contains:
    update() interface method (all effects must have an update method)
    a SegmentSetPS pointer to access the effect's SegmentSetPS from outside the effect
    a few macros for common effect code pieces (see above) */
class EffectBasePS {
    public:
    
        uint16_t
            rateOrig,
            *rate = nullptr;

        bool
            showNow = true;

        //pointer to the SegmentSetPS the effect is using
        //defaults to null, because utility classes use the effectBase but don't use a SegmentSetPS
        //this sounds bad, but it lets you use them in EffectSets, which is very useful
        SegmentSetPS
            *segSet = nullptr;

        //virtual update function to be implemented in each effect
        //making it virtual so that the update functions of effects can be called from the EffectBase class
        //This is used in the EffectGroup class to update multiple effects
        virtual void update() = 0;

        //similar to the virtual update function, allows the deletion of any class instance derived from
        //the EffectBase class
        virtual ~EffectBasePS() = 0;
};

#endif