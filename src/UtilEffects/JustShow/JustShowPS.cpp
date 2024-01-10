#include "JustShowPS.h"

JustShowPS::JustShowPS(SegmentSetPS &SegSet, uint16_t Rate)  //
{
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();
}

//Just calls segDrawUtils::show() at the update rate
void JustShowPS::update() {
    currentTime = millis();

    if( active && (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;
        showCheckPS();
    }
}