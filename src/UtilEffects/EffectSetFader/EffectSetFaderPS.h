#ifndef EffectSetFaderPS_h
#define EffectSetFaderPS_h

#include "Effects/EffectBasePS.h"
#include "UtilEffects/EffectFader/EffectFaderPS.h"
#include "UtilEffects/EffectSet/EffectSetPS.h"
//#include "MathUtils/mathUtilsPS.h"

/*
A util for fading effects in and out, like when a song fades out at it's end, for smoothly transitioning between effects
This util is intended to be added to the effects in an effect set (See EffectSetPS.h for info) to automatically
fade effects in and out. 

You can configure the fade time, and if the effects fade in, out, or both.

Note that the fader works by changing the brightness of the effect(s) segment set(s)
It's fine to have multiple effects that share segment sets; the fader only sets the brightness for a segment set once per cycle.

DO NOT call update() if the Effect Set is empty; the segment set brightnesses will be default to 255, and fading in will not work.

Also, the util's EffectSet and EffectFader instances are public, so you can modify them directly,
although you shouldn't need to.

Setup, Running, and Resetting:
    To setup the Util, you'll first need an effect set and an effect array (See EffectSetPS.h for info)
    like:
        EffectBasePS *effArray[2] = {nullptr, nullptr};
        EffectSetPS effectSet(effArray, SIZE(effArray), 1, 10000); //(run time 1000ms)
    *Note that I've specified the effArray to be size 2, and that in the EffectSet, 
    I've specified the destruct limit to be 1, this will because we're going to add the EffectSetFader to the 
    effectSet in the first array index. The second index is for our actual effect. 

    You'll then create the fader with a run time:
        EffectSetFaderPS setFader(&effectSet, 3000); //(run time 3000ms)
    
    Finally, in the Arduino setup() function, you'll want to add the setFader to the effArray in the first index:
        effArray[0] = &setFader;
    This adds the setFader to the effectSet, so that the fader will be updated at the same time as the other effects.

    !!!It's IMPORTANT that the fader is placed in the array before any effects, otherwise, you'll see a flash when the 
    set if first updated.!!!!!

    At this point, when you update() the effectSet, the effect will be faded in and out (taking 3000ms for both fades)
    as the effectSet starts/ends. (The fades run concurrently with the effect, so the effect spends 4000ms at full brightness 
    (10000 - 3000 *2)

    Once the effectSet is done, you'll probably want to reset() it, either to restart the effect, or swap in a new effect.
    The EffectSetFader only runs once, and must be manually reset, with it's own reset() function.
    You should do this wherever you reset your EffectSet. 
    !!!BUT DO SO BEFORE destructing any effects, since the fader needs access to their segment sets!!!!

Example Calls:
    (see setup guide above, but for reference)
    EffectSetFaderPS effectSetFader(effectSet, 2000)
    Will create a fader to fade the effects in the effect set in and out, 
    taking 2000ms for the fades.
    
Constructor Inputs:
    effectSet -- The effect set that the fader will operate on (see EffectSetPS.h)
    fadeRunTime -- The time in ms that the fader take to fade in/out (to change this later use setupFader(newFadeRunTime))

Other Settings:
    rateOrig (default 60 ms) -- The default update rate for the fader
    *rate (default bound to rateOrig) -- The update rate for the fader. You only need to change this if the brightness changes look "choppy".
                                         It is a pointer, so it can be tied to and external var if needed.
    fadeIn (default true) -- If false, no fade ins will be done
    fadeOut (default true) --  If false, no fade outs will be done

Functions:
    setupFader(newFadeRunTime) -- Changes the run time of the fader, also restarts the fader
    reset() -- Restarts the EffectSetFader, and also resets the EffectFader if it exists
    reset(&newEffectSet, newFadeRunTime) -- Resets the fader to use a new effect set and fade run time.
    resetFader() -- Resets the EffectFader, restarting it (see EffectFadePS reset())
    resetBrightness() -- Resets the segment set(s) to their original brightness setting(s) (as recorded during the first update()) 
                         (see EffectFadePS resetBrightness())
    update() -- Updates the fader. If the fader is part of a EffectSet, this will be called whenever the set is updated

Reference Vars:
    fadeRunTime -- The time in ms that the fader take to fade in/out (set using setupFader(newFadeRunTime))

Flags:
    fadeInStarted (default false) -- Set true if a fade in has started
    fadeOutStarted (default false) -- Set true if a fade out has started
*/
class EffectSetFaderPS : public EffectBasePS {
    public:
        EffectSetFaderPS(EffectSetPS &EffectSet, uint16_t FadeRunTime); 

        ~EffectSetFaderPS(); 

        uint16_t
            fadeRunTime, //For reference, set with setupFader()
            rateOrig = 60, //The default update rate (ms)
            *rate = &rateOrig; //The update rate is a pointer, so it can be tied to and external var if needed, 
                               //by default it's tied to rateOrig
        
        bool
            fadeIn = true,
            fadeOut = true,
            fadeInStarted = false,
            fadeOutStarted = false;
        
        EffectSetPS
            *effectSet = nullptr;

        EffectFaderPS
            *effectFader = nullptr;

        void 
            setupFader(uint16_t newFadeRunTime),
            reset(EffectSetPS &newEffectSet, uint16_t newFadeRunTime),
            reset(),
            resetFader(),
            resetBrightness(),
            update();
    
    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint16_t
            *runTime = nullptr,
            *setTimeElap;
        
        bool
            *infinite = nullptr,
            firstUpdate = true,
            resetNextUpdate = false;
};

#endif