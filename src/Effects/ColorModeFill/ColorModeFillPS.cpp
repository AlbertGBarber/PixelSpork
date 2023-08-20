#include "ColorModeFillPS.h"

ColorModeFillPS::ColorModeFillPS(SegmentSet &SegSet, uint8_t ColorMode, uint16_t Rate):
    colorMode(ColorMode)
    {    
        //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
        bindSegSetPtrPS();
        bindClassRatesPS();
	}

void ColorModeFillPS::update(){
    currentTime = millis();

    if( ( currentTime - prevTime ) >= *rate ) {
        prevTime = currentTime;
        segDrawUtils::fillSegSetColor(*segSet, color, colorMode);
        showCheckPS();
    }
}