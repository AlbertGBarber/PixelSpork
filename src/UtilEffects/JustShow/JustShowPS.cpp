#include "JustShowPS.h"

JustShowPS::JustShowPS(SegmentSet &SegSet, uint16_t Rate):
    SegSet(SegSet)
    {    
        //bind the rate and SegmentSet pointer vars since they are inherited from BaseEffectPS
        bindSegPtrPS();
        bindClassRatesPS();
	}

//Just calls segDrawUtils::show() at the update rate
void JustShowPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        showCheckPS();
    }
}