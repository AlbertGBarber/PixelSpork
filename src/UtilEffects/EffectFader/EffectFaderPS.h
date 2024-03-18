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
A utility class for fading effects in and out, like when a song fades out at its end. 
Useful for smoothly transitioning between effects. 
Uses the effects' segment set brightness value to do the fading. 
See https://github.com/AlbertGBarber/PixelSpork/wiki/Advanced-Segment-Usage#brightness for info.

Requires an EffectSetPS to function.

Note that this utility will fades effects either in or out once, 
and must be manually reset once finished. 
For a more automated version, see EffectSetFaderPS.h.

Also note that due to the way the segment set brightness works, this utility may not work well 
with any effects that need to read or add LED colors from the FastLED `leds` array (See brightness link above for more).

    Setup:
        To setup the utility, you'll first need an EffectSetPS and an array of effects.

        For Example:
            EffectBasePS *effArray[1] = {nullptr};
            EffectSetPS effectSet(effArray, SIZE(effArray), 0, 10000); //(run time 1000ms)

        You'll need to populate this set with an effect yourself.

        You'll then create the Fader utility:
            EffectSetFaderPS setFader(effectSet, true, 3000); //(run time 3000ms)

        The Fader instance, `setFader`, is setup to fade the effects from 0 to 100% over 3000ms.

        When you update you `effectSet`, also call `setFader.update()` to update the Fader.

        Once the effect is faded in, the Fader will be "done", and needs to be `reset()` to restart.

    Notes:   
        * The Fader works by changing the brightness of the effect(s) segment set(s). 
          It's fine to have multiple effects that share segment sets; the Fader is coded to only set the brightness 
          for a segment set once per cycle.

        * Effects will either be brightened up from the Fader's minimum brightness (see below) or darkened towards it.

        * The Fader assumes that the supplied segments are already set to the normal 
          brightness (whatever you set that to be). 
          You do not need to pre-adjust them to the fade starting point, the Fader handles all the busy work. 

        * The Fader records the initial brightness of the segment sets during the first update() call. 
          You can use `resetBrightness()` to restore the segment sets to the initial brightness.

        * _**DO NOT**_ `update()` if the Effect Set is empty; the segment set brightness's will be default to 255, 
          and fading in will not work.

        * If you change the segment set's brightness while the Fader is running, 
          the Fader may either not finish, or finish early.

        * _**DO NOT**_  change with the Fader's `runTime` or fade direction (`direct`), while the fade is running.

Example Calls:
    (see setup guide above, but for reference)
    EffectFaderPS effectFader(effectSet, false, 2000)
    Will create a fader to fade the effects in the effect set out (direction is false), 
    taking 2000ms for the fades.

Constructor Inputs:
    effectSet -- The effect set that the fader will operate on (see EffectSetPS.h)
    direct -- The direction of the fade (true is fading in, false out) 
              Change this only once a fade is "done" or not yet started.
    runTime -- The time (ms) taken to fade in/out. 
               Changing this will only be reflected when you reset().

Other Settings:
    rateOrig (default 60 ms) -- The default update rate for the fader
    *rate (default bound to rateOrig) -- The update rate for the fader. You only need to change this if the brightness changes look "choppy".
                                         It is a pointer, so it can be tied to and external var if needed.
    minBrightness (default 5, max 255) -- The minimum brightness the fade will reach or start from. (255 is fully bright).

Functions:
    reset() -- Restarts the fader.
    reset(&EffectSet, newDirect) -- Resets the fader with a new EffectSet and direction
    resetBrightness() -- Resets the segment set(s) to their original brightness setting(s) 
                         (as recorded during the first fader update())
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