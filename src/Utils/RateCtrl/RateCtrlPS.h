#ifndef RateCtrlPS_h
#define RateCtrlPS_h

#include "FastLED.h"
#include "Effects/EffectBasePS.h"
//does not allow using a global Rate b/c the Rate can use easing
//which changes the rate as part of the class function
//if you try to bind rate to an external variable, it will just be rebound to the
//rateOrig (see Effectbase.h)
//You can change rateOrig, but it is not a pointer, so you can't bind it externally

class RateCtrlPS : public EffectBasePS{
    public:
        RateCtrlPS(uint16_t StartRate, uint16_t EndRate, bool Easing, bool Cycle, uint16_t Rate);

    uint16_t
        startRate,
        endRate,
        changeHold = 0,
        rateTemp,
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
        int8_t
            stepMulti;
        
        unsigned long
            currentTime,
            prevTime = 0;
        
        bool
            _direct;

        void
            setDirect(bool newDirect);
};  

#endif