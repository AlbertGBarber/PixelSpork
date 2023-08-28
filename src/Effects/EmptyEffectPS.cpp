#include "EmptyEffectPS.h"

EmptyEffectPS::EmptyEffectPS(SegmentSetPS &SegSet, uint8_t AnArg, uint16_t Rate)
    : anArg(AnArg)  //set anArg = AnArg (the input argument)
{
    //bind the rate and segSet pointer vars since they are inherited from BaseEffectPS
    bindSegSetPtrPS();
    bindClassRatesPS();
    //bind background color pointer (if needed)
    //bindBGColorPS();
}

void EmptyEffectPS::update() {
    currentTime = millis();

    if( (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;
        //Do effect stuff!
        showCheckPS();
    }
}