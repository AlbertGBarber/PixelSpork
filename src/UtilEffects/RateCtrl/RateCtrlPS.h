#ifndef RateCtrlPS_h
#define RateCtrlPS_h

#include "Effects/EffectBasePS.h"

//A utility that takes start rate and and end rate (uint16_t's) and produces an outputRate
//that smoothly transitions from the start rate to the end rate
//The direction of the transition is automatically determined from the start and end rates

//Easing set to produce a nonlinear transition to the output rate
//this uses FastLED's ease8InOutApprox function

//The utility can be set to cycle continuously between the start and end rates
//ie it will constantly transition back and forth between them

//You can also pause the rate after it has transitioned using the changeHold variable

//The transition rate is stored in outputRate. Use &outputRate to bind it to an effect rate.

//The RateCtrlPS update rate is a pointer, and can be bound externally, like in other effects

//Example calls: 
    //RateCtrlPS(90, 20, false, true, 100);
    //Transitions between a rate of 90ms to 20ms and back again (cycle is on)
    //easing is turned off, the utility updates every 100ms

//Constructor Inputs:
    //StartRate -- The initial rate for the transition
    //EndRate -- The final/target rate for the transition
    //Easing -- Sets the rate easing setting used in RateCtrl 
    //Cycle -- Sets the utility to cycle continuously between the start and end rates
    //Rate -- The update rate of the utility (ms)

//Functions:
    //direct() --  returns the direction of the transition: true if the end rate > start rate, false otherwise
    //             DO NOT try to set the direction manually
    //reset(StartRate, EndRate, Rate) -- resets the utility with a new start rate, end rate, and update rate
    //                                   DO NOT change the start/end rates unless you know what you're doing
    //reset() -- restarts the transition using the current start/end rates
    //update() -- updates the effect

//Other Settings:
    //changeHold (default 0) -- Sets a time (ms) that the output rate will be held at after finishing a transiton
    //cycle -- Sets if the utility will cycle continuously between the start and end rates
    //easing -- Sets if easing will be used for the rate transtion
    //outputRate -- The output rate of the transition

//Flags:
    //rateReached -- Set when the ulility reaches the end rate

class RateCtrlPS : public EffectBasePS{
    public:
        RateCtrlPS(uint16_t StartRate, uint16_t EndRate, bool Easing, bool Cycle, uint16_t Rate);

    uint16_t
        startRate,
        endRate,
        changeHold = 0,
        outputRate;
    
    bool
        cycle,
        rateReached,
        easing,
        direct(void);
    
    void 
        update(),
        reset( uint16_t StartRate, uint16_t EndRate, uint16_t  Rate),
        reset();

    private:
        unsigned long
            currentTime,
            prevTime = 0;

        int8_t
            stepMulti;
        
        uint8_t
            ratio;
        
        uint16_t
            rateTemp;
        
        bool
            _direct;

        void
            setDirect(bool newDirect);
};  

#endif