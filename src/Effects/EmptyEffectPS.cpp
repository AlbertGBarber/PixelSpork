#include "EmptyEffectPS.h"

EmptyEffectPS::EmptyEffectPS(SegmentSet &SegSet, uint16_t Rate):
    SegSet(SegSet)
    {    
        //bind the rate and SegmentSet pointer vars since they are inherited from BaseEffectPS
        bindSegPtrPS();
        bindClassRatesPS();
        //bind background color pointer (if needed)
        //bindBGColorPS();
	}

void EmptyEffectPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        
        showCheckPS();
    }
}