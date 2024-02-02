#ifndef RateRandomizerPS_h
#define RateRandomizerPS_h

#include "Effects/EffectBasePS.h"

/* 
Utility that produces a random offset from a base number
ie with a base number of 80 and and offset range of +/- 20, the output could be any number between 60 and 100
Useful for varying the update rate of effects randomly, to add some extra variation

Generally it's useful to use the same base number as the rate of your effect
Picking any update rate for the RateRandomizer that is double or more that the base rate generally looks good

The baseRate and rate are both pointers, so you can point them to external variables
ie baseRate = &yourRate

The randomized rate is stored in outputRate. Use &outputRate to bind it to an effect rate.

Example calls: 
    RateRandomizerPS rateRand(80, -10, 10, 300);
    Produces a rate chosen randomly using base of 80, and upper and lower bounds of +/- 10 
    so the output will be vary from 70 to 90
    the output rate will change every 300ms

Constructor Inputs:
    baseRate -- The base rate that the random rate is added/subtracted from
    rateRangeMin (int16_t, so it can be negative) -- The lowest possible random amount to be added to the baseRate
    rateRangeMax (int16_t, so it can be negative) -- The highest possible random amount to be added to the baseRate
    rate -- The update rate (ms)

Other Settings:
    active (default true) -- If false, the effect will be blocked from updating
    
Functions:
    resetToBaseRate() -- resets the outputRate to the baseRate
    update() -- updates the effect
*/
class RateRandomizerPS : public EffectBasePS {
    public:
        RateRandomizerPS(uint16_t BaseRate, int16_t RateRangeMin, int16_t RateRangeMax, uint16_t Rate);

        int16_t
            rateRangeMin,
            rateRangeMax;

        uint16_t
            baseRateOrig,
            *baseRate = nullptr,
            outputRate;

        void
            resetToBaseRate(),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        int8_t
            direct;

        int32_t
            tempOut;
};

#endif