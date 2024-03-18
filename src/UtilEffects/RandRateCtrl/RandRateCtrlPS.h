#ifndef RandRateCtrlPS_h
#define RandRateCtrlPS_h

#include "Effects/EffectBasePS.h"
#include "UtilEffects/RateRandomizer/RateRandomizerPS.h"
#include "UtilEffects/RateCtrl/RateCtrlPS.h"

/* 
A utility that combines the effects of RateCtrl.h and RateRandomizer.h 
to produce an output rate that varies around a base rate +/- a set limit. 
For example with a base rate of 80 and and range of +/- 20, the output rate could be any number between 60 and 100. 
The utility smoothly transitions to the current target rate before choosing a new target. 
You can use the output rate to randomly vary the speed of an effect. 

The randomized rate is stored as `outputRate`. 

You can use the output rate in your effects like:
    yourEffect.rate = &yourRandRateControl.outputRate;
    //The line above binds an effect's "rate" setting to the "outputRate" of the rate control utility
    //Because the effect's update "rate" is a pointer, but the utility's outputRate is not, 
    //we use "&" to bind it by address.
    //Note that this means that both settings point to same place in memory
    //If you want to change the effect's rate to something else, you'll need to bind it to a new variable.

Note that you cannot use the `outputRate` in an effect's constructor, you must bind it after the effect has been created.

    Notes:
        * You can change all of the utility's settings on the fly.

        * The utility always starts a new transition to an new random rate when 
          it reaches the current target rate. For a one-time transition see RateCtrl.h.

        * The utility will shift towards the target rate by 1 every update, 
          so the utility's update `rate` controls how fast the output rate transitions.

        * The upper random range limit is **must be** greater than the lower limit! ie, `rateRangeMax > rateRangeMin`.

        * You can have the utility pause for a set time (`pauseTime`) once a transition is complete. 
          Once the pause time is up, the utility will start a new transition. 
          However, because the utility picks the next target rate at random, it's possible (likely even!) 
          that the new target will be quite close to the current rate. 
          This means that the utility will pause again, leaving the output rate relatively the same for an 
          extended period of time. 

          To combat this, and keep the rate varied, the pause time is scaled by the difference 
          between the current and target rate. The larger the rate difference, the closer the 
          pause time will be to the `pauseTime` setting. So, when the rate difference is small, 
          we transition to it quickly and only pause for a moment, but when the difference is large, 
          we spend more time transitioning and pause for longer. 
          Overall, this helps the rate vary more often, while still preserving pausing functionality.

          The scaling is controlled by `usePauseRatio` (bool) and is turned on by default. 
          When turned off, the pause time will always be the `pauseTime` setting. 
          Note that the `pauseTime` setting is _`not`_ modified by the scaling 
          (the utility uses an internal variable for the actual pause time), 
          so you can change if freely.

Example calls: 
    RandRateCtrlPS randRateCtrl(80, -60, 40, 0, 160);
    Produces a rate chosen randomly using base of 80ms, 
    and upper and lower bounds of -60, +40ms,
    so the output rate will be vary from 20 to 120.
    The pause time between rate transitions is 0
    The output rate will be adjusted every 160ms.

Constructor Inputs:
    baseRate -- The base rate for the output rate. 
                The random output rates are calculated as baseRate + random(rateRangeMin, rateRangeMax).
                Like the utility's update `rate`, this is a pointer, allowing you to bind it to an external variable. 
                By default it's bound the effect's local variable, `baseRateOrig`.
    rateRangeMin (int32_t, so it can be negative) -- The lowest possible random amount to be added to the baseRate.
                                                     **Must be** less than rateRangeMax.
    rateRangeMax (int32_t, so it can be negative) -- The highest possible random amount to be added to the baseRate.
                                                     **Must be** greater than rateRangeMax.
    pauseTime -- The time (ms) the utility will pause for after finishing a transition.
    rate -- The update rate of the utility (ms) 

Other Settings:
    usePauseRatio (default true) -- If true, then the utility will adjust the pause time after each transition to be
                                    in proportion to the difference between the current output rate and the new target rate.
                                    (see notes in intro)
    active (default true) -- If false, the utility will be disabled (updates() will be ignored).

Functions:
    reset() -- Picks a new random update rate and removes any pauses.
    resetToBaseRate() -- Resets the outputRate to the base rate. 
                         Also turns the utility off by setting "active" to false.
    pickNewRate() -- Picks a new end rate to transition to (you shouldn't need to call this).
    update() -- updates the utility.

Flags:
    paused -- Set true when the utility is paused. Is reset to false once the pause time has passed.
*/
class RandRateCtrlPS : public EffectBasePS {
    public:
        RandRateCtrlPS(uint16_t BaseRate, int32_t RateRangeMin, int32_t RateRangeMax, uint16_t PauseTime, uint16_t Rate);

        uint16_t
            baseRateOrig,
            *baseRate = nullptr,  //the input rate of the utility
            pauseTime = 0,
            outputRate;  //the output rate of the utility

        int32_t
            rateRangeMin,
            rateRangeMax;

        bool
            paused = false,
            rateReached = false,
            usePauseRatio = true;

        void
            pickNewRate(),
            reset(),
            resetToBaseRate(),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0,
            pauseStartTime = 0;

        int8_t
            stepDirect;

        uint16_t
            endRate,
            rateRangeDiff,
            startEndDiff,
            pauseTimeFinal;

        int32_t
            temp;

        bool
            direct;

        void
            updateRate();
};

#endif