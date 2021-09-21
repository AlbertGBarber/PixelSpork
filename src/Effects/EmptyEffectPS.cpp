#include "EmptyEffectPS.h"

EmptyEffectPS::EmptyEffectPS(SegmentSet &SegmentSet, uint16_t Rate):
    segmentSet(SegmentSet)
    {    
        //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
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