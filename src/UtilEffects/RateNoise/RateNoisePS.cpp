#include "RateNoisePS.h"

RateNoisePS::RateNoisePS(uint16_t RateMin, uint16_t RateMax, uint16_t NoiseSpeed, uint16_t Rate)
    : rateMin(RateMin), rateMax(RateMax), noiseSpeed(NoiseSpeed)  //
{
    bindClassRatesPS();
    setRandFact();
}

//Updates the utility
//The update process is pretty simple, we just get a new noise value based on the time
//and the do some scaling to map it between the max and min rate settings
void RateNoisePS::update() {
    currentTime = millis();

    if( active && (currentTime - prevTime) >= *rate ) {
        prevTime = currentTime;
        
        //Get 16bit noise data, varying with time
        noiseData = inoise16(currentTime * noiseSpeed, (currentTime * randFact) / 255);
        //The second input is intended to help add extra randomness to the noise
        //I've tested a lot of different methods to do this, the easiest way is to have something that varies with time. 
        //To keep the effect subtle, and not dominate the primary first time,
        //I scale the time down using a ratio, randFact/255.

        //inoise tends to pick values in the middle of the range,
        //so we want to treat this range as the full "range" of the noise
        noiseData = constrain(noiseData, 13000, 51000);

        //map the noise data to between rateMin and rateMax
        outputRate = map(noiseData, 13000, 51000, rateMin, rateMax);
    }
}

void RateNoisePS::setRandFact(){
    randFact = random8();
}
