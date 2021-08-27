#include "BGRateFillPS.h"

BGRateFillPS::BGRateFillPS(SegmentSet &SegmentSet, uint8_t colorMode, uint16_t Rate):
    segmentSet(SegmentSet), colorMode(ColorMode)
    {    
        //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
        bindSegPtrPS();
        bindClassRatesPS();
	}

void BGRateFillPS::update(){
    currentTime = millis();
    //if we're using an external rate variable, get its value
    globalRateCheckPS();
    if( ( currentTime - prevTime ) >= rate ) {
        prevTime = currentTime;
        segDrawUtils::fillSegSetColor(segmentSet, color, colorMode);
        showCheckPS();
    }
}