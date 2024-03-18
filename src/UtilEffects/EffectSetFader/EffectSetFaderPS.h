#ifndef EffectSetFaderPS_h
#define EffectSetFaderPS_h

#include "Effects/EffectBasePS.h"
#include "UtilEffects/EffectFader/EffectFaderPS.h"
#include "UtilEffects/EffectSet/EffectSetPS.h"
//#include "MathUtils/mathUtilsPS.h"

/*
A utility class for fading effects in and out, allowing a smooth transitioning between effects, 
like when a song fades out as it ends. It is intended to be used as part of an EffectSetPS 
to fade the effects in the set. The utility will automatically fade effects in the set in and out as 
the set approaches its `runTime`. 
You can configure the utility's fade time, and if the effects fade in, out, or both.

Note that the fade time runs concurrently with effects, so if you have an effect with a run time of 10sec 
and a fader fading in and out over 2sec, the time where the effect is 
not fading will be 6sec: the 10sec run time minus the 4sec spent fading in and out.

    Notes:
        Under the hood, the utility uses an instance of the EffectFaderPS utility class to handle the fading, 
        while the set fader handles the timing and resetting of the instance. 
        The instance is public, and can be accessed `effectFader` in the utility. 
        The fader works by changing the brightness of the effect(s) segment set(s).
        See https://github.com/AlbertGBarber/PixelSpork/wiki/Advanced-Segment-Usage#brightness.

        Note that any restrictions that apply to the EffectFaderPS utility also apply to this utility. 
        This generally means that the fader may not work well with any effects that need to read or add LED colors 
        from the FastLED `leds` array (see previous link for more).

        **_DO NOT_** `update()` the utility if the Effect Set is empty; the segment set brightness's will be default to 255, 
        and fading in will not work.

    Setup, Running, and Resetting:
        To setup the utility, you'll first need an Effect Set and array of effect(s).

        For Example:
            EffectBasePS *effArray[2] = {nullptr, nullptr};
            EffectSetPS effectSet(effArray, SIZE(effArray), 1, 14000); //(run time 14000ms)

        Note that we're going to add our Effect Set Fader to the Effect Set, putting it in the first index, 
        so I've made the effect array length 2. We will use the second index for our actual effect, 
        which you'll need to create and add yourself. 
        Also note that I've set the "destuct limit" in the effect set to 1, 
        which prevents us from accidentally destructing the Set Fader. 
        This is only relevant if you plan on dynamically allocating your effects (you probably don't).

        With the Effect Set created, we need to create our set fader:       
            EffectSetFaderPS setFader(effectSet, 3000); //(run time 3000ms)

        Note that we use our effect set as an input to the fader. This tells the fader to fade only effects in that set. 
            
        Finally, in the Arduino `setup()` function, we need to add the fader to the effect set in the first index:
            effectSet.setEffect(&setFader, 0); 
            //note that the setEffect function expects an effect pointer, so we need to pass our 
            //fader by address using &.
        
        This adds the fader to the effect set, so that the fader will be updated alongside our effect.

        _**IMPORTANT**_: the fader should be placed in the effect array before any effects, otherwise, 
        you'll see a flash when the set first updates.

        At this point, when you update the effect set, the effect will be faded in and out 
        (taking 3000ms for both fades) as the effect set starts/ends. 
        Note that the fades run concurrently with the effect, 
        so the effect spends 8000ms (14000 - 3000 * 2) at full brightness.

        Once the effect set has run, you'll probably want to `reset()` it, either to restart the effect, 
        or swap in a new effect. Do to some code complications, the fader utility only runs once, 
        and must be manually reset, with it's own `reset()` function. 
        In practice, this is pretty simple, just reset the fader whenever you reset you effect set.

Example Calls:
    (see setup guide above, but for reference)
    EffectSetFaderPS effectSetFader(effectSet, 2000, true, true);
    Will create a fader with a fade time of 2000ms.
    It is configured to both fade in and out.
    
Constructor Inputs:
    effectSet -- The effect set that the fader will operate on (see EffectSetPS.h)
    fadeRunTime -- The time in ms that the fader take to fade in/out. Can be changed later using `setupFader()`.
    fadeIn (optional, default true) -- If false, no fade in will be done.
    fadeOut (optional, default true) --  If false, no fade out will be done.

Other Settings:
    rateOrig (default 60 ms) -- The default update rate for the fader
    *rate (default bound to rateOrig) -- The update rate for the fader. You only need to change this if the brightness changes look "choppy".
                                         It is a pointer, so it can be tied to and external var if needed.
    active (default true) -- If false, the utility will be disabled (updates() will be ignored).
    EffectFaderPS effectFader -- The EffectFaderPS instance. 
                                 You shouldn't need to touch this, but it's public just in case.
    
Functions:
    setupFader(newFadeRunTime) -- Changes the run time of the fader, also restarts the fader
    reset() -- Restarts the EffectSetFader, and also resets the EffectFader if it exists
    reset(&newEffectSet, newFadeRunTime) -- Resets the fader to use a new effect set and fade run time.
    resetFader() -- Resets the EffectFader, restarting it (see EffectFadePS reset()). You shouldn't need to call this.
    resetBrightness() -- Resets the segment set(s) to their original brightness setting(s) (as recorded during the first update()) 
                         (see EffectFadePS resetBrightness()). You shouldn't need to call this.
    update() -- Updates the fader. If the fader is part of a EffectSet, this will be called whenever the set is updated

Reference Vars:
    fadeRunTime -- The time in ms that the fader take to fade in/out (set using setupFader())

Flags:
    fadeInStarted (default false) -- Set true if a fade in has started
    fadeOutStarted (default false) -- Set true if a fade out has started
*/
class EffectSetFaderPS : public EffectBasePS {
    public:
        //Base constructor, will fade in and out
        EffectSetFaderPS(EffectSetPS &EffectSet, uint16_t FadeRunTime);
        
        //Constructor with fade in and out options
        EffectSetFaderPS(EffectSetPS &EffectSet, uint16_t FadeRunTime, bool FadeIn, bool FadeOut);

        ~EffectSetFaderPS();

        uint16_t
            fadeRunTime,        //For reference, set with setupFader()
            rateOrig = 60,      //The default update rate (ms)
            *rate = &rateOrig;  //The update rate is a pointer, so it can be tied to and external var if needed,
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
            prevTime = 0,
            *runTime = nullptr,
            *setTimeElap;

        bool
            *infinite = nullptr,
            firstUpdate = true,
            resetNextUpdate = false;
};

#endif