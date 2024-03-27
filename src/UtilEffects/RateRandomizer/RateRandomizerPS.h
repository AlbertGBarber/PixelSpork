#ifndef RateRandomizerPS_h
#define RateRandomizerPS_h

#include "Effects/EffectBasePS.h"

/* 
A utility class whose output is a random offset from a base number (uint16_t). 
For example, with a base number of 80 and and offset range of +/- 20, the output could be any number between 60 and 100. 
This is useful for randomly varying the update rate of effects, but could be used for other purposes. 
Note that this is a full class, so the utility produces a new output value at regular intervals as it is 
updated at its update `rate`. 

The output value is stored as `outputRate`. 

You can use the output rate in your effects like:
    yourEffect.rate = &yourRateRandomizer.outputRate;
    //The line above binds an effect's "rate" setting to the "outputRate" of the rate control utility
    //Because the effect's update "rate" is a pointer, but the utility's outputRate is not, 
    //we use "&" to bind it by address.
    //Note that this means that both settings point to same place in memory
    //If you want to change the effect's rate to something else, you'll need to bind it to a new variable.
    
Note that you cannot use the `outputRate` in an effect's constructor, you must bind it after the effect has been created.

Finally, note that the base rate (`baseRate`) is a pointer, like the typical effect `rate` setting. By default it is bound to the local utility setting `baseRateOrig`, which is set to whatever `BaseRate` value you use in the constructor. See [common vars](https://github.com/AlbertGBarber/PixelSpork/wiki/Common-Effect-Variables-and-Functions) `rate` entry for more info. 

Example calls: 
    RateRandomizerPS rateRand(80, -10, 10, 300);
    Produces a  random rate using a base of 80, 
    and upper and lower bounds of +/- 10.
    So the output will be vary from 70 to 90
    The output rate will change every 300ms.

Constructor Inputs:
    baseRate -- The base rate that the random rate is added/subtracted from. 
                Like the utility's update `rate`, it is a pointer. 
                By default it's bound to the utility's local variable, `baseRateOrig`.
    rateRangeMin (int32_t, range +/- 65535) -- The lowest possible value that can be added to the baseRate.
    rateRangeMax (int32_t, range +/- 65535) -- The highest possible value that can be added to the baseRate.
    rate -- The update rate (ms).

Outputs: 
    outputRate -- The output rate of the utility (see intro).

Other Settings:
    active (default true) -- If false, the effect will be blocked from updating
    
Functions:
    resetToBaseRate() -- Resets the outputRate to the baseRate. 
                         Also sets "active" to false, turning off the utility.
    update() -- Updates the utility.
*/
class RateRandomizerPS : public EffectBasePS {
    public:
        RateRandomizerPS(uint16_t BaseRate, int32_t RateRangeMin, int32_t RateRangeMax, uint16_t Rate);

        int32_t
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

        int32_t
            tempOut;
};

#endif