#ifndef RandRateCtrlPS_h
#define RandRateCtrlPS_h

#include "Effects/EffectBasePS.h"
#include "UtilEffects/RateRandomizer/RateRandomizerPS.h"
#include "UtilEffects/RateCtrl/RateCtrlPS.h"

//A utility that combines the effects of RateCtrl and RateRandomizer to
//Produce a rate that varies around a base rate in a range specified
//The rate will smoothly transition from one rate to the next
//Ie with a base rate of 80 and and range of +/- 20, the target rate could be any number between 60 and 100
//the output rate will smoothly transition to the target, and then choose a new target rate

//Generally it's useful to use the same base number as the rate of your effect
//Picking any update rate for that is double or more that the base rate generally looks good

//To produce the rates, the class uses an instance of both RateCtrl and RateRandomizer, called RC and RR respectively
//These are automatically set up when the RandRateCtrl is created
//You can change their settings using the functions listed below

//You can also pause the rate after it has transitioned using the holdTime variable

//The randomized rate is stored in outputRate. Use outputRate to bind it to an effect rate.

//The RandRateCtrlPS update rate is a pointer, and can be bound externally, like in other effects

//Example calls: 
    //RandRateCtrlPS(80, -20, 20, true, 160);
    //Produces a rate choosen randomly using base of 80, and upper and lower bounds of +/- 20
    //so the output will be vary from 60 to 100, transitioning smoothly, with easing turned on
    //the output rate will change every 160ms

//Constructor Inputs:
    //BaseRate -- The base rate that the random rate is added/subtracted from
    //RateRangeMin (int16_t, so it can be negative) -- The lowest possible random amount to be added to the baseRate
    //RateRangeMax (int16_t, so it can be negative) -- The highest possible random amount to be added to the baseRate
    //Easing -- Sets the rate easing setting used in RateCtrl 
    //Rate -- The update rate of the utility (ms)

//Functions:
    //Note: Most of these are setter functions for changing the varaibles in the RateRandomizer and RateCtrl instances
    //These instances are public. You can access them directly via  RR-> and RC-> respectively
    //setRangeMin(newRangeMin) -- Changes the rateRangeMin used in the RateRandomizer instance 
    //setRangeMax(newRangeMax) -- Changes the rateRangeMax used in the RateRandomizer instance 
    //setBaseRate(newBaseRate) -- Changes the baseRate used in the RateRandomizer instance
    //setRandomizer(newBaseRate, newRangeMin, newRangeMax) -- Changes the base rate, and min/max ranges of the RateRandomizer all at once
    //setEasing(bool newEasing) -- Changes the easing setting of the RateCtrl instance
    //reset() -- Ends the current transition instantly
    //update() -- updates the effect

//Other Settings:
    //holdTime (default 0) -- Sets a time (ms) that the output rate will be held at after finishing a transiton before starting the next

//Flags:
    //holdActive -- Set when the class is in the holding time after transitioning between rates

//Notes:
    //The update rate used in the RateCtrl instance is bound (via pointers) to the rate used in the RandRateCtrlPS
    //So changing the RandRateCtrlPS rate will change the RateCtrl as well to keep them in sync

    //A new random rate is only picked once a rate transition is finished, so it's update rate doesn't matter

    //DO NOT change the cycle or changeHold settings of the RateCtrl instance
class RandRateCtrlPS : public EffectBasePS {
    public:
        RandRateCtrlPS(uint16_t BaseRate, int16_t RateRangeMin, int16_t RateRangeMax, bool Easing, uint16_t Rate);

        ~RandRateCtrlPS();  

        RateRandomizerPS
            *RR;
        
        RateCtrlPS
            *RC;
        
        uint16_t
            holdTime = 0,
            *outputRate;
        
        bool
            holdActive = false;
        
        void 
            setRangeMin(int16_t newRangeMin),
            setRangeMax(int16_t newRangeMax),
            setBaseRate(uint16_t newBaseRate),
            setRandomizer(uint16_t newBaseRate, int16_t newRangeMin, int16_t newRangeMax),
            setEasing(bool newEasing),
            reset(),
            update(void);
    
    private:
        unsigned long
            currentTime,
            holdStartTime = 0,
            prevTime = 0;
};

#endif