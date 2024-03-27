#ifndef RateCtrlPS_h
#define RateCtrlPS_h

#include "Effects/EffectBasePS.h"

/* 
A utility class that produces a output that transitions smoothly between a start and end value (uint16_t's). 
The output is intended to be used as an effect's update rate, speeding up or slowing down the effect, 
however, the output value is just a common uint16_t and could be easily be used for other purposes. 

The output value is stored as `outputRate`. 

You can use the output rate in your effects like:
    yourEffect.rate = &yourRateControl.outputRate;
    //The line above binds an effect's "rate" setting to the "outputRate" of the rate control utility
    //Because the effect's update "rate" is a pointer, but the utility's outputRate is not, 
    //we use "&" to bind it by address.
    //Note that this means that both settings point to same place in memory
    //If you want to change the effect's rate to something else, you'll need to bind it to a new variable.

Note that you cannot use the `outputRate` in an effect's constructor, you must bind it after the effect has been created.

    Feature Notes:
        * The direction of the transition is automatically determined by the utility; 
          no need to order the start and end values. 

        * The utility is designed to transition once from the start to end values, 
          setting its `rateReached` flag to true when finished. However it can be set to loop (using `looped`), 
          which will cause the utility to constantly transition back and forth between the start/end rates.

        * When looping, you can configure the utility to pause for a set time (by setting a non-zero `pauseTime` value) 
          once the end value is reached. Note that the `rateReached` flag is set true before the pause begins.

        * You can set the utility to use easing (via `easing`) for the transition curve. 
          This will make the transition non-linear by using FastLED's `ease8InOutApprox`, 
          slowing the rate of transition as it approaches the end value. 

    Other Notes:
        * You may change the utility's settings on the fly, including the start and end values. 
          However, be sure the changing the start or end value wouldn't change the direction of the transition. 
          ie if the original start value was less than the end, any new start value must still be less than any 
          new end value, and visa versa. There is also a `reset()` function for totally changing the values. 

        * The utility shifts towards the end value by 1 each update, so the update `rate` sets how 
          fast the output value transitions.

        * To perform easing, the utility actually adjusts its own update rate. 
          The utility's update `rate` setting is not changed by the easing, but is used to calculate a 
          temporary update rate.

Example calls: 
    RateCtrlPS rateCtrl(90, 20, false, true, 0, 100);
    Transitions between a rate of 90ms to 20ms and back again (looped is on)
    The pause time between transitions is 0.
    Easing is turned off, the utility updates every 100ms.

Constructor Inputs:
    startRate -- The starting value of the transition output.
    endRate -- The ending value of the transition output.
    easing -- If true, the transition will change non-linearly at the start and end. 
              (using FastLED's ease8InOutApprox function)
    looped -- If true, the utility will loop after reaching the end rate, 
              cycling between the start and end rate constantly.
    pauseTime -- Sets a time (ms) that the utility will pause for after a transition. Only used when looping.
    rate -- The update rate of the utility (ms)

Outputs: 
    outputRate -- The output rate of the utility (see intro).

Other Settings:
    active (default true) -- If false, the utility will be disabled (updates() will be ignored).
    
Functions:
    reset(newStartRate, newEndRate) -- resets the utility with a new start and end rate, and restarts the transition.
    reset() -- Restarts the transition using the current start and end rates.
    update() -- updates the utility.

Reference Vars:
    direct -- The "direction" the output value is shifting.
              If true, the output is being incremented, false; being decremented

Flags:
    rateReached -- Set true when the utility reaches the end rate, is reset either via reset() or when looping.

*/
class RateCtrlPS : public EffectBasePS {
    public:
        RateCtrlPS(uint16_t StartRate, uint16_t EndRate, bool Easing, bool Looped, uint16_t PauseTime, uint16_t Rate);

        uint16_t
            startRate,
            endRate,
            pauseTime = 0,
            outputRate;

        bool
            looped,
            rateReached, //set true when a transition has finished
            easing,
            direct;  //for reference only, don't set this!!

        void
            update(),
            reset(uint16_t newStartRate, uint16_t newEndRate),
            reset();

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        int8_t
            stepDirect;

        uint8_t
            ratio;

        uint16_t
            rateTemp;
};

#endif