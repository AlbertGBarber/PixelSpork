#include "ColorModeFillPS.h"

ColorModeFillPS::ColorModeFillPS(SegmentSet &SegSet, uint8_t ColorMode, uint16_t Rate):
    SegSet(SegSet), colorMode(ColorMode)
    {    
        //bind the rate and SegSet pointer vars since they are inherited from BaseEffectPS
        bindSegPtrPS();
        bindClassRatesPS();
	}

void ColorModeFillPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        segDrawUtils::fillSegSetColor(SegSet, color, colorMode);
        showCheckPS();
    }
}