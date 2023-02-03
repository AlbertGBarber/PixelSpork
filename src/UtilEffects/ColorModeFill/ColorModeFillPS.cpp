#include "ColorModeFillPS.h"

ColorModeFillPS::ColorModeFillPS(SegmentSet &SegmentSet, uint8_t ColorMode, uint16_t Rate):
    segmentSet(SegmentSet), colorMode(ColorMode)
    {    
        //bind the rate and segmentSet pointer vars since they are inherited from BaseEffectPS
        bindSegPtrPS();
        bindClassRatesPS();
	}

void ColorModeFillPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        segDrawUtils::fillSegSetColor(segmentSet, color, colorMode);
        showCheckPS();
    }
}