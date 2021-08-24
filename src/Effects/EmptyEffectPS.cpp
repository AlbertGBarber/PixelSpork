#include "EmptyEffectPS.h"

EmptyEffectPS::EmptyEffectPS(SegmentSet &SegmentSet, uint16_t Rate):
    segmentSet(SegmentSet)
    {    
        //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
        bindSegPtrPS();
        bindClassRatesPS();
	}

void EmptyEffectPS::update(){
    currentTime = millis();
    //if we're using an external rate variable, get its value
    globalRateCheckPS();
    if( ( currentTime - prevTime ) >= rate ) {
        prevTime = currentTime;
        
        showCheckPS();
    }
}