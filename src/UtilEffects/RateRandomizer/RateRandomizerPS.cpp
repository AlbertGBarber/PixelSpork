#include "RateRandomizerPS.h"

RateRandomizerPS::RateRandomizerPS(uint16_t BaseRate, int16_t RateRangeMin, int16_t RateRangeMax, uint16_t Rate)
    : rateRangeMin(RateRangeMin), rateRangeMax(RateRangeMax)  //
{
    //do the pointer binding for the base rate so it can be externally tied
    baseRateOrig = BaseRate;
    baseRate = &baseRateOrig;
    //set the value of the output rate to the base rate so that it has a value before we update
    outputRate = *baseRate;
    bindClassRatesPS();
}

void RateRandomizerPS::resetToBaseRate() {
    outputRate = *baseRate;
    active = false;
}

//update call is pretty simple
//we take the base rate and add on a random value between the rate ranges
//this is stored in tempOut, a int32, so we can check for int16 over/under flow
void RateRandomizerPS::update() {
    currentTime = millis();

    if( active ) {
        if( (currentTime - prevTime) >= *rate ) {
            prevTime = currentTime;

            tempOut = *baseRate + random(rateRangeMin, rateRangeMax);

            if( tempOut < 0 ) {
                tempOut = 0;
            } else if( tempOut > 65536 ) {
                tempOut = 65536;
            }

            outputRate = tempOut;
        }
    }
}