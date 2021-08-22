#ifndef RateCtrlPS_h
#define RateCtrlPS_h

#include "FastLED.h"
#include "Effects/EffectBasePS.h"
//does not allow using a global rate b/c the changeRate can use easing

class RateCtrlPS : public EffectBasePS{
    public:
        RateCtrlPS(uint16_t StartRate, uint16_t EndRate, bool Easing, bool Cycle, uint16_t ChangeRate);

    uint16_t
        startRate,
        endRate,
        changeHold = 0,
        changeRate,
        rate;
    
    bool
        cycle,
        rateReached,
        easing,
        direct(void);
    
    void 
        update(),
        reset( uint16_t StartRate, uint16_t EndRate, uint16_t ChangeRate ),
        reset();

    private:
        int8_t
            stepMulti;
        
        uint16_t
            changeRateTemp;
        
        unsigned long
            currentTime,
            prevTime = 0;
        
        bool
            _direct;

        void
            setDirect(bool newDirect);
};  

#endif