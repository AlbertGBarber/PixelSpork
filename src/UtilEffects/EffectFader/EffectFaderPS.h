#ifndef EffectFaderPS_h
#define EffectFaderPS_h

#if ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WConstants.h"
    #include "WProgram.h"
    #include "pins_arduino.h"
#endif

#include "Effects/EffectBasePS.h"
#include "UtilEffects/EffectSet/EffectSetPS.h"

/* 
A util for fading effects in and out, like when a song fades out at it's end
useful for smoothly transitioning between effects.
(although blending directly to the next effect(s) would be better, it is far more complicated
and would probably require a whole frame buffer for each effect, taking far too much memory)

Note that this util will only fade the effects either in or out, and needs manual resetting
for a more automated version, see EffectSetFaderPS

Setup:
    To setup the Util, you'll first need an effect set and an effect array (See EffectSetPS.h for info)
    like:
        EffectBasePS *effArray[2] = {nullptr};
        EffectSetPS effectSet(effArray, SIZE(effArray), 0, 10000); //(run time 1000ms)

    You'll then create the fader with a run time:
        EffectSetFaderPS setFader(&effectSet, true, 3000); //(run time 3000ms)
    This fader is setup to fade the effects in the effectSet in over 3000ms

    Then, when you update you effectSet, also call setFader.update() to update the fader.

    Once the effects are faded in, the effectFader will be "done", and needs to be reset() to restart.

Notes:   
    The fader works by changing the brightness of the effect(s) segment set(s)
    It's fine to have multiple effects that share segment sets; the fader only sets the brightness for a segment set once per cycle

    Effects will either be brightened up from the fader's minimum brightness (see below) or darkened towards it.

    The fader assumes that the supplied segments are already set to their regular brightness.
    You do not need to pre-adjust them to the fade starting point, the fader handles all the busy work.
    The fader also records the inital brightness of the segment sets during the first update() call.
    You can use resetBrightness() to restore the segment sets to the inital brightness.

    DO NOT call update() if the Effect Set is empty; the segment set brightnesses will be default to 255, 
    and fading in will not work.

    If you change the segment set's brightness while the fader is running, the fader may either not finish, or finish early.
    Don't mess with the runTime or direct, while the fade is running.

Example Calls:
    (see setup guide above, but for reference)
    EffectFaderPS effectFader(effectSet, false, 2000)
    Will create a fader to fade the effects in the effect set out (direction is false), 
    taking 2000ms for the fades.

Constructor Inputs:
    effectSet -- The effect set that the fader will operate on (see EffectSetPS.h)
    direct -- The direction of the fade (true is fading in, false out) Change this only once a fade is "done" or not started
    runTime -- The time in ms that the fader take to fade in/out. Changing this will only be reflected when you reset().

Other Settings:
    rateOrig (default 60 ms) -- The default update rate for the fader
    *rate (default bound to rateOrig) -- The update rate for the fader. You only need to change this if the brightness changes look "choppy".
                                         It is a pointer, so it can be tied to and external var if needed.
    minBrightness (default 5, max 255) -- The minimum brightness the fade will reach or start from.

Functions:
    reset() -- Restarts the fader
    reset(&EffectSet, newDirect) -- Resets the fader with a new EffectSet and direction
    resetBrightness() -- Resets the segment set(s) to their original brightness setting(s) (as recorded during the first update())
    update() -- Updates the fader. 

Flags:
    done (default false) -- Set true if a fade has finished
    started (default false) -- Set true if a fade has started
*/
class EffectFaderPS {

    public:
        EffectFaderPS(EffectSetPS &EffectSet, bool Direct, uint16_t RunTime);

        ~EffectFaderPS();

        bool
            direct,           //true -> increasing brightness
            done = false,     //is the fade finished?
            started = false;  //has a fade been started?

        uint8_t
            minBrightness = 5;

        uint16_t
            runTime,            //If you change this, it will only be reflected when you reset()
            rateOrig = 60,      //The default update rate (ms)
            *rate = &rateOrig;  //The update rate is a pointer, so it can be tied to and external var if needed,
                                //by default it's tied to rateOrig

        void
            reset(),
            resetBrightness(),
            reset(EffectSetPS &EffectSet, bool newDirect),
            update();

        EffectSetPS
            *effectSet;  //Pointer to the effect set, shouldn't need to mess with this, but is public just in case

    private:
        unsigned long
            startTime,
            currentTime,
            prevTime = 0;

        uint8_t
            *numEffects,
            maxBrightness,
            totalBriChng = 0,
            *origBrightness_arr = nullptr;

        int16_t
            briChng;

        float
            stepRate;

        SegmentSetPS
            *getSegPtr(uint8_t effectNum),
            *segSet = nullptr;

        void
            resetSegVars();
};

#endif