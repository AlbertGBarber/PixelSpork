#ifndef RateNoisePS_h
#define RateNoisePS_h

#include "Effects/EffectBasePS.h"
#include "GeneralUtils/generalUtilsPS.h"
//#include "MathUtils/mathUtilsPS.h"

/*
A utility that uses FastLED Perlin noise (https://github.com/s-marley/FastLED-basics/tree/main/6.%20Noise)
to produce a output value that varies randomly, but smoothly over time, and is locked to
withing a set range. The output is intended to be used as an effect's update "rate" setting
to randomly vary the effect's speed. However, there's nothing special about the output, 
it's just a uint16_t value, so you could easily use it for other purposes.

The output rate is stored as `outputRate`. 

You can use the output rate in your effects like:
    yourEffect.rate = &yourRateNoise.outputRate;
    //The line above binds an effect's "rate" setting to the "outputRate" of the utility
    //Because the effect's update "rate" is a pointer, but the utility's outputRate is not, 
    //we use "&" to bind it by address.
    //Note that this means that both settings point to same place in memory
    //If you want to change the effect's rate to something else, you'll need to bind it to a new variable.

Note that you cannot use the `outputRate` in an effect's constructor, you must bind it after the effect has been created.

The utility only has a few settings. These can all be changed freely on the fly.

Notes:
    * FastLED noise tends to mostly fall in the center of it's range,
      so you may need to expand your min and max range values to have a "full" real range.

    * Because Perlin noise is not truly random (same input -> same output), to add an extra degree of variation I have
      introduced an extra random factor, "randFact", which adds a slow shift to the noise. It's value is randomized
      on start up, but you may want to re-randomize it by calling "setRandFact()" when you first use the utility. 
      You may also set randFact to 0 to ignore it entirely. 

Example calls: 
    RateNoisePS rateNoise(20, 120, 10, 80);
    Produces an output rate that varies smoothly between 20 and 120. 
    The speed multiplier of the noise is 10.
    The utility updates at a rate of 80ms.

Constructor Inputs:
    rateMin -- The minimum possible value of the output rate.
    rateMax -- The maximum possible value of the output rate.
    noiseSpeed -- How quickly the output rate changes. Is a multiplier, so the effect amplifies at higher values.
                  Recommend values between 5 - 40. Effects become very "jerky" at 100+. Higher -> faster rate change.
    rate -- The update rate of the utility. Note that a faster update rate will not increase how fast the noise changes,
            but will make it "smoother".
        
Outputs:
    outputRate -- The output rate of the utility (see intro).

Other Settings:
    randFact (set to a random value) -- A randomly set noise adjustment. 
                                        Adds more variation to the noise output (see "Notes" above). 
                                        Setting it to 0 will remove its effect. 
                                        Can be re-randomized by calling setRandFact(), or can be set manually.
    active (default true) -- If false, the utility will be disabled (updates() will be ignored).

Functions:
    setRandFact() -- Sets a new random "randFact" value.
    update() -- Updates the utility.

*/
class RateNoisePS : public EffectBasePS {
    public:
        RateNoisePS(uint16_t RateMin, uint16_t RateMax, uint16_t NoiseSpeed, uint16_t Rate);

        uint16_t
            outputRate, //The output rate of the utility
            rateMin,
            rateMax,
            randFact,
            noiseSpeed; //how quickly the outputRate changes higher->faster, rec values between 5 - 40?

        void   
            setRandFact(),
            update(void);

    private:
        unsigned long
            currentTime,
            prevTime = 0;
        
        uint16_t 
            noiseData;
};

#endif